

namespace r_exec {

template<class C, class U> Object<C, U>::Object() : C(), hash_value_(0), invalidated_(0) {
}

template<class C, class U> Object<C, U>::Object(r_code::Mem* /* mem */) : C(), hash_value_(0), invalidated_(0) {

  C::set_oid(UNDEFINED_OID);
}

template<class C, class U> Object<C, U>::~Object() {

  invalidate();
}

template<class C, class U> bool Object<C, U>::is_invalidated() {

  return invalidated_ == 1;
}

template<class C, class U> bool Object<C, U>::invalidate() {

  if (invalidated_)
    return true;
  invalidated_ = 1;

  acq_views();
  C::views_.clear();
  rel_views();

  if (C::code(0).getDescriptor() == Atom::MARKER) {

    for (uint16 i = 0; i < C::references_size(); ++i)
      C::get_reference(i)->remove_marker(this);
  }

  if (C::is_registered())
    r_code::Mem::Get()->delete_object(this);

  return false;
}

template<class C, class U> void Object<C, U>::compute_hash_value() {

  hash_value_ = C::code(0).asOpcode() << 20; // 12 bits for the opcode.
  hash_value_ |= (C::code_size() & 0x00000FFF) << 8; // 12 bits for the code size.
  hash_value_ |= C::references_size() & 0x000000FF; // 8 bits for the reference set size.
}

template<class C, class U> float32 Object<C, U>::get_psln_thr() {

  psln_thrCS_.enter();
  float32 r = C::code(C::code(0).getAtomCount()).asFloat(); // psln is always the last member of an object.
  psln_thrCS_.leave();
  return r;
}

template<class C, class U> void Object<C, U>::mod(uint16 member_index, float32 value) {

  if (member_index != C::code_size() - 1)
    return;
  float32 v = C::code(member_index).asFloat() + value;
  if (v < 0)
    v = 0;
  else if (v > 1)
    v = 1;

  psln_thrCS_.enter();
  C::code(member_index) = Atom::Float(v);
  psln_thrCS_.leave();
}

template<class C, class U> void Object<C, U>::set(uint16 member_index, float32 value) {

  if (member_index != C::code_size() - 1)
    return;

  psln_thrCS_.enter();
  C::code(member_index) = Atom::Float(value);
  psln_thrCS_.leave();
}

template<class C, class U> r_code::_View *Object<C, U>::get_view(r_code::Code *group, bool lock) {

  if (lock)
    acq_views();

  r_code::_View probe;
  probe.references_[0] = group;

  std::unordered_set<r_code::_View *, r_code::_View::Hash, r_code::_View::Equal>::const_iterator v = C::views_.find(&probe);
  if (v != C::views_.end()) {

    if (lock)
      rel_views();
    return (r_exec::View *)*v;
  }

  if (lock)
    rel_views();
  return NULL;
}
}
