// Copyright(C) 1999-2021, 2023, 2025, 2025 National Technology & Engineering Solutions
// of Sandia, LLC (NTESS).  Under the terms of Contract DE-NA0003525 with
// NTESS, the U.S. Government retains certain rights in this software.
//
// See packages/seacas/LICENSE for details

#include "ED_SystemInterface.h"
#include "exo_entity.h"
#include "exodusII.h" // for ex_get_var, EX_INVALID_ID, etc
#include "fmt/color.h"
#include "fmt/ostream.h"
#include "smart_assert.h" // for SMART_ASSERT
#include "stringx.h"      // for to_lower
#include <cstdint>        // for int64_t
#include <cstdlib>        // for exit
#include <cstring>        // for strlen
#include <string>
#include <vector>

namespace {
  size_t get_index(int file_id, EXOTYPE exo_type, size_t id, const char *label);
  size_t get_num_entities(int file_id, EXOTYPE exo_type);
  size_t get_num_variables(int file_id, EXOTYPE type, const char *label);
  size_t get_num_attributes(int file_id, EXOTYPE type, size_t id, const char *label);
#ifndef NDEBUG
  size_t get_num_timesteps(int file_id);
#endif
} // namespace

Exo_Entity::Exo_Entity(int file_id, size_t id) : fileId(file_id), id_(id)
{
  SMART_ASSERT(file_id > 0);
  SMART_ASSERT(static_cast<int>(id) > EX_INVALID_ID);
}

Exo_Entity::Exo_Entity(int file_id, size_t id, size_t nnodes)
    : fileId(file_id), id_(id), numEntity(nnodes)
{
  SMART_ASSERT(file_id > 0);
  SMART_ASSERT(static_cast<int>(id) > EX_INVALID_ID);
}

Exo_Entity::~Exo_Entity()
{
  delete[] truth_;
  if (numVars > 0) {
    for (int i = 0; i < numVars; ++i) {
      delete[] results_[i];
    }
    delete[] results_;
  }
  if (numAttr > 0) {
    for (int i = 0; i < numAttr; ++i) {
      delete[] attributes_[i];
    }
  }
}

void Exo_Entity::initialize(int file_id, size_t id)
{
  fileId = file_id;
  id_    = id;

  index_ = get_index(fileId, exodus_type(), id_, label());

  entity_load_params();

  internal_load_params();
}

bool Exo_Entity::is_valid_var(size_t var_index) const
{
  SMART_ASSERT(static_cast<int>(var_index) < numVars);
  if (truth_ == nullptr) {
    get_truth_table();
  }

  return (truth_[var_index] != 0);
}

std::string Exo_Entity::Load_Results(int time_step, int var_index)
{
  SMART_ASSERT(Check_State());

  if (fileId < 0) {
    return "exodiff: ERROR:  Invalid file id!";
  }
  if (id_ == EX_INVALID_ID) {
    return "exodiff: ERROR:  Must initialize block parameters first!";
  }
  if (var_index < 0 || var_index >= numVars) {
    Error("Exo_Entity::Load_Results(): var_index is invalid. Aborting...\n");
  }
  SMART_ASSERT(time_step >= 1 && time_step <= static_cast<int>(get_num_timesteps(fileId)));

  if (time_step != currentStep) {
    Free_Results();
    currentStep = time_step;
  }

  if (truth_ == nullptr) {
    get_truth_table();
  }

  if (truth_[var_index] != 0) {
    if ((results_[var_index] == nullptr) && (numEntity != 0u)) {
      results_[var_index] = new double[numEntity];
      SMART_ASSERT(results_[var_index] != nullptr);
    }
    if (numEntity != 0u) {
      int err = 0;
      err     = ex_get_var(fileId, time_step, exodus_type(), var_index + 1, id_, numEntity,
                           results_[var_index]);

      if (err < 0) {
        Error(fmt::format("Exo_Entity::Load_Results(): Call to exodus routine"
                          " returned error value! {} id = {}\n"
                          "Aborting...\n",
                          label(), id_));
      }
      else if (err > 0) {
        return fmt::format("WARNING:  Number {} returned from call to exodus get variable routine.",
                           err);
      }
    }
    else {
      return std::string("WARNING:  No items in this ") + label();
    }
  }
  else {
    return std::string("WARNING: Variable not stored in this ") + label();
  }
  return "";
}

