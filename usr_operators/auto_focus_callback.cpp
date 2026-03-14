

#include "auto_focus_callback.h"
#include "../r_exec/auto_focus.h"


r_exec::Controller *auto_focus(r_code::_View *view) {

  return new r_exec::AutoFocusController(view);
}
