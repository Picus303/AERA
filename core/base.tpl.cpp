

namespace core {

template<class C> inline P<C>::P() : object_(NULL) {
}

template<class C> inline P<C>::P(C *o) : object_(o) {

  if (object_)
    object_->incRef();
}

template<class C> inline P<C>::P(const P<C> &p) : object_(p.object_) {

  if (object_)
    object_->incRef();
}

template<class C> inline P<C>::~P() {

  if (object_)
    object_->decRef();
}

template<class C> inline C *P<C>::operator ->() const {

  return (C *)object_;
}

template<class C> inline bool P<C>::operator ==(C *c) const {

  return object_ == c;
}

template<class C> inline bool P<C>::operator !=(C *c) const {

  return object_ != c;
}

template<class C> template<class D> inline bool P<C>::operator ==(P<D> &p) const {

  return object_ == p.object_;
}

template<class C> template<class D> inline bool P<C>::operator !=(P<D> &p) const {

  return object_ != p.object_;
}

template<class C> inline bool P<C>::operator !() const {

  return !object_;
}

template<class C> inline P<C>& P<C>::operator =(C *c) {

  if (object_ == c)
    return *this;
  if (object_)
    object_->decRef();
  object_ = c;
  if (object_)
    object_->incRef();

  return *this;
}

template<class C> template<class D> inline P<C> &P<C>::operator =(const P<D> &p) {

  return this->operator =(static_cast<C *>((D *)p.object_));
}

template<class C> inline P<C> &P<C>::operator =(const P<C> &p) {

  return this->operator =((C *)p.object_);
}

////////////////////////////////////////////////////////////////////////////////////

template<class C> inline _ObjectAdapter<C>::_ObjectAdapter() : _Object(), C() {
}
}
