

#ifndef cpp_programs_h
#define cpp_programs_h

#include <unordered_map>
#include "overlay.h"


namespace r_exec {

class r_exec_dll CPPPrograms {
public:
  typedef Controller *(*Program)(r_code::_View *);
private:
  static std::unordered_map<std::string, Program> Programs_;
public:
  static void Register(const std::string &pgm_name, Program pgm);
  static Program Get(const std::string &pgm_name);
  static Controller *New(const std::string &pgm_name, r_code::_View *view);
};
}


#endif
