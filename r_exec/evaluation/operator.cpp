

#include "operator.h"
#include "context.h"
#include "runtime/mem.h"
#include "init.h"
#include "metadata/opcodes.h"
#include "runtime/group.h"
#include "../core/utils.h"
#include "../r_code/utils.h"
#include <math.h>
#include "hlp_context.h"

using namespace std;
using namespace std::chrono;
using namespace r_code;

namespace r_exec {

resized_vector<Operator> Operator::Operators_;

void Operator::Register(uint16 opcode, bool(*o)(const Context &)) {

  if (Operators_[opcode].operator_)
    Operators_[opcode].setOverload(o);
  else
    Operators_[opcode] = Operator(o);
}


std::vector<r_code::Atom> OpContext::build_and_evaluate_expression(_Fact* q0, _Fact* q1, Atom op) {
  if (q0->get_reference(0)->code(MK_VAL_VALUE).isFloat() && q1->get_reference(0)->code(MK_VAL_VALUE).isFloat()) {

    float32 _q0 = q0->get_reference(0)->code(MK_VAL_VALUE).asFloat();
    float32 _q1 = q1->get_reference(0)->code(MK_VAL_VALUE).asFloat();

    Atom result;
    auto opcode = op.asOpcode();
    if (opcode == Opcodes::Gtr) { result = Atom::Boolean(_q1 > _q0); }
    else if (opcode == Opcodes::Lsr) { result = Atom::Boolean(_q1 < _q0); }
    else if (opcode == Opcodes::Gte) { result = Atom::Boolean(_q1 >= _q0); }
    else if (opcode == Opcodes::Lse) { result = Atom::Boolean(_q1 <= _q0); }
    else if (opcode == Opcodes::Add) { result = Atom::Float(_q1 + _q0); }
    else if (opcode == Opcodes::Sub) { result = Atom::Float(_q1 - _q0); }
    else if (opcode == Opcodes::Mul) { result = Atom::Float(_q1 * _q0); }
    else if (opcode == Opcodes::Div) {
      if (_q0 == 0)
        return std::vector<r_code::Atom>();
      result = Atom::Float(_q1 / _q0);
    }
    if (!result.isUndefined()) {
      std::vector<r_code::Atom> out;
      out.push_back(result);
      return out;
    }
  }
  P<r_code::LocalObject> expression = build_expression_object(q0, q1, op);
  return evaluate_expression(expression);
}

P<r_code::LocalObject> OpContext::build_expression_object(_Fact* q0, _Fact* q1, Atom op) {

  uint16 target_source_index = 0;
  uint16 consequent_source_index = 0;

  Atom target_val = q0->get_reference(0)->code(MK_VAL_VALUE);
  Atom consequent_val = q1->get_reference(0)->code(MK_VAL_VALUE);

  // Build the expression (op q1 q0), copying the code structure at q1 and q0. For example (- q1 q0) in the case of subtraction.
  P<LocalObject> expression = new LocalObject();
  uint16 extent_index = 1;
  expression->code(0) = op;
  Atom* copy_ptr = &consequent_val;
  if (consequent_val.getDescriptor() == Atom::I_PTR) {
    consequent_source_index = consequent_val.asIndex();
    copy_ptr = &q1->get_reference(0)->code(0);
    extent_index += 2;
    expression->code(1) = Atom::IPointer(extent_index);
  }
  StructureValue::copy_structure(expression, extent_index, copy_ptr, consequent_source_index);

  copy_ptr = &target_val;
  if (target_val.getDescriptor() == Atom::I_PTR) {
    target_source_index = target_val.asIndex();
    copy_ptr = &q0->get_reference(0)->code(0);
    expression->code(2) = Atom::IPointer(extent_index);
  }
  StructureValue::copy_structure(expression, extent_index, copy_ptr, target_source_index);

  return expression;
}

std::vector<r_code::Atom> OpContext::evaluate_expression(r_code::LocalObject* expression) {
  // Use HLPContext to evaluate the expression. The result goes in the OpContext's result array.
  Overlay overlay((size_t)0);
  HLPContext c(&expression->code(0), 0, (HLPOverlay*)&overlay);

  uint16 atom_count = c.get_children_count();
  for (uint16 i = 1; i <= atom_count; ++i) {

    if (!c.get_child_deref(i).evaluate_no_dereference())
      return std::vector<r_code::Atom>();
  }

  Operator op = Operator::Get(c[0].asOpcode());
  HLPContext* _c = new HLPContext(c);
  OpContext op_context(_c);
  op(op_context);
  return op_context.result();
}



////////////////////////////////////////////////////////////////////////////////

bool now(const Context &context) {

  context.setTimestampResult(Now());
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool rnd(const Context &context) {

  Context range = *context.get_child(1);

  if (!range[0].isFloat()) {

    context.setAtomicResult(Atom::Nil());
    return false;
  }

  /*Random r;float32 rng=range[0].asFloat();
  float32 result=r(range[0].asFloat());
  result/=ULONG_MAX;*/
  float32 result = (((float32)(rand() % 100)) / 100)*range[0].asFloat();
  context.setAtomicResult(Atom::Float(result));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool equ(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  bool r = (lhs == rhs);
  context.setAtomicResult(Atom::Boolean(r));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool neq(const Context &context) {

  bool r = *context.get_child(1) != *context.get_child(2);
  context.setAtomicResult(Atom::Boolean(r));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool gtr(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      bool r = lhs[0].asFloat() > rhs[0].asFloat();
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  } else if (lhs[0].getDescriptor() == Atom::TIMESTAMP) {

    if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {

      bool r = Utils::GetTimestamp(&lhs[0]) > Utils::GetTimestamp(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].getDescriptor() == Atom::DURATION) {
      bool r = Utils::GetDuration(&lhs[0]) > Utils::GetDuration(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }

  context.setAtomicResult(Atom::UndefinedBoolean());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool lsr(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      bool r = lhs[0].asFloat() < rhs[0].asFloat();
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  } else if (lhs[0].getDescriptor() == Atom::TIMESTAMP) {

    if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {

      bool r = Utils::GetTimestamp(&lhs[0]) < Utils::GetTimestamp(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].getDescriptor() == Atom::DURATION) {
      bool r = Utils::GetDuration(&lhs[0]) < Utils::GetDuration(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }

  context.setAtomicResult(Atom::UndefinedBoolean());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool gte(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      bool r = lhs[0].asFloat() >= rhs[0].asFloat();
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  } else if (lhs[0].getDescriptor() == Atom::TIMESTAMP) {

    if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {

      bool r = Utils::GetTimestamp(&lhs[0]) >= Utils::GetTimestamp(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].getDescriptor() == Atom::DURATION) {
      bool r = Utils::GetDuration(&lhs[0]) >= Utils::GetDuration(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }

  context.setAtomicResult(Atom::UndefinedBoolean());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool lse(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      bool r = lhs[0].asFloat() <= rhs[0].asFloat();
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  } else if (lhs[0].getDescriptor() == Atom::TIMESTAMP) {

    if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {

      bool r = Utils::GetTimestamp(&lhs[0]) <= Utils::GetTimestamp(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].getDescriptor() == Atom::DURATION) {
      bool r = Utils::GetDuration(&lhs[0]) <= Utils::GetDuration(&rhs[0]);
      context.setAtomicResult(Atom::Boolean(r));
      return true;
    }
  }

  context.setAtomicResult(Atom::UndefinedBoolean());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool add(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      if (lhs[0] == Atom::PlusInfinity()) {

        context.setAtomicResult(Atom::PlusInfinity());
        return true;
      }

      if (rhs[0] == Atom::PlusInfinity()) {

        context.setAtomicResult(Atom::PlusInfinity());
        return true;
      }

      context.setAtomicResult(Atom::Float(lhs[0].asFloat() + rhs[0].asFloat()));
      return true;
    } else if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {

      if (lhs[0] != Atom::PlusInfinity()) {

        context.setTimestampResult(Utils::GetTimestamp(&rhs[0]) + microseconds((int64)lhs[0].asFloat()));
        return true;
      }
    }
    else if (rhs[0].getDescriptor() == Atom::DURATION) {
      if (lhs[0] != Atom::PlusInfinity()) {
        context.setDurationResult(Utils::GetDuration(&rhs[0]) + microseconds((int64)lhs[0].asFloat()));
        return true;
      }
    }
  } else if (lhs[0].getDescriptor() == Atom::TIMESTAMP) {

    if (rhs[0].isFloat()) {

      if (rhs[0] != Atom::PlusInfinity()) {

        context.setTimestampResult(Utils::GetTimestamp(&lhs[0]) + microseconds((int64)rhs[0].asFloat()));
        return true;
      }
    }
    else if (rhs[0].getDescriptor() == Atom::DURATION) {
      context.setTimestampResult(Utils::GetTimestamp(&lhs[0]) + Utils::GetDuration(&rhs[0]));
      return true;
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].getDescriptor() == Atom::DURATION) {
      context.setDurationResult(Utils::GetDuration(&lhs[0]) + Utils::GetDuration(&rhs[0]));
      return true;
    }
    else if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {
      context.setTimestampResult(Utils::GetDuration(&lhs[0]) + Utils::GetTimestamp(&rhs[0]));
      return true;
    }
    else if (rhs[0].isFloat()) {
      if (rhs[0] != Atom::PlusInfinity()) {
        context.setDurationResult(Utils::GetDuration(&lhs[0]) + microseconds((int64)rhs[0].asFloat()));
        return true;
      }
    }
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool sub(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      if (lhs[0] == Atom::PlusInfinity()) {

        context.setAtomicResult(Atom::PlusInfinity());
        return true;
      }

      if (rhs[0] == Atom::PlusInfinity()) {

        context.setAtomicResult(Atom::Float(0));
        return true;
      }

      context.setAtomicResult(Atom::Float(lhs[0].asFloat() - rhs[0].asFloat()));
      return true;
    }
  } else if (lhs[0].getDescriptor() == Atom::TIMESTAMP) {

    if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {

      context.setDurationResult(duration_cast<microseconds>(Utils::GetTimestamp(&lhs[0]) - Utils::GetTimestamp(&rhs[0])));
      return true;
    } else if (rhs[0].isFloat()) {

      if (rhs[0] != Atom::PlusInfinity()) {

        context.setTimestampResult(Utils::GetTimestamp(&lhs[0]) - microseconds((int64)rhs[0].asFloat()));
        return true;
      }
    }
    else if (rhs[0].getDescriptor() == Atom::DURATION) {
      context.setTimestampResult(Utils::GetTimestamp(&lhs[0]) - Utils::GetDuration(&rhs[0]));
      return true;
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].getDescriptor() == Atom::DURATION) {
      context.setDurationResult(Utils::GetDuration(&lhs[0]) - Utils::GetDuration(&rhs[0]));
      return true;
    }
    else if (rhs[0].isFloat()) {
      if (rhs[0] != Atom::PlusInfinity()) {
        context.setDurationResult(Utils::GetDuration(&lhs[0]) - microseconds((int64)rhs[0].asFloat()));
        return true;
      }
    }
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool mul(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      if (lhs[0] == Atom::PlusInfinity()) {

        if (rhs[0] == Atom::PlusInfinity()) {

          context.setAtomicResult(Atom::PlusInfinity());
          return true;
        }

        if (rhs[0].asFloat() > 0) {

          context.setAtomicResult(Atom::PlusInfinity());
          return true;
        }

        if (rhs[0].asFloat() <= 0) {

          context.setAtomicResult(Atom::Float(0));
          return true;
        }
      }

      if (rhs[0] == Atom::PlusInfinity()) {

        if (lhs[0].asFloat() > 0) {

          context.setAtomicResult(Atom::PlusInfinity());
          return true;
        }

        if (lhs[0].asFloat() <= 0) {

          context.setAtomicResult(Atom::Float(0));
          return true;
        }
      }

      context.setAtomicResult(Atom::Float(lhs[0].asFloat()*rhs[0].asFloat()));
      return true;
    }
    else if (rhs[0].getDescriptor() == Atom::DURATION) {
      if (lhs[0] != Atom::PlusInfinity()) {
        context.setAtomicResult(Atom::Float(lhs[0].asFloat() * (float32)Utils::GetDuration(&rhs[0]).count()));
        return true;
      }
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].isFloat()) {
      if (rhs[0] != Atom::PlusInfinity()) {
        float64 lhs_float = (float64)Utils::GetDuration(&lhs[0]).count();
        context.setDurationResult(microseconds((int64)(lhs_float * rhs[0].asFloat())));
        return true;
      }
    }
    else if (rhs[0].getDescriptor() == Atom::DURATION) {
      // Counter-intuitive, but is the existing behavior.
      context.setAtomicResult(Atom::Float((float32)Utils::GetDuration(&lhs[0]).count() * (float32)Utils::GetDuration(&rhs[0]).count()));
      return true;
    }
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool div(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      if (rhs[0].asFloat() != 0) {

        if (lhs[0] == Atom::PlusInfinity()) {

          if (rhs[0] == Atom::PlusInfinity()) {

            context.setAtomicResult(Atom::PlusInfinity());
            return true;
          }

          if (rhs[0].asFloat() > 0) {

            context.setAtomicResult(Atom::PlusInfinity());
            return true;
          }

          if (rhs[0].asFloat() <= 0) {

            context.setAtomicResult(Atom::Float(0));
            return true;
          }
        }

        if (rhs[0] == Atom::PlusInfinity()) {

          if (lhs[0].asFloat() > 0) {

            context.setAtomicResult(Atom::PlusInfinity());
            return true;
          }

          if (lhs[0].asFloat() <= 0) {

            context.setAtomicResult(Atom::Float(0));
            return true;
          }
        }

        context.setAtomicResult(Atom::Float(lhs[0].asFloat() / rhs[0].asFloat()));
        return true;
      }
    }
    else if (rhs[0].getDescriptor() == Atom::DURATION) {
      if (lhs[0] != Atom::PlusInfinity()) {
        context.setAtomicResult(Atom::Float(lhs[0].asFloat() / (float32)Utils::GetDuration(&rhs[0]).count()));
        return true;
      }
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].isFloat()) {
      if (rhs[0] != Atom::PlusInfinity()) {
        float64 lhs_float = (float64)Utils::GetDuration(&lhs[0]).count();
        context.setDurationResult(microseconds((int64)(lhs_float / rhs[0].asFloat())));
        return true;
      }
    }
    else if (rhs[0].getDescriptor() == Atom::DURATION) {
      // Counter-intuitive, but is the existing behavior.
      context.setAtomicResult(Atom::Float((float32)Utils::GetDuration(&lhs[0]).count() / (float32)Utils::GetDuration(&rhs[0]).count()));
      return true;
    }
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool dis(const Context &context) {

  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].isFloat()) {

      context.setAtomicResult(Atom::Float(abs(lhs[0].asFloat() - rhs[0].asFloat())));
      return true;
    }
  } else if (lhs[0].getDescriptor() == Atom::TIMESTAMP) {

    if (rhs[0].getDescriptor() == Atom::TIMESTAMP) {

      context.setDurationResult(abs(duration_cast<microseconds>(Utils::GetTimestamp(&lhs[0]) - Utils::GetTimestamp(&rhs[0]))));
      return true;
    }
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION) {
    if (rhs[0].getDescriptor() == Atom::DURATION) {
      context.setDurationResult(abs(Utils::GetDuration(&lhs[0]) - Utils::GetDuration(&rhs[0])));
      return true;
    }
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool ln(const Context &context) {

  Context arg = *context.get_child(1);

  if (arg[0].isFloat()) {

    if (arg[0].asFloat() != 0) {

      context.setAtomicResult(Atom::Float(::log(arg[0].asFloat())));
      return true;
    }
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool exp(const Context &context) {

  Context arg = *context.get_child(1);

  if (arg[0].isFloat()) {

    context.setAtomicResult(Atom::Float(::exp(arg[0].asFloat())));
    return true;
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool log(const Context &context) {

  Context arg = *context.get_child(1);

  if (arg[0].isFloat()) {

    if (arg[0].asFloat() != 0) {

      context.setAtomicResult(Atom::Float(log10(arg[0].asFloat())));
      return true;
    }
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool e10(const Context &context) {

  Context arg = *context.get_child(1);

  if (arg[0].isFloat()) {

    context.setAtomicResult(Atom::Float(pow(10, arg[0].asFloat())));
    return true;
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool syn(const Context &context) {

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool ins(const Context &context) {

  return IPGMContext::Ins(*(IPGMContext *)context.get_implementation());
}

////////////////////////////////////////////////////////////////////////////////

bool red(const Context &context) {

  return IPGMContext::Red(*(IPGMContext *)context.get_implementation());
}

////////////////////////////////////////////////////////////////////////////////

bool fvw(const Context &context) {

  return IPGMContext::Fvw(*(IPGMContext *)context.get_implementation());
}

////////////////////////////////////////////////////////////////////////////////

bool is_sim(const Context &context) {

  const IPGMContext &ipgm_context = *(IPGMContext *)context.get_implementation();
  IPGMContext arg = ipgm_context.get_child_deref(1);
  Code* obj = arg.get_object();

  bool result = false;
  if (obj->code(0).asOpcode() == Opcodes::Goal)
    result = ((Goal*)obj)->is_simulation();
  else if (obj->code(0).asOpcode() == Opcodes::Pred)
    result = ((Pred*)obj)->is_simulation();

  context.setAtomicResult(Atom::Boolean(result));
  return true;
}

bool minimum(const Context &context) {
  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat() && rhs[0].isFloat()) {
    if (lhs[0] == Atom::MinusInfinity() || rhs[0] == Atom::MinusInfinity()) {
      context.setAtomicResult(Atom::MinusInfinity());
      return true;
    }

    if (lhs[0] == Atom::PlusInfinity()) {
      context.setAtomicResult(Atom::Float(rhs[0].asFloat()));
      return true;
    }
    if (rhs[0] == Atom::PlusInfinity()) {
      context.setAtomicResult(Atom::Float(lhs[0].asFloat()));
      return true;
    }

    context.setAtomicResult(Atom::Float(min(lhs[0].asFloat(), rhs[0].asFloat())));
    return true;
  }
  else if (lhs[0].getDescriptor() == Atom::TIMESTAMP && rhs[0].getDescriptor() == Atom::TIMESTAMP) {
    context.setTimestampResult(min(Utils::GetTimestamp(&lhs[0]), Utils::GetTimestamp(&rhs[0])));
    return true;
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION && rhs[0].getDescriptor() == Atom::DURATION) {
    context.setDurationResult(min(Utils::GetDuration(&lhs[0]), Utils::GetDuration(&rhs[0])));
    return true;
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

bool maximum(const Context &context) {
  Context lhs = *context.get_child(1);
  Context rhs = *context.get_child(2);

  if (lhs[0].isFloat() && rhs[0].isFloat()) {
    if (lhs[0] == Atom::PlusInfinity() || rhs[0] == Atom::PlusInfinity()) {
      context.setAtomicResult(Atom::PlusInfinity());
      return true;
    }

    if (lhs[0] == Atom::MinusInfinity()) {
      context.setAtomicResult(Atom::Float(rhs[0].asFloat()));
      return true;
    }
    if (rhs[0] == Atom::MinusInfinity()) {
      context.setAtomicResult(Atom::Float(lhs[0].asFloat()));
      return true;
    }

    context.setAtomicResult(Atom::Float(max(lhs[0].asFloat(), rhs[0].asFloat())));
    return true;
  }
  else if (lhs[0].getDescriptor() == Atom::TIMESTAMP && rhs[0].getDescriptor() == Atom::TIMESTAMP) {
    context.setTimestampResult(max(Utils::GetTimestamp(&lhs[0]), Utils::GetTimestamp(&rhs[0])));
    return true;
  }
  else if (lhs[0].getDescriptor() == Atom::DURATION && rhs[0].getDescriptor() == Atom::DURATION) {
    context.setDurationResult(max(Utils::GetDuration(&lhs[0]), Utils::GetDuration(&rhs[0])));
    return true;
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

bool id(const Context& context) {
  Context arg = *context.get_child(1);

  if (arg[0].isFloat()) {

    context.setAtomicResult(Atom::Float(arg[0].asFloat()));
    return true;
  }
  // TODO: Support copying a structured object (with possible nested structures).

  context.setAtomicResult(Atom::Nil());
  return false;
}

}