std::string Exo_Entity::Load_Results(int t1, int t2, double proportion, int var_index)
{
  static std::vector<double> results2;

  SMART_ASSERT(Check_State());

  if (fileId < 0) {
    return "exodiff: ERROR:  Invalid file id!";
  }
  if (id_ == EX_INVALID_ID) {
    return "exodiff: ERROR:  Must initialize block parameters first!";
  }
  SMART_ASSERT(var_index >= 0 && var_index < numVars);
  SMART_ASSERT(t1 >= 1 && t1 <= static_cast<int>(get_num_timesteps(fileId)));
  SMART_ASSERT(t2 >= 1 && t2 <= static_cast<int>(get_num_timesteps(fileId)));

  if (t1 != currentStep) {
    Free_Results();
    currentStep = t1;
  }

  if (truth_ == nullptr) {
    get_truth_table();
  }

  if (truth_[var_index] != 0) {
    if ((results_[var_index] == nullptr) && (numEntity != 0u)) {
      results_[var_index] = new double[numEntity];
      SMART_ASSERT(results_[var_index] != nullptr);
    }
    if (numEntity != 0u) {
      int err =
          ex_get_var(fileId, t1, exodus_type(), var_index + 1, id_, numEntity, results_[var_index]);

      if (err < 0) {
        Error(fmt::format(
            "Exo_Entity::Load_Results(): Call to exodus routine returned error value! {} id = {}\n"
            "Aborting...\n",
            label(), id_));
      }
      else if (err > 0) {
        return fmt::format("WARNING:  Number {} returned from call to exodus get variable routine.",
                           err);
      }

      if (t1 != t2) {
        results2.resize(numEntity);
        err = ex_get_var(fileId, t2, exodus_type(), var_index + 1, id_, numEntity, Data(results2));

        if (err < 0) {
          Error(fmt::format("Exo_Entity::Load_Results(): Call to exodus routine"
                            " returned error value! {} id = {}\n"
                            "Aborting...\n",
                            label(), id_));
        }

        double *results1 = results_[var_index];
        for (size_t i = 0; i < numEntity; i++) {
          results1[i] = (1.0 - proportion) * results1[i] + proportion * results2[i];
        }
      }
    }
    else {
      return std::string("WARNING:  No items in this ") + label();
    }
  }
  else {
    return std::string("WARNING: Variable not stored in this ") + label();
  }
  return "";
}

const double *Exo_Entity::Get_Results(int var_index) const
{
  SMART_ASSERT(Check_State());
  if (currentStep == 0) {
    return nullptr;
  }
  SMART_ASSERT(var_index >= 0 && var_index < numVars);
  if (var_index >= 0 && var_index < numVars) {
    return results_[var_index];
  }

  return nullptr;
}

void Exo_Entity::Free_Results()
{
  SMART_ASSERT(Check_State());

  currentStep = 0;
  for (int v = 0; v < numVars; ++v) {
    delete[] results_[v];
    results_[v] = nullptr;
  }
}

void Exo_Entity::get_truth_table() const
{
  if (numVars > 0 && truth_ == nullptr) {
    truth_ = new int[numVars];
    SMART_ASSERT(truth_ != nullptr);
    // initialize to true for the case of no objects in the block (some older
    // versions of ex_get_object_truth_vector do not set the values at all)
    for (int i = 0; i < numVars; ++i) {
      truth_[i] = 1;
    }
    int err = ex_get_object_truth_vector(fileId, exodus_type(), id_, numVars, truth_);
    if (err < 0) {
      Error("Exo_Entity::get_truth_table(): ex_get_object_truth_vector returned error.\n");
    }
  }
}

std::string Exo_Entity::Load_Attributes(int attr_index)
{
  SMART_ASSERT(Check_State());

  if (fileId < 0) {
    return "exodiff: ERROR:  Invalid file id!";
  }
  if (id_ == EX_INVALID_ID) {
    return "exodiff: ERROR:  Must initialize block parameters first!";
  }
  SMART_ASSERT(attr_index >= 0 && attr_index < numAttr);

  if ((attributes_[attr_index] == nullptr) && (numEntity != 0u)) {
    attributes_[attr_index] = new double[numEntity];
    SMART_ASSERT(attributes_[attr_index] != nullptr);
  }

  if (numEntity != 0u) {
    int err = 0;
    err     = ex_get_one_attr(fileId, exodus_type(), id_, attr_index + 1, attributes_[attr_index]);

    if (err < 0) {
      Error(fmt::format("Exo_Entity::Load_Attributes(): Call to exodus routine"
                        " returned error value! {} id = {}\n"
                        "Aborting...\n",
                        label(), id_));
    }
    else if (err > 0) {
      return fmt::format("WARNING:  Number {} returned from call to exodus get variable routine.",
                         err);
    }
  }
  else {
    return std::string("WARNING:  No items in this ") + label();
  }

  return "";
}

const double *Exo_Entity::Get_Attributes(int attr_index) const
{
  SMART_ASSERT(Check_State());
  SMART_ASSERT(attr_index >= 0 && attr_index < numAttr);
  return attributes_[attr_index];
}

void Exo_Entity::Free_Attributes()
{
  SMART_ASSERT(Check_State());

  for (int v = 0; v < numAttr; ++v) {
    delete[] attributes_[v];
    attributes_[v] = nullptr;
  }
}

const std::string &Exo_Entity::Get_Attribute_Name(int attr_index) const
{
  SMART_ASSERT(attr_index >= 0 && attr_index < numAttr);
  return attributeNames[attr_index];
}

int Exo_Entity::Find_Attribute_Index(const std::string &name) const
{
  std::string lower_name = name;
  to_lower(lower_name);
  int indx = -1;
  for (int i = 0; i < numAttr; i++) {
    if (attributeNames[i] == lower_name) {
      indx = i;
      break;
    }
  }
  return indx;
}

