

#ifndef callbacks_h
#define callbacks_h

#include "../types.h"

namespace usr_operators {

bool print(std::chrono::microseconds relative_time, bool suspended, const char *msg, uint8 object_count, r_code::Code **objects);

}


#endif
