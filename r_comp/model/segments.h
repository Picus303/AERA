

#ifndef segments_h
#define segments_h

#include <unordered_map>
#include "../r_code/object.h"
#include "../r_code/list.h"

#include "class.h"

namespace r_comp {

class Reference {
public:
  Reference();
  Reference(const uint16 i, const Class &c, const Class &cc);
  uint16 index_;
  Class class_;
  Class cast_class_;
};

// All classes below map components of r_code::Image into r_comp::Image.
// Both images are equivalent, the latter being easier to work with (uses vectors instead of a contiguous structure, that is r_code::Image::data).
// All read(word32*,uint32)/write(word32*) functions defined in the classes below perfom read/write operations in an r_code::Image::data.

class dll_export Metadata {
private:
  uint32 get_class_array_size();
  uint32 get_classes_size();
  uint32 get_sys_classes_size();
  uint32 get_class_names_size();
  uint32 get_operator_names_size();
  uint32 get_function_names_size();
public:
  Metadata();

  std::unordered_map<std::string, Class> classes_; // non-sys classes, operators and device functions.
  std::unordered_map<std::string, Class> sys_classes_;

  r_code::resized_vector<std::string> class_names_; // classes and sys-classes; does not include set classes.
  r_code::resized_vector<std::string> operator_names_;
  r_code::resized_vector<std::string> function_names_;
  r_code::resized_vector<Class> classes_by_opcodes_; // classes indexed by opcodes; used to retrieve member names; registers all classes (incl. set classes).
  r_code::resized_vector<uint16> usr_classes_;

  Class *get_class(std::string &class_name);
  Class *get_class(uint16 opcode);

  void write(word32 *data);
  void read(word32 *data, uint32 size);
  uint32 get_size();
};

class dll_export ObjectMap {
public:
  r_code::resized_vector<uint16> objects_;

  void shift(uint16 offset);

  void write(word32 *data);
  void read(word32 *data, uint32 size);
  uint32 get_size() const;
};

class dll_export CodeSegment {
public:
  r_code::resized_vector<r_code::SysObject *> objects_;

  ~CodeSegment();

  void write(word32 *data);
  void read(word32 *data, uint16 object_count);
  uint32 get_size();
};

class dll_export ObjectNames {
public:
  std::unordered_map<uint32, std::string> symbols_; // indexed by objects' OIDs.

  ~ObjectNames();

  void write(word32 *data);
  void read(word32 *data);
  uint32 get_size();

  /**
   * Find the name in symbols_.
   * \param name The symbol name to search for.
   * \return The OID, or UNDEFINED_OID if not found.
   */
  uint32 findSymbol(const std::string& name);
};

class dll_export Image {
private:
  uint32 map_offset_;
  std::unordered_map<r_code::Code *, uint16> ptrs_to_indices_; // used for injection in memory.

  void add_object(r_code::Code *object, bool include_invalidated);
  r_code::SysObject *add_object(r_code::Code *object, std::vector<r_code::SysObject *> &imported_objects);
  uint32 get_reference_count(const r_code::Code *object) const;
  void build_references();
  void build_references(r_code::SysObject *sys_object, r_code::Code *object);
  void unpack_objects(r_code::resized_vector<r_code::Code *> &ram_objects);
public:
  ObjectMap object_map_;
  CodeSegment code_segment_;
  ObjectNames object_names_;

  Timestamp timestamp_;

  Image();
  ~Image();

  void add_sys_object(r_code::SysObject *object, std::string name); // called by the compiler.
  void add_sys_object(r_code::SysObject *object); // called by add_object().

  void get_objects(r_code::Mem *mem, r_code::resized_vector<r_code::Code *> &ram_objects);
  template<class O> void get_objects(r_code::resized_vector<r_code::Code *> &ram_objects) {

    for (uint32 i = 0; i < code_segment_.objects_.size(); ++i) {

      uint16 opcode = code_segment_.objects_[i]->code_[0].asOpcode();
      ram_objects[i] = new O(code_segment_.objects_[i]);
    }
    unpack_objects(ram_objects);
  }

  /**
   * Add all the objects to this Image. (Called by the rMem.)
   * \param objects The list of objects to add.
   * \param include_invalidated (optional) If true add all the objects, if false only
   * add object o if not o->is_invalidated(). If omitted, don't include invalidated.
   */
  void add_objects(r_code::list<P<r_code::Code> > &objects, bool include_invalidated = false);

  void add_objects(r_code::list<P<r_code::Code> > &objects, std::vector<r_code::SysObject *> &imported_objects); // called by any r_exec code for decompiling on the fly.

  template<class I> I *serialize() {

    I *image = (I *)I::Build(timestamp_, object_map_.get_size(), code_segment_.get_size(), object_names_.get_size());

    object_map_.shift(image->map_size());
    object_map_.write(image->data());
    code_segment_.write(image->data() + image->map_size());
    object_names_.write(image->data() + image->map_size() + image->code_size());

    return image;
  }

  template<class I> void load(I *image) {

    timestamp_ = image->timestamp();
    object_map_.read(image->data(), image->map_size());
    code_segment_.read(image->data() + image->map_size(), image->map_size());
    object_names_.read(image->data() + image->map_size() + image->code_size());
  }
};
}


#endif
