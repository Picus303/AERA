

#ifndef operator_h
#define operator_h

#include "../r_code/object.h"
#include "../r_code/utils.h"
#include "factory.h"

#include "_context.h"


namespace r_exec {

// Wrapper class for evaluation contexts.
// Template operator functions is not an option since some operators are defined in usr_operators.dll.
class dll_export Context {
private:
  _Context *implementation_;
public:
  Context(_Context *implementation) : implementation_(implementation) {}
  ~Context() { delete implementation_; }

  _Context *get_implementation() const { return implementation_; }

  uint16 get_children_count() const { return implementation_->get_children_count(); }
  Context get_child(uint16 index) const { return Context(implementation_->get_child_new(index)); }

  Context operator *() const { return Context(implementation_->dereference_new()); }
  Context &operator =(const Context &c) {

    // Copy the existing implementation before deleting it.
    _Context* copy = c.get_implementation()->clone();
    delete implementation_;
    implementation_ = copy;
    return *this;
  }

  bool operator ==(const Context &c) const { return implementation_->equal(c.get_implementation()); }
  bool operator !=(const Context &c) const { return !implementation_->equal(c.get_implementation()); }

  Atom &operator [](uint16 i) const { return implementation_->get_atom(i); }

  virtual void setAtomicResult(Atom a) const { implementation_->setAtomicResult(a); }
  virtual void setTimestampResult(Timestamp t) const { implementation_->setTimestampResult(t); }
  virtual void setDurationResult(std::chrono::microseconds d) const { implementation_->setDurationResult(d); }
  virtual uint16 setCompoundResultHead(Atom a) const { return implementation_->setCompoundResultHead(a); }
  virtual void addCompoundResultPart(Atom a) const { implementation_->addCompoundResultPart(a); }

  void trace(std::ostream& out) const { return implementation_->trace(out); }
};

class OpContext : public Context {
private:
  std::vector<r_code::Atom> operation_results_;

  std::vector<Atom>& operation_results() const {
    return const_cast<OpContext*>(this)->operation_results_;
  }

public:
  OpContext(_Context* implementation) : Context(implementation) {}
  ~OpContext() {}

  void setAtomicResult(Atom a) const override {
    Context::setAtomicResult(a);
    operation_results().push_back(a);
  }
  void setTimestampResult(Timestamp t) const override {
    Context::setTimestampResult(t);
    operation_results().resize(operation_results_.size() + 3);
    uint16 value_index = operation_results().size() - 3;
    r_code::Utils::SetTimestamp(&operation_results()[value_index], t);
  }
  void setDurationResult(std::chrono::microseconds d) const override {
    Context::setDurationResult(d);
    operation_results().resize(operation_results_.size() + 3);
    uint16 value_index = operation_results().size() - 3;
    r_code::Utils::SetDuration(&operation_results()[value_index], d);
  }
  uint16 setCompoundResultHead(Atom a) const override {
    uint16 value_index = Context::setCompoundResultHead(a);
    addCompoundResultPart(a);
    return value_index;
  }
  void addCompoundResultPart(Atom a) const override{
    Context::addCompoundResultPart(a);
    operation_results().push_back(a);
  }

  const std::vector<r_code::Atom>& result() { return operation_results_; }

  static std::vector<r_code::Atom> build_and_evaluate_expression(_Fact* q0, _Fact* q1, r_code::Atom op);
  static P<r_code::LocalObject> build_expression_object(_Fact* q0, _Fact* q1, r_code::Atom op);
  static std::vector<r_code::Atom> evaluate_expression(r_code::LocalObject* expression);
};

bool red(const Context &context); // executive-dependent.

bool syn(const Context &context);

class Operator {
private:
  static r_code::resized_vector<Operator> Operators_; // indexed by opcodes.

  bool(*operator_)(const Context &);
  bool(*overload_)(const Context &);
public:
  static void Register(uint16 opcode, bool(*op)(const Context &)); // first, register std operators; next register user-defined operators (may be registered as overloads).
  static Operator Get(uint16 opcode) { return Operators_[opcode]; }
  Operator() : operator_(NULL), overload_(NULL) {}
  Operator(bool(*o)(const Context &)) : operator_(o), overload_(NULL) {}
  ~Operator() {}

  void setOverload(bool(*o)(const Context &)) { overload_ = o; }

  bool operator ()(const Context &context) const {
    if (operator_(context))
      return true;
    if (overload_)
      return overload_(context);
    return false;
  }

  bool is_red() const { return operator_ == red; }
  bool is_syn() const { return operator_ == syn; }
};

// std operators ////////////////////////////////////////

bool now(const Context &context);

bool rnd(const Context &context);

bool equ(const Context &context);
bool neq(const Context &context);
bool gtr(const Context &context);
bool lsr(const Context &context);
bool gte(const Context &context);
bool lse(const Context &context);

bool add(const Context &context);
bool sub(const Context &context);
bool mul(const Context &context);
bool div(const Context &context);

bool dis(const Context &context);

bool ln(const Context &context);
bool exp(const Context &context);
bool log(const Context &context);
bool e10(const Context &context);

bool ins(const Context &context); // executive-dependent.

bool fvw(const Context &context); // executive-dependent.

bool is_sim(const Context &context);
bool minimum(const Context &context);
bool maximum(const Context &context);
bool id(const Context& context);
}


#endif
