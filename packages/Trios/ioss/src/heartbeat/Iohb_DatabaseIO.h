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

#ifndef IOSS_Iohb_DatabaseIO_h
#define IOSS_Iohb_DatabaseIO_h

#include <Ioss_CodeTypes.h>
#include <Ioss_DatabaseIO.h>
#include <Ioss_IOFactory.h>
#include <Ioss_Field.h>
#include <Ioss_DBUsage.h>

#include <string>
#include <assert.h>
#include <iostream>

namespace Ioss {
  class GroupingEntity;
  class Region;
  class EntityBlock;
  class NodeBlock;
  class FaceBlock;
  class ElementBlock;
  class NodeSet;
  class EdgeSet;
  class FaceSet;
  class CommSet;
}

namespace Iohb {
  class Layout;
}

namespace Iohb {

  enum Format{DEFAULT=0,SPYHIS=1};

  class IOFactory : public Ioss::IOFactory
    {
    public:
      static const IOFactory* factory();
    private:
      IOFactory();
      Ioss::DatabaseIO* make_IO(const std::string& filename,
				Ioss::DatabaseUsage db_usage,
				MPI_Comm communicator) const;
    };

  class DatabaseIO : public Ioss::DatabaseIO
    {
    public:
      DatabaseIO(Ioss::Region *region, const std::string& filename,
		 Ioss::DatabaseUsage db_usage, MPI_Comm communicator);
      ~DatabaseIO();

      int node_global_to_local(int /* global */, bool /* must_exist */) const {return 0;}
      int element_global_to_local(int /* global */) const {return 0;}

      // Check capabilities of input/output database...
      bool supports_nodal_fields()   const {return false;}
      bool supports_edge_fields()    const {return false;}
      bool supports_face_fields()    const {return false;}
      bool supports_element_fields() const {return false;}
      bool supports_nodelist_fields() const {return false;}

      void read_meta_data() {}

      bool begin(Ioss::State state);
      bool   end(Ioss::State state);

      bool begin_state(Ioss::Region *region, int state, double time);
      bool   end_state(Ioss::Region *region, int state, double time);

    private:
      void initialize(const Ioss::Region *region) const;
      
      int get_field_internal(const Ioss::Region* reg, const Ioss::Field& field,
			     void *data, size_t data_size) const;

      int get_field_internal(const Ioss::ElementBlock* eb, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int get_field_internal(const Ioss::FaceBlock* eb, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int get_field_internal(const Ioss::EdgeBlock* eb, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int get_field_internal(const Ioss::NodeBlock* nb, const Ioss::Field& field,
			     void *data, size_t data_size) const;

      int get_field_internal(const Ioss::NodeSet* ns, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int get_field_internal(const Ioss::EdgeSet* es, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int get_field_internal(const Ioss::FaceSet* fs, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int get_field_internal(const Ioss::CommSet* cs, const Ioss::Field& field,
			     void *data, size_t data_size) const;

      int put_field_internal(const Ioss::Region* region, const Ioss::Field& field,
			     void *data, size_t data_size) const;

      int put_field_internal(const Ioss::ElementBlock* eb, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int put_field_internal(const Ioss::FaceBlock* fb, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int put_field_internal(const Ioss::EdgeBlock* nb, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int put_field_internal(const Ioss::NodeBlock* nb, const Ioss::Field& field,
			     void *data, size_t data_size) const;

      int put_field_internal(const Ioss::NodeSet* ns, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int put_field_internal(const Ioss::EdgeSet* es, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int put_field_internal(const Ioss::FaceSet* fs, const Ioss::Field& field,
			     void *data, size_t data_size) const;
      int put_field_internal(const Ioss::CommSet* cs, const Ioss::Field& field,
			     void *data, size_t data_size) const;

      // Private member functions
      DatabaseIO(const DatabaseIO& from); // do not implement
      DatabaseIO& operator=(const DatabaseIO& from); // do not implement

      std::ostream *logStream;
      Layout *layout_;
      Layout *legend_;

      std::string tsFormat;
      int precision_;
      bool showLabels;
      bool showLegend;

      bool initialized_;
      enum Format fileFormat;
    };
}
#endif // IOSS_Iohb_DatabaseIO_h