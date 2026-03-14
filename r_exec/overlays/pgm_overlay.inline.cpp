

namespace r_exec {

inline r_code::Code *PGMOverlay::getInputObject(uint16 i) const {

  return input_views_[i]->object_;
}

inline r_code::_View *PGMOverlay::getInputView(uint16 i) const {

  return (r_code::_View *)input_views_[i];
}
}
