

#include "auto_focus_callback.h"
#include "auto_focus.h"


r_exec::Controller *auto_focus(r_code::_View *view) {

  return new r_exec::AutoFocusController(view);
}
