

#ifndef r_code_image_h
#define r_code_image_h

#include "../core/types.h"

#include <fstream>


using namespace core;

namespace r_comp {
class Image;
}

namespace r_code {

// An image contains the following:
// - sizes of what follows
// - object map: list of indexes (4 bytes) of objects in the code segment
// - code segment: list of objects
// - object:
// - size of the code (number of atoms)
// - size of the reference set (number of pointers)
// - size of the marker set (number of pointers)
// - size of the view set (number of views)
// - code: indexes in internal pointers are relative to the beginning of the object, indexes in reference pointers are relative to the beginning of the reference set
// - reference set:
// - number of pointers
// - pointers to the relocation segment, i.e. indexes of relocation entries
// - marker set:
// - number of pointers
// - pointers to the relocation segment, i.e. indexes of relocation entries
// - view set: list of views
// - view:
// - size of the code (number of atoms)
// - size of the reference set (number of pointers)
// - list of atoms
// - reference set:
// - pointers to the relocation segment, i.e. indexes of relocation entries
//
// RAM layout of Image::data [sizes in word32]:
//
// data[0]: index of first object in data (i.e. def_size+map_size) [1]
// ... ...
// data[map_size-1]: index of last object in data [1]
// data[map_size]: first word32 of code segment [1]
// ... ...
// data[map_size+code_size-1]: last word32 of code segment [1]
//
// I is the implementation class; prototype:
// class ImageImpl{
// protected:
// uin64 get_timestamp() const;
// uint32 map_size() const;
// uint32 code_size() const;
// word32 *data(); // [object map|code segment|reloc segment]
// public:
// void *operator new(size_t,uint32 data_size);
// ImageImpl(uint32 map_size,uint32 code_size);
// ~ImageImpl();
// };

template<class I> class Image :
  public I {
  friend class r_comp::Image;
public:
  static Image<I> *Build(Timestamp timestamp, uint32 map_size, uint32 code_size, uint32 names_size);
  // file IO
  static Image<I> *Read(std::ifstream &stream);
  static void Write(Image<I> *image, std::ofstream &stream);

  Image();
  ~Image();

  uint32 get_size() const; // size of data in word32
  uint32 getObjectCount() const;
  word32 *get_object(uint32 i); // points to the code size of the object; the first atom is at get_object()+2
  word32 *getCodeSegment(); // equals get_object(0)
  uint32 getCodeSegmentSize() const;

  void trace() const;
};

// utilities
uint32 dll_export GetSize(const std::string &s); // returns the number of word32 needed to encode the string
void dll_export Write(word32 *data, const std::string &s);
void dll_export Read(word32 *data, std::string &s);
}


#include "image.tpl.cpp"


#endif
