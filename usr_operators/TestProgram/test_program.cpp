

#include "test_program.h"

#include "mem.h"

using namespace r_code;

// Sample c++ user-defined program.
class TestController :
  public r_exec::Controller {
private:
  float32 arg1;
  bool arg2;
public:
  TestController(r_code::_View *icpp_pgm_view) : r_exec::Controller(icpp_pgm_view) {

    // Load arguments here: one float and one Boolean.
    uint16 arg_set_index = get_object()->code(ICPP_PGM_ARGS).asIndex();
    uint16 arg_count = get_object()->code(arg_set_index).getAtomCount();
    if (arg_count != 2) {

      std::cerr << "test_program error: expected 2 arguments, got " << arg_count << std::endl;
      return;
    }
    arg1 = get_object()->code(arg_set_index + 1).asFloat();
    arg2 = get_object()->code(arg_set_index + 2).asBoolean();
  }

  ~TestController() {
  }

  Code *get_core_object() const override { return get_object()->get_reference(0); }

  void take_input(r_exec::View *input) override {

    // Inputs are all types of objects - salient or that have become salient depending on their view's sync member.
    // Manual filtering may be needed instead of pattern-matching.

    //input->object->trace();
  }
};

////////////////////////////////////////////////////////////////////////////////

r_exec::Controller *test_program(r_code::_View *view) {

  return new TestController(view);
}
