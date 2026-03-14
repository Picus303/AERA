

#include "overlay.h"
#include "controller.h"
#include "runtime/mem.h"

using namespace std::chrono;
using namespace r_code;

#define MAX_VALUE_SIZE 128

namespace r_exec {

Overlay::Overlay()
  // MAX_VALUE_SIZE is the limit; if the array is resized later on, some contexts with data==VALUE_ARRAY
  // may point to invalid adresses: case of embedded contexts with both data==VALUE_ARRAY.
  : Overlay(MAX_VALUE_SIZE)
{}

Overlay::Overlay(size_t values_size) : _Object(), invalidated_(0) {

  code_ = new r_code::Atom[1];
  values_.resize(values_size);
}

Overlay::Overlay(Controller *c, bool load_code) : _Object(), controller_(c), value_commit_index_(0), code_(NULL), invalidated_(0) {

  values_.resize(128);
  if (load_code)
    this->load_code();
}

Overlay::~Overlay() {

  if (code_)
    delete[] code_;
}

Code* Overlay::get_object() const { return ((Controller*)controller_)->get_object(); }
r_exec::View* Overlay::get_view() const { return ((Controller*)controller_)->get_view(); }

inline Code *Overlay::get_core_object() const {

  return controller_->get_core_object();
}

void Overlay::load_code() {

  if (code_)
    delete[] code_;

  Code *object = get_core_object();
  // copy the original pgm/hlp code.
  code_size_ = object->code_size();
  code_ = new r_code::Atom[code_size_];
  memcpy(code_, &object->code(0), code_size_ * sizeof(r_code::Atom));
}

void Overlay::reset() {

  memcpy(code_, &get_object()->get_reference(0)->code(0), code_size_ * sizeof(r_code::Atom)); // restore code to prisitne copy.
}

void Overlay::rollback() {

  Code *object = get_core_object();
  Atom *original_code = &object->code(0);
  for (uint16 i = 0; i < patch_indices_.size(); ++i) // upatch code.
    code_[patch_indices_[i]] = original_code[patch_indices_[i]];
  patch_indices_.clear();

  if (value_commit_index_ != values_.size()) { // shrink the values down to the last commit index.

    if (value_commit_index_ > 0)
      values_.resize(value_commit_index_);
    else
      values_.clear();
    value_commit_index_ = values_.size();
  }
}

void Overlay::commit() {

  patch_indices_.clear();
  value_commit_index_ = values_.size();
}

void Overlay::patch_code(uint16 index, Atom value) {

  code_[index] = value;
  patch_indices_.push_back(index);
}

uint16 Overlay::get_last_patch_index() {

  return patch_indices_.size();
}

void Overlay::unpatch_code(uint16 patch_index) {

  Code *object = get_core_object();
  Atom *original_code = &object->code(0);
  for (uint16 i = patch_index; i < patch_indices_.size(); ++i)
    code_[patch_indices_[i]] = original_code[patch_indices_[i]];
  patch_indices_.resize(patch_index);
}

Overlay *Overlay::reduce(r_exec::View *input) {

  return NULL;
}

r_code::Code *Overlay::build_object(Atom head) const {

  return _Mem::Get()->build_object(head);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Controller::Controller(_View *view) : _Object(), invalidated_(0), activated_(0), view_(view) {

  if (!view)
    return;

  switch (get_object()->code(0).getDescriptor()) {
  case Atom::INSTANTIATED_PROGRAM:
  case Atom::INSTANTIATED_INPUT_LESS_PROGRAM:
  case Atom::INSTANTIATED_ANTI_PROGRAM:
    time_scope_ = Utils::GetDuration<Code>(get_object(), IPGM_TSC);
    break;
  case Atom::INSTANTIATED_CPP_PROGRAM:
    time_scope_ = Utils::GetDuration<Code>(get_object(), ICPP_PGM_TSC);
    break;
  }
}

Controller::~Controller() {
}

void Controller::set_view(View *view) {

  view_ = view;
}

void Controller::_take_input(r_exec::View *input) { // called by groups at update and injection time.

  if (is_alive() && !input->object_->is_invalidated())
    take_input(input);
}

// Put this in the cpp file to avoid an include loop between controller.h and mem.h.
void Controller::push_reduction_job(_ReductionJob* j) {
  _Mem::Get()->push_reduction_job(j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

OController::OController(_View *view) : Controller(view) {
}

OController::~OController() {
}
}
