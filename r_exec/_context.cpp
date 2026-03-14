

#include "_context.h"

using namespace std::chrono;
using namespace r_code;

namespace r_exec {

void _Context::setAtomicResult(Atom a) const { // patch code with 32 bits data.

  overlay_->patch_code(index_, a);
}

void _Context::setTimestampResult(Timestamp t) const { // patch code with a VALUE_PTR

  overlay_->patch_code(index_, Atom::ValuePointer(overlay_->values_.size()));
  overlay_->values_.resize(overlay_->values_.size() + 3);
  uint16 value_index = overlay_->values_.size() - 3;
  Utils::SetTimestamp(&overlay_->values_[value_index], t);
}

void _Context::setDurationResult(microseconds d) const { // patch code with a VALUE_PTR
  overlay_->patch_code(index_, Atom::ValuePointer(overlay_->values_.size()));
  overlay_->values_.resize(overlay_->values_.size() + 3);
  uint16 value_index = overlay_->values_.size() - 3;
  Utils::SetDuration(&overlay_->values_[value_index], d);
}

uint16 _Context::setCompoundResultHead(Atom a) const { // patch code with a VALUE_PTR.

  uint16 value_index = overlay_->values_.size();
  overlay_->patch_code(index_, Atom::ValuePointer(value_index));
  addCompoundResultPart(a);
  return value_index;
}

void _Context::addCompoundResultPart(Atom a) const { // store result in the value array.

  overlay_->values_.push_back(a);
}

void _Context::trace(std::ostream& out) const {

  out << "======== CONTEXT ========\n";
  switch (data_) {
  case UNDEFINED:
    out << "undefined\n";
    return;
  case MKS:
    out << "--> mks\n";
    return;
  case VWS:
    out << "--> vws\n";
    return;
  }

  Atom::TraceContext context;
  for (uint16 i = 0; i < get_object_code_size(); ++i) {

    if (index_ == i)
      out << ">>";
    out << i << "\t";
    code_[i].trace(context, out);
    out << std::endl;
  }
}
}
