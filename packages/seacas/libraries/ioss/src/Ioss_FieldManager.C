// Copyright(C) 1999-2010
// Sandia Corporation. Under the terms of Contract
// DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains
// certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Sandia Corporation nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <Ioss_Field.h>
#include <Ioss_FieldManager.h>
#include <cassert>
#include <cstddef>
#include <map>
#include <string>
#include <utility>

/** \brief Add a field to the field manager.
 *
 *  Assumes that a field with the same name does not already exist.
 *
 *  \param[in] new_field The field to add
 *
 */
void Ioss::FieldManager::add(const Ioss::Field &new_field)
{
  if (!exists(new_field.get_name())) {
    IOSS_FUNC_ENTER(m_);
    fields.insert(FieldValuePair(new_field.get_name(), new_field));
  }
}

/** \brief Checks if a field with a given name exists in the field manager.
 *
 * \param[in] field_name The name of the field to check for.
 * \returns True if the field exists in the field manager.
 *
 */
bool Ioss::FieldManager::exists(const std::string &field_name) const
{
  IOSS_FUNC_ENTER(m_);
  return (fields.find(field_name) != fields.end());
}

/** \brief Get a field from the field manager.
 *
 *  \param[in] field_name The name of the field to get.
 *  \returns The field object.
 *
 */
Ioss::Field Ioss::FieldManager::get(const std::string &field_name) const
{
  IOSS_FUNC_ENTER(m_);
  auto iter = fields.find(field_name);
  assert(iter != fields.end());
  return (*iter).second;
}

/** \brief Get a reference to a field from the field manager.
 *
 *  \param[in] field_name The name of the field to get.
 *  \returns A reference to the field object.
 *
 */
const Ioss::Field &Ioss::FieldManager::getref(const std::string &field_name) const
{
  IOSS_FUNC_ENTER(m_);
  auto iter = fields.find(field_name);
  assert(iter != fields.end());
  return (*iter).second;
}

/** \brief Remove a field from the field manager.
 *
 * Assumes that a field with the given name exists in the field manager.
 *
 * \param[in] field_name The name of the field to remove.
 */
void Ioss::FieldManager::erase(const std::string &field_name)
{
  assert(exists(field_name));
  IOSS_FUNC_ENTER(m_);
  auto iter = fields.find(field_name);
  if (iter != fields.end()) {
    fields.erase(iter);
  }
}

/** \brief Get the names of all fields in the field manager.
 *
 * \param[out] names All field names in the field manager.
 * \returns The number of fields extracted from the field manager.
 *
 */
int Ioss::FieldManager::describe(NameList *names) const
{
  IOSS_FUNC_ENTER(m_);
  int                          the_count = 0;
  FieldMapType::const_iterator I;
  for (I = fields.begin(); I != fields.end(); ++I) {
    names->push_back((*I).first);
    the_count++;
  }
  return the_count;
}

/** \brief Get the names of all fields of a specified RoleType in the field manager.
 *
 * \param[in] role The role type (MESH, ATTRIBUTE, TRANSIENT, REDUCTION, etc.)
 * \param[out] names All field names of the specified RoleType in the field manager.
 * \returns The number of fields extracted from the field manager.
 *
 */
int Ioss::FieldManager::describe(Ioss::Field::RoleType role, NameList *names) const
{
  IOSS_FUNC_ENTER(m_);
  int                          the_count = 0;
  FieldMapType::const_iterator I;
  for (I = fields.begin(); I != fields.end(); ++I) {
    if ((*I).second.get_role() == role) {
      names->push_back((*I).first);
      the_count++;
    }
  }
  return the_count;
}

/** \brief Get the number of fields in the field manager.
 *
 *  \returns The number of fields in the field manager.
 */
size_t Ioss::FieldManager::count() const
{
  IOSS_FUNC_ENTER(m_);
  return fields.size();
}
