
#include "auto_focus_program.h"
#include "attention/auto_focus.h"


namespace aera::builtin {

r_exec::Controller* CreateAutoFocusProgram(r_code::_View* view) {

  return new r_exec::AutoFocusController(view);
}

}  // namespace aera::builtin
