

#include "r_comp.h"
#include <iostream>


#ifdef R_COMP_EXPORTS
void Init() {

  std::cout << "r_comp library loaded" << std::endl;
}
#endif

