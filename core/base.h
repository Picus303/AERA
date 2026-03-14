

#ifndef core_base_h
#define core_base_h

//#define WITH_DETAIL_OID // Enable get_detail_oid() in every object.

#include <cstdlib>
#include <atomic>

#include "types.h"

namespace core {

class _Object;

// Smart pointer (ref counting, deallocates when ref count<=0).
// No circular refs (use std c++ ptrs).
// No passing in functions (cast P<C> into C*).
// Cannot be a value returned by a function (return C* instead).
template<class C> class P {
private:
  _Object *object_;
public:
  P();
  P(C *o);
  P(const P<C> &p);
  ~P();
  C *operator ->() const;
  template<class D> operator D *() const {

    return static_cast<D *>((C *)object_);
  }
  bool operator ==(C *c) const;
  bool operator !=(C *c) const;
  bool operator !() const;
  bool operator <(P<C> &p) const       { return (size_t)object_ < (size_t)p.object_; }
  bool operator <(const P<C> &p) const { return (size_t)object_ < (size_t)p.object_; }
  template<class D> bool operator ==(P<D> &p) const;
  template<class D> bool operator !=(P<D> &p) const;
  P<C> &operator =(C *c);
  P<C> &operator =(const  P<C> &p);
  template<class D> P<C> &operator =(const P<D> &p);
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Root smart-pointable object class.
class core_dll _Object {
  template<class C> friend class P;
  friend class _P;
protected:
#ifdef ARCH_32
  uint32 __vfptr_padding_Object_;
#endif
  std::atomic_int32_t refCount_;
  _Object();
#ifdef WITH_DETAIL_OID
  uint64 detail_oid_;
#endif
public:
  virtual ~_Object();
  void incRef();
  virtual void decRef();
#ifdef WITH_DETAIL_OID
  uint64 get_detail_oid() const { return detail_oid_; }

  /**
   * Set this object's detail OID and also set the static last_detail_oid
   * so that the next detail OID will be higher than this one.
   * \param detail_oid The detail OID.
   */
  void set_detail_oid(uint64 detail_oid);
#endif
};

// Template version of the well-known DP. Adapts C to _Object.
template<class C> class _ObjectAdapter :
  public C,
  public _Object {
protected:
  _ObjectAdapter();
};
}


#include "base.tpl.cpp"


#endif