void Exo_Entity::internal_load_params()
{
  int name_size = ex_inquire_int(fileId, EX_INQ_MAX_READ_NAME_LENGTH);
  {
    std::vector<char> name(name_size + 1);
    ex_get_name(fileId, exodus_type(), id_, Data(name));
    if (name[0] != '\0') {
      name_ = Data(name);
      to_lower(name_);
      generatedName_ = false;
    }
    else {
      name_ = short_label();
      name_ += "_";
      name_ += std::to_string(id_);
      generatedName_ = true;
    }
  }
  numVars = get_num_variables(fileId, exodus_type(), label());
  if (numVars != 0) {
    results_ = new double *[numVars];
    SMART_ASSERT(results_ != nullptr);
    for (int i = 0; i < numVars; ++i) {
      results_[i] = nullptr;
    }
  }

  if (exodus_type() != EX_ASSEMBLY) {
    numAttr = get_num_attributes(fileId, exodus_type(), id_, label());
    if (numAttr != 0) {
      attributes_.resize(numAttr);

      char **names = get_name_array(numAttr, name_size);
      int    err   = ex_get_attr_names(fileId, exodus_type(), id_, names);
      if (err < 0) {
        Error(fmt::format(
            "Exo_Read::Get_Init_Data(): Failed to get {} attribute names!  Aborting...\n",
            label()));
      }

      for (int vg = 0; vg < numAttr; ++vg) {
        SMART_ASSERT(names[vg] != nullptr);
        if (std::strlen(names[vg]) == 0) {
          std::string name = "attribute_" + std::to_string(vg + 1);
          attributeNames.push_back(name);
        }
        else if (static_cast<int>(std::strlen(names[vg])) > name_size) {
          fmt::print(stderr, fmt::fg(fmt::color::red),
                     "exodiff: ERROR: {} attribute names appear corrupt\n"
                     "                A length is 0 or greater than name_size({})\n"
                     "                Here are the names that I received from"
                     " a call to ex_get_attr_names(...):\n",
                     label(), name_size);
          for (int k = 1; k <= numAttr; ++k) {
            fmt::print(stderr, fmt::fg(fmt::color::red), "\t\t{}) \"{}\"\n", k, names[k - 1]);
          }
          fmt::print(stderr, fmt::fg(fmt::color::red), "                 Aborting...\n");
          exit(1);
        }
        else {
          std::string n(names[vg]);
          to_lower(n);
          attributeNames.push_back(n);
        }
      }
      free_name_array(names, numAttr);
    }
  }
}

namespace {
  size_t get_index(int file_id, EXOTYPE exo_type, size_t id, const char *label)
  {
    // Get ids...
    size_t count = get_num_entities(file_id, exo_type);
    if ((ex_int64_status(file_id) & EX_IDS_INT64_API) != 0) {
      std::vector<int64_t> ids(count);
      ex_get_ids(file_id, exo_type, Data(ids));

      for (size_t i = 0; i < count; i++) {
        if (static_cast<size_t>(ids[i]) == id) {
          return i;
        }
      }
    }
    else {
      std::vector<int> ids(count);
      ex_get_ids(file_id, exo_type, Data(ids));

      for (size_t i = 0; i < count; i++) {
        if (static_cast<size_t>(ids[i]) == id) {
          return i;
        }
      }
    }

    Error(fmt::format("{} id {} does not exist!\n", label, id));
    return 0;
  }

  size_t get_num_entities(int file_id, EXOTYPE exo_type)
  {
    ex_inquiry inquiry = EX_INQ_INVALID;
    switch (exo_type) {
    case EX_ASSEMBLY: inquiry = EX_INQ_ASSEMBLY; break;
    case EX_ELEM_BLOCK: inquiry = EX_INQ_ELEM_BLK; break;
    case EX_NODE_SET: inquiry = EX_INQ_NODE_SETS; break;
    case EX_SIDE_SET: inquiry = EX_INQ_SIDE_SETS; break;
    case EX_EDGE_BLOCK: inquiry = EX_INQ_EDGE_BLK; break;
    case EX_FACE_BLOCK: inquiry = EX_INQ_FACE_BLK; break;
    default: Error("Invalid entity type in get_num_entities\n");
    }
    SMART_ASSERT(inquiry != EX_INQ_INVALID);
    return ex_inquire_int(file_id, inquiry);
  }

  size_t get_num_variables(int file_id, EXOTYPE type, const char *label)
  {
    int num_vars = 0;
    int err      = ex_get_variable_param(file_id, type, &num_vars);
    if (err < 0) {
      Error(fmt::format("Failed to get number of '{}' variables!  Aborting...\n", label));
    }
    return num_vars;
  }

  size_t get_num_attributes(int file_id, EXOTYPE type, size_t id, const char *label)
  {
    int num_attr = 0;
    int err      = ex_get_attr_param(file_id, type, id, &num_attr);
    if (err < 0) {
      Error(fmt::format("Failed to get number of '{}' attributes!  Aborting...\n", label));
    }
    return num_attr;
  }

#ifndef NDEBUG
  size_t get_num_timesteps(int file_id) { return ex_inquire_int(file_id, EX_INQ_TIME); }
#endif
} // namespace
