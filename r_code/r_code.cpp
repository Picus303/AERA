

#include "r_code.h"
#include <iostream>


#ifdef R_CODE_EXPORTS
void Init() {

  std::cout << "r_code library loaded" << std::endl;
}
#endif
