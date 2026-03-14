

#include "cpp_programs.h"

#include <iostream>

using namespace std;

namespace r_exec {

unordered_map<std::string, CPPPrograms::Program> CPPPrograms::Programs_;

void CPPPrograms::Register(const std::string &pgm_name, Program pgm) {

  Programs_[pgm_name] = pgm;
}

CPPPrograms::Program CPPPrograms::Get(const std::string &pgm_name) {

  unordered_map<std::string, Program>::const_iterator it = Programs_.find(pgm_name);
  if (it != Programs_.end())
    return it->second;
  return NULL;
}

Controller *CPPPrograms::New(const std::string &pgm_name, r_code::_View *view) {

  CPPPrograms::Program pgm = Get(pgm_name);
  if (pgm != NULL)
    return pgm(view);
  else {

    std::cerr << "cpp pgm " << pgm_name << " could not be found\n";
    return NULL;
  }
}
}
