

#include "operators.h"

#include "init.h"
#include "runtime/mem.h"

static uint16 Vec3Opcode;
static uint16 Vec2Opcode;
static uint16 VecOpcode;
static uint16 QuatOpcode;

namespace usr_operators {

using r_code::Atom;

////////////////////////////////////////////////////////////////////////////////

bool add(const r_exec::Context &context) {

  r_exec::Context lhs = *context.get_child(1);
  r_exec::Context rhs = *context.get_child(2);

  if (lhs[0].asOpcode() == Vec3Opcode && rhs[0].asOpcode() == Vec3Opcode) {

    context.setCompoundResultHead(Atom::Object(Vec3Opcode, 3));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() + (*rhs.get_child(1))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() + (*rhs.get_child(2))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(3))[0].asFloat() + (*rhs.get_child(3))[0].asFloat()));
    return true;
  }

  if (lhs[0].asOpcode() == Vec2Opcode && rhs[0].asOpcode() == Vec2Opcode) {

    context.setCompoundResultHead(Atom::Object(Vec2Opcode, 2));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() + (*rhs.get_child(1))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() + (*rhs.get_child(2))[0].asFloat()));
    return true;
  }


  if (lhs[0].asOpcode() == VecOpcode && rhs[0].asOpcode() == VecOpcode) {

    r_exec::Context lhs_vals = *lhs.get_child(1);
    r_exec::Context rhs_vals = *rhs.get_child(1);
    if (lhs_vals.get_children_count() == rhs_vals.get_children_count()) {

      int dimensionality = rhs_vals.get_children_count();
      uint16 vals_index = 2 + context.setCompoundResultHead(Atom::Object(VecOpcode, 1));
      context.addCompoundResultPart(Atom::IPointer(vals_index));

      context.addCompoundResultPart(Atom::Set(dimensionality));
      for (int i = 1; i <= dimensionality; ++i) {
        context.addCompoundResultPart(Atom::Float((*lhs_vals.get_child(i))[0].asFloat() + (*rhs_vals.get_child(i))[0].asFloat()));
      }
      return true;
    }
  }

  if (lhs[0].asOpcode() == QuatOpcode && rhs[0].asOpcode() == QuatOpcode) {

    context.setCompoundResultHead(Atom::Object(QuatOpcode, 4));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() + (*rhs.get_child(1))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() + (*rhs.get_child(2))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(3))[0].asFloat() + (*rhs.get_child(3))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(4))[0].asFloat() + (*rhs.get_child(4))[0].asFloat()));
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool sub(const r_exec::Context &context) {

  r_exec::Context lhs = *context.get_child(1);
  r_exec::Context rhs = *context.get_child(2);

  if (lhs[0].asOpcode() == Vec3Opcode && rhs[0].asOpcode() == Vec3Opcode) {

    context.setCompoundResultHead(Atom::Object(Vec3Opcode, 3));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() - (*rhs.get_child(1))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() - (*rhs.get_child(2))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(3))[0].asFloat() - (*rhs.get_child(3))[0].asFloat()));
    return true;
  }

  if (lhs[0].asOpcode() == Vec2Opcode && rhs[0].asOpcode() == Vec2Opcode) {

    context.setCompoundResultHead(Atom::Object(Vec2Opcode, 2));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() - (*rhs.get_child(1))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() - (*rhs.get_child(2))[0].asFloat()));
    return true;
  }

  if (lhs[0].asOpcode() == VecOpcode && rhs[0].asOpcode() == VecOpcode) {

    r_exec::Context lhs_vals = *lhs.get_child(1);
    r_exec::Context rhs_vals = *rhs.get_child(1);
    if (lhs_vals.get_children_count() == rhs_vals.get_children_count()) {

      int dimensionality = rhs_vals.get_children_count();
      uint16 vals_index = 2 + context.setCompoundResultHead(Atom::Object(VecOpcode, 1));
      context.addCompoundResultPart(Atom::IPointer(vals_index));

      context.addCompoundResultPart(Atom::Set(dimensionality));
      for (int i = 1; i <= dimensionality; ++i) {
        context.addCompoundResultPart(Atom::Float((*lhs_vals.get_child(i))[0].asFloat() - (*rhs_vals.get_child(i))[0].asFloat()));
      }
      return true;
    }
  }

  if (lhs[0].asOpcode() == QuatOpcode && rhs[0].asOpcode() == QuatOpcode) {

    context.setCompoundResultHead(Atom::Object(QuatOpcode, 4));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() - (*rhs.get_child(1))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() - (*rhs.get_child(2))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(3))[0].asFloat() - (*rhs.get_child(3))[0].asFloat()));
    context.addCompoundResultPart(Atom::Float((*lhs.get_child(4))[0].asFloat() - (*rhs.get_child(4))[0].asFloat()));
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool mul(const r_exec::Context &context) {

  r_exec::Context lhs = *context.get_child(1);
  r_exec::Context rhs = *context.get_child(2);

  if (lhs[0].isFloat()) {

    if (rhs[0].asOpcode() == Vec3Opcode) {

      context.setCompoundResultHead(Atom::Object(Vec3Opcode, 3));
      context.addCompoundResultPart(Atom::Float(lhs[0].asFloat()*(*rhs.get_child(1))[0].asFloat()));
      context.addCompoundResultPart(Atom::Float(lhs[0].asFloat()*(*rhs.get_child(2))[0].asFloat()));
      context.addCompoundResultPart(Atom::Float(lhs[0].asFloat()*(*rhs.get_child(3))[0].asFloat()));
      return true;
    }

    if (rhs[0].asOpcode() == Vec2Opcode) {

      context.setCompoundResultHead(Atom::Object(Vec2Opcode, 2));
      context.addCompoundResultPart(Atom::Float(lhs[0].asFloat() * (*rhs.get_child(1))[0].asFloat()));
      context.addCompoundResultPart(Atom::Float(lhs[0].asFloat() * (*rhs.get_child(2))[0].asFloat()));
      return true;
    }

    if(rhs[0].asOpcode() == VecOpcode) {

      r_exec::Context rhs_vals = *rhs.get_child(1);
      int dimensionality = rhs_vals.get_children_count();
      uint16 vals_index = 2 + context.setCompoundResultHead(Atom::Object(VecOpcode, 1));
      context.addCompoundResultPart(Atom::IPointer(vals_index));

      context.addCompoundResultPart(Atom::Set(dimensionality));
      for (int i = 1; i <= dimensionality; ++i) {
        context.addCompoundResultPart(Atom::Float(lhs[0].asFloat() * (*rhs_vals.get_child(i))[0].asFloat()));
      }
      return true;
    }
  }
  else if (lhs[0].asOpcode() == Vec3Opcode) {

    if (rhs[0].isFloat()) {

      context.setCompoundResultHead(Atom::Object(Vec3Opcode, 3));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat()*rhs[0].asFloat()));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat()*rhs[0].asFloat()));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(3))[0].asFloat()*rhs[0].asFloat()));
      return true;
    }
  }
  else if (lhs[0].asOpcode() == Vec2Opcode) {

    if (rhs[0].isFloat()) {

      context.setCompoundResultHead(Atom::Object(Vec2Opcode, 2));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() * rhs[0].asFloat()));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() * rhs[0].asFloat()));
      return true;
    }
  }
  else if (lhs[0].asOpcode() == VecOpcode) {

    if (rhs[0].isFloat()) {

      r_exec::Context lhs_vals = *lhs.get_child(1);
      int dimensionality = lhs_vals.get_children_count();
      uint16 vals_index = 2 + context.setCompoundResultHead(Atom::Object(VecOpcode, 1));
      context.addCompoundResultPart(Atom::IPointer(vals_index));

      context.addCompoundResultPart(Atom::Set(dimensionality));
      for (int i = 1; i <= dimensionality; ++i) {
        context.addCompoundResultPart(Atom::Float((*lhs_vals.get_child(i))[0].asFloat() * rhs[0].asFloat()));
      }
      return true;
    }
  }
  else if (lhs[0].asOpcode() == QuatOpcode && rhs[0].asOpcode() == QuatOpcode) {

    float32 w1 = (*lhs.get_child(1))[0].asFloat();
    float32 x1 = (*lhs.get_child(2))[0].asFloat();
    float32 y1 = (*lhs.get_child(3))[0].asFloat();
    float32 z1 = (*lhs.get_child(4))[0].asFloat();

    float32 w2 = (*rhs.get_child(1))[0].asFloat();
    float32 x2 = (*rhs.get_child(2))[0].asFloat();
    float32 y2 = (*rhs.get_child(3))[0].asFloat();
    float32 z2 = (*rhs.get_child(4))[0].asFloat();

    float32 w_new = round((-x1 * x2 - y1 * y2 - z1 * z2 + w1 * w2) * 100.) / 100.;
    float32 x_new = round(( x1 * w2 + y1 * z2 - z1 * y2 + w1 * x2) * 100.) / 100.;
    float32 y_new = round((-x1 * z2 + y1 * w2 + z1 * x2 + w1 * y2) * 100.) / 100.;
    float32 z_new = round(( x1 * y2 - y1 * x2 + z1 * w2 + w1 * z2) * 100.) / 100.;

    context.setCompoundResultHead(Atom::Object(QuatOpcode, 4));
    context.addCompoundResultPart(Atom::Float(w_new));
    context.addCompoundResultPart(Atom::Float(x_new));
    context.addCompoundResultPart(Atom::Float(y_new));
    context.addCompoundResultPart(Atom::Float(z_new));

    return true;
  }
  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool div(const r_exec::Context& context) {

  r_exec::Context lhs = *context.get_child(1);
  r_exec::Context rhs = *context.get_child(2);

  if (lhs[0].asOpcode() == Vec3Opcode) {

    if (rhs[0].isFloat() && rhs[0].asFloat() != 0) {

      context.setCompoundResultHead(Atom::Object(Vec3Opcode, 3));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() / rhs[0].asFloat()));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() / rhs[0].asFloat()));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(3))[0].asFloat() / rhs[0].asFloat()));
      return true;
    }
  }
  else if (lhs[0].asOpcode() == Vec2Opcode) {

    if (rhs[0].isFloat() && rhs[0].asFloat() != 0) {

      context.setCompoundResultHead(Atom::Object(Vec2Opcode, 2));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(1))[0].asFloat() / rhs[0].asFloat()));
      context.addCompoundResultPart(Atom::Float((*lhs.get_child(2))[0].asFloat() / rhs[0].asFloat()));
      return true;
    }
  }
  else if (lhs[0].asOpcode() == VecOpcode) {

    if (rhs[0].isFloat() && rhs[0].asFloat() != 0) {

      r_exec::Context lhs_vals = *lhs.get_child(1);
      int dimensionality = lhs_vals.get_children_count();
      uint16 vals_index = 2 + context.setCompoundResultHead(Atom::Object(VecOpcode, 1));
      context.addCompoundResultPart(Atom::IPointer(vals_index));

      context.addCompoundResultPart(Atom::Set(dimensionality));
      for (int i = 1; i <= dimensionality; ++i) {
        context.addCompoundResultPart(Atom::Float((*lhs_vals.get_child(i))[0].asFloat() / rhs[0].asFloat()));
      }
      return true;
    }
  }
  else if (lhs[0].asOpcode() == QuatOpcode && rhs[0].asOpcode() == QuatOpcode) {

    //taking the conjugate here.
    float32 w1 = (*lhs.get_child(1))[0].asFloat();
    float32 x1 = -(*lhs.get_child(2))[0].asFloat();
    float32 y1 = -(*lhs.get_child(3))[0].asFloat();
    float32 z1 = -(*lhs.get_child(4))[0].asFloat();

    float32 w2 = (*rhs.get_child(1))[0].asFloat();
    float32 x2 = (*rhs.get_child(2))[0].asFloat();
    float32 y2 = (*rhs.get_child(3))[0].asFloat();
    float32 z2 = (*rhs.get_child(4))[0].asFloat();

    // And take the conjugate again
    float32 w_new = round((-x1 * x2 - y1 * y2 - z1 * z2 + w1 * w2) * 100.) / 100.;
    float32 x_new = -round((x1 * w2 + y1 * z2 - z1 * y2 + w1 * x2) * 100.) / 100.;
    float32 y_new = -round((-x1 * z2 + y1 * w2 + z1 * x2 + w1 * y2) * 100.) / 100.;
    float32 z_new = -round((x1 * y2 - y1 * x2 + z1 * w2 + w1 * z2) * 100.) / 100.;

    float32 _1 = w_new < 0 ? -1. : 1.;

    context.setCompoundResultHead(Atom::Object(QuatOpcode, 4));
    context.addCompoundResultPart(Atom::Float(_1 * w_new));
    context.addCompoundResultPart(Atom::Float(_1 * x_new));
    context.addCompoundResultPart(Atom::Float(_1 * y_new));
    context.addCompoundResultPart(Atom::Float(_1 * z_new));

    return true;
  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool dis(const r_exec::Context &context) {

  r_exec::Context lhs = *context.get_child(1);
  r_exec::Context rhs = *context.get_child(2);

  if (lhs[0].asOpcode() == Vec3Opcode && rhs[0].asOpcode() == Vec3Opcode) {

    float32 d1 = (*lhs.get_child(1))[0].asFloat() - (*rhs.get_child(1))[0].asFloat();
    float32 d2 = (*lhs.get_child(2))[0].asFloat() - (*rhs.get_child(2))[0].asFloat();
    float32 d3 = (*lhs.get_child(3))[0].asFloat() - (*rhs.get_child(3))[0].asFloat();

    float32 norm2 = d1 * d1 + d2 * d2 + d3 * d3;
    context.setAtomicResult(Atom::Float(sqrt(norm2)));
    return true;
  }

  if (lhs[0].asOpcode() == Vec2Opcode && rhs[0].asOpcode() == Vec2Opcode) {

    float32 d1 = (*lhs.get_child(1))[0].asFloat() - (*rhs.get_child(1))[0].asFloat();
    float32 d2 = (*lhs.get_child(2))[0].asFloat() - (*rhs.get_child(2))[0].asFloat();

    float32 norm2 = d1 * d1 + d2 * d2;
    context.setAtomicResult(Atom::Float(sqrt(norm2)));
    return true;
  }

  if (lhs[0].asOpcode() == VecOpcode && rhs[0].asOpcode() == VecOpcode) {

    r_exec::Context lhs_vals = *lhs.get_child(1);
    r_exec::Context rhs_vals = *rhs.get_child(1);
    if (lhs_vals.get_children_count() == rhs_vals.get_children_count()) {

      int dimensionality = lhs_vals.get_children_count();
      float32 norm2 = 0;
      for (int i = 1; i <= dimensionality; ++i) {
        float32 d = (*lhs_vals.get_child(i))[0].asFloat() - (*rhs_vals.get_child(i))[0].asFloat();
        norm2 += (d * d);
      }
      context.setAtomicResult(Atom::Float(sqrt(norm2)));
      return true;
    }
  }

  if (lhs[0].asOpcode() == QuatOpcode && rhs[0].asOpcode() == QuatOpcode) {


    float32 w1 = (*lhs.get_child(1))[0].asFloat();
    float32 x1 = (*lhs.get_child(2))[0].asFloat();
    float32 y1 = (*lhs.get_child(3))[0].asFloat();
    float32 z1 = (*lhs.get_child(4))[0].asFloat();

    float32 w2 = (*rhs.get_child(1))[0].asFloat();
    float32 x2 = (*rhs.get_child(2))[0].asFloat();
    float32 y2 = (*rhs.get_child(3))[0].asFloat();
    float32 z2 = (*rhs.get_child(4))[0].asFloat();

    float32 inner_product = x1 * x2 + y1 * y2 + z1 * z2 + w1 * w2;
    float32 dis = acos(2 * inner_product * inner_product - 1);

    context.setAtomicResult(Atom::Float(dis));

  }

  context.setAtomicResult(Atom::Nil());
  return false;
}

////////////////////////////////////////////////////////////////////////////////

}

r_code::resized_vector<uint16> Operators::Init(OpcodeRetriever r) {

  const char* vec3 = "vec3";
  const char* vec2 = "vec2";
  const char* vec = "vec";
  const char* quat = "quat";
  r_code::resized_vector<uint16> initialized_opcodes;
  initialized_opcodes.push_back(Vec3Opcode = r(vec3));
  initialized_opcodes.push_back(Vec2Opcode = r(vec2));
  initialized_opcodes.push_back(VecOpcode = r(vec));
  initialized_opcodes.push_back(QuatOpcode = r(quat));

  return initialized_opcodes;
}
