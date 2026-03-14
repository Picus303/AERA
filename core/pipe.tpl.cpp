

#include <memory.h>


namespace core {

#ifdef PIPE_1
template<typename T, uint32 _S> Pipe11<T, _S>::Pipe11() : Semaphore(0, 65535) {

  head_ = tail_ = -1;
  first_ = last_ = new Block(NULL);
  spare_ = NULL;
}

template<typename T, uint32 _S> Pipe11<T, _S>::~Pipe11() {

  delete first_;
  if (spare_)
    delete spare_;
}

template<typename T, uint32 _S> inline void Pipe11<T, _S>::_clear() { // leaves spare_ as is

  enter();
  reset();
  if (first_->next_)
    delete first_->next_;
  first_->next_ = NULL;
  head_ = tail_ = -1;
  leave();
}

template<typename T, uint32 _S> inline T Pipe11<T, _S>::_pop() {

  T t = first_->buffer_[head_];
  if (++head_ == _S) {

    enter();
    if (first_ == last_)
      head_ = tail_ = -1; // stay in the same block; next push will reset head_ and tail_ to 0
    else {

      if (!spare_) {

        spare_ = first_;
        first_ = first_->next_;
        spare_->next_ = NULL;
      } else {

        Block *b = first_->next_;
        first_->next_ = NULL;
        delete first_;
        first_ = b;
      }
      head_ = 0;
    }
    leave();
  }
  return t;
}

template<typename T, uint32 _S> inline void Pipe11<T, _S>::push(T &t) {

  enter();
  if (++tail_ == 0)
    head_ = 0;
  uint32 index = tail_;
  if (tail_ == _S) {

    if (spare_) {

      last_->next_ = spare_;
      last_ = spare_;
      last_->next_ = NULL;
      spare_ = NULL;
    } else
      last_ = new Block(last_);
    tail_ = 0;
    index = tail_;
  }
  leave();

  last_->buffer_[index] = t;
  release();
}

template<typename T, uint32 _S> inline T Pipe11<T, _S>::pop() {

  Semaphore::acquire();
  return _pop();
}

template<typename T, uint32 _S> inline void Pipe11<T, _S>::clear() {

  _clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S> Pipe1N<T, _S>::Pipe1N() {
}

template<typename T, uint32 _S> Pipe1N<T, _S>::~Pipe1N() {
}

template<typename T, uint32 _S> void Pipe1N<T, _S>::clear() {

  popCS_.enter();
  Pipe11<T, _S>::_clear();
  popCS_.leave();
}

template<typename T, uint32 _S> T Pipe1N<T, _S>::pop() {

  Semaphore::acquire();
  popCS_.enter();
  T t = Pipe11<T, _S>::_pop();
  popCS_.leave();
  return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S> PipeN1<T, _S>::PipeN1() {
}

template<typename T, uint32 _S> PipeN1<T, _S>::~PipeN1() {
}

template<typename T, uint32 _S> void PipeN1<T, _S>::clear() {

  pushCS_.enter();
  Pipe11<T, _S>::_clear();
  pushCS_.leave();
}

template<typename T, uint32 _S> void PipeN1<T, _S>::push(T &t) {

  pushCS_.enter();
  Pipe11<T, _S>::push(t);
  pushCS_.leave();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S> PipeNN<T, _S>::PipeNN() {
}

template<typename T, uint32 _S> PipeNN<T, _S>::~PipeNN() {
}

template<typename T, uint32 _S> void PipeNN<T, _S>::clear() {

  pushCS_.enter();
  popCS_.enter();
  Pipe11<T, _S>::_clear();
  popCS_.leave();
  pushCS_.leave();
}

template<typename T, uint32 _S> void PipeNN<T, _S>::push(T &t) {

  pushCS_.enter();
  Pipe11<T, _S>::push(t);
  pushCS_.leave();
}

template<typename T, uint32 _S> T PipeNN<T, _S>::pop(bool waitForItem) {

  if (waitForItem)
    Semaphore::acquire();
  else {
    // Use 0 timeout.
    if (Semaphore::acquire(0))
      // A timeout means there are no items.
      return NULL;
  }
  popCS_.enter();
  T t = Pipe11<T, _S>::_pop();
  popCS_.leave();
  return t;
}
#elif defined PIPE_2
template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> Pipe<T, _S, Head, Tail, P, Push, Pop>::Pipe() : Semaphore(0, 1) {

  head_ = -1;
  tail_ = 0;

  waitingList_ = 0;

  first_ = last_ = new Block(NULL);
  spare_ = new Block(NULL);

  _push = new Push<T, _S, P>(*(P *)this);
  _pop = new Pop<T, _S, P>(*(P *)this);
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> Pipe<T, _S, Head, Tail, P, Push, Pop>::~Pipe() {

  delete first_;
  if (spare_)
    delete spare_;
  delete _push;
  delete _pop;
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline void Pipe<T, _S, Head, Tail, P, Push, Pop>::shrink() {

  if (!spare_) {

    spare_ = first_;
    first_ = first_->next_;
    spare_->next_ = NULL;
  } else {

    Block *b = first_->next_;
    first_->next_ = NULL;
    delete first_;
    first_ = b;
  }
  head_ = -1;
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline void Pipe<T, _S, Head, Tail, P, Push, Pop>::grow() {

  if (spare_) {

    last_->next_ = spare_;
    last_ = spare_;
    last_->next_ = NULL;
    spare_ = NULL;
  } else
    last_ = new Block(last_);
  tail_ = 0;
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline void Pipe<T, _S, Head, Tail, P, Push, Pop>::push(T &t) {

  (*_push)(t);
}

template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> inline T Pipe<T, _S, Head, Tail, P, Push, Pop>::pop() {

  return (*_pop)();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<class Pipe> PipeFunctor<Pipe>::PipeFunctor(Pipe &p) : pipe(p) {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> Push1<T, _S, Pipe>::Push1(Pipe &p) : PipeFunctor<Pipe>(p) {
}

template<typename T, uint32 _S, class Pipe> void Push1<T, _S, Pipe>::operator ()(T &t) {

  pipe.last_->buffer_[pipe.tail_] = t;

  if (++pipe.tail_ == (int32)_S)
    pipe.grow();

  int32 count = Atomic::Decrement32(&pipe.waitingList_);
  if (count >= 0) // at least one reader is waiting
    pipe.release(); // unlock one reader
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> PushN<T, _S, Pipe>::PushN(Pipe &p) : PipeFunctor<Pipe>(p), Semaphore(0, 1) {
}

template<typename T, uint32 _S, class Pipe> void PushN<T, _S, Pipe>::operator ()(T &t) {

check_tail: int32 tail = Atomic::Increment32(&pipe.tail_) - 1;

  if (tail < (int32)_S)
    pipe.last_->buffer_[tail] = t;
  else if (tail == (int32)_S) {

    pipe.grow(); // tail set to 0

    pipe.last_->buffer_[pipe.tail_++] = t;

    release(); // unlock writers
    acquire(); // make sure the sem falls back to 0
  } else { // tail>_S: pipe.last_ and pipe.tail_ are being changed

    acquire(); // wait
    release(); // unlock other writers
    goto check_tail;
  }

  int32 count = Atomic::Decrement32(&pipe.waitingList_);
  if (count >= 0) // at least one reader is waiting
    pipe.release(); // unlock one reader
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> Pop1<T, _S, Pipe>::Pop1(Pipe &p) : PipeFunctor<Pipe>(p) {
}

template<typename T, uint32 _S, class Pipe> T Pop1<T, _S, Pipe>::operator ()() {

  int32 count = Atomic::Increment32(&pipe.waitingList_);
  if (count > 0) // no free lunch
    pipe.acquire(); // wait for a push

  if (pipe.head_ == (int32)_S - 1)
    pipe.shrink();

  T t = pipe.first_->buffer_[++pipe.head_];

  return t;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 _S, class Pipe> PopN<T, _S, Pipe>::PopN(Pipe &p) : PipeFunctor<Pipe>(p), Semaphore(0, 1) {
}

template<typename T, uint32 _S, class Pipe> T PopN<T, _S, Pipe>::operator ()() {

  int32 count = Atomic::Increment32(&pipe.waitingList_);
  if (count > 0) // no free lunch
    pipe.acquire(); // wait for a push

check_head: int32 head = Atomic::Increment32(&pipe.head_);
  if (head < (int32)_S)
    return pipe.first_->buffer_[head];

  if (head == (int32)_S) {

    pipe.shrink(); // head set to -1

    release(); // unlock readers
    acquire(); // make sure the sem falls back to 0

    return pipe.first_->buffer_[++pipe.head_];
  } else { // head>_S: pipe.first_ and pipe.head_ are being changed

    acquire(); // wait
    release(); // unlock other readers
    goto check_head;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> Pipe11<T, S>::Pipe11() : Pipe<T, S, int32, int32, Pipe11<T, S>, Push1, Pop1>() {
}

template<typename T, uint32 S> Pipe11<T, S>::~Pipe11() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> Pipe1N<T, S>::Pipe1N() : Pipe<T, S, int32, int32 volatile, Pipe1N, Push1, PopN>() {
}

template<typename T, uint32 S> Pipe1N<T, S>::~Pipe1N() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> PipeN1<T, S>::PipeN1() : Pipe<T, S, int32 volatile, int32, PipeN1, PushN, Pop1>() {
}

template<typename T, uint32 S> PipeN1<T, S>::~PipeN1() {
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template<typename T, uint32 S> PipeNN<T, S>::PipeNN() : Pipe<T, S, int32 volatile, int32 volatile, PipeNN, PushN, PopN>() {
}

template<typename T, uint32 S> PipeNN<T, S>::~PipeNN() {
}
#endif
}
