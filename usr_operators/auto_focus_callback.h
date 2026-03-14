

#ifndef auto_focus_proxy_h
#define auto_focus_proxy_h

#include "../r_exec/overlay.h"


extern "C" {
r_exec::Controller dll_export *auto_focus(r_code::_View *view);
}


#endif
