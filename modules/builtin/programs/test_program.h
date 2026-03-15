
#ifndef builtin_test_program_h
#define builtin_test_program_h

namespace r_code {
class _View;
}

namespace r_exec {
class Controller;
}

namespace aera::builtin {

r_exec::Controller* CreateTestProgram(r_code::_View* view);

}  // namespace aera::builtin

#endif
