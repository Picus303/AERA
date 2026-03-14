

#ifndef core_pipe_h
#define core_pipe_h

#include "utils.h"


#define PIPE_1

namespace core {

// Pipes are thread safe, depending on their type:
// 11: 1 writer, 1 reader
// 1N: 1 writer, N readers
// N1: N writers, 1 reader
// NN: N writers, N readers
#ifdef PIPE_1
template<typename T, uint32 _S> class Pipe11 :
  public Semaphore,
  public CriticalSection {
private:
  class Block {
  public:
    T buffer_[_S * sizeof(T)];
    Block *next_;
    Block(Block *prev) : next_(NULL) { if (prev) prev->next_ = this; }
    ~Block() { if (next_) delete next_; }
  };
  int32 head_;
  int32 tail_;
  Block *first_;
  Block *last_;
  Block *spare_;
protected:
  void _clear();
  T _pop();
public:
  Pipe11();
  ~Pipe11();
  void clear();
  void push(T &t); // increases the size as necessary
  T pop(); // decreases the size as necessary
};

template<typename T, uint32 _S> class Pipe1N :
  public Pipe11<T, _S> {
private:
  CriticalSection popCS_;
public:
  Pipe1N();
  ~Pipe1N();
  void clear();
  T pop();
};

template<typename T, uint32 _S> class PipeN1 :
  public Pipe11<T, _S> {
private:
  CriticalSection pushCS_;
public:
  PipeN1();
  ~PipeN1();
  void clear();
  void push(T &t);
};

template<typename T, uint32 _S> class PipeNN :
  public Pipe11<T, _S> {
private:
  CriticalSection pushCS_;
  CriticalSection popCS_;
public:
  PipeNN();
  ~PipeNN();
  void clear();
  void push(T &t);

  /**
   * Pop the head item.
   * \param waitForItem (optional) If true and the pipe is empty, block until
   * another thread adds an item with push(). If false and the pipe is empty,
   * return NULL immediately.If omitted, then wait for an item.
   * \return The popped item, or NULL if waitForItem is false and the pipe is empty.
   */
  T pop(bool waitForItem = true);
};
#elif defined PIPE_2
template<typename T, uint32 _S, class Pipe> class Push1;
template<typename T, uint32 _S, class Pipe> class PushN;
template<typename T, uint32 _S, class Pipe> class Pop1;
template<typename T, uint32 _S, class Pipe> class PopN;

// a Pipe<T,_S> is a linked list of blocks containing _S objects of type T
// push() adds an object at the tail of the last block and moves the tail forward; when the tail reaches the end of the last block, a new block is appended to the list
// pop() moves the head forward and returns the object at this location; when the head reaches the end of the first block, this block is deallocated
// Synchronization between readers and writers is lock-free under no contention (reqCount), and uses a lock (Semaphore::) under contention
// single writer pipes can use an int32 tail, whereas multiple writer versions require int32 volatile tail; idem for readers
// The Head and Tail argument is meant to allow the parameterizing of heads and tails
// Push and Pop are functors tailored to the multiplicity of resp. the read and write threads
// P is the actual pipe class (e.g. Pipe11, etc.)
template<typename T, uint32 _S, typename Head, typename Tail, class P, template<typename, uint32, class> class Push, template<typename, uint32, class> class Pop> class Pipe :
  public Semaphore {
  template<typename T, uint32 _S, class Pipe> friend class Push1;
  template<typename T, uint32 _S, class Pipe> friend class PushN;
  template<typename T, uint32 _S, class Pipe> friend class Pop1;
  template<typename T, uint32 _S, class Pipe> friend class PopN;
protected:
  class Block {
  public:
    T buffer_[_S];
    Block *next_;
    Block(Block *prev) : next_(NULL) { if (prev) prev->next_ = this; }
    ~Block() { if (next_) delete next_; }
  };
  Block *first_;
  Block *last_;
  Block *spare_; // pipes always retain a spare block: if a block is to be deallocated and there is no spare, it becomes the spare

  Head head_; // starts at -1
  Tail tail_; // starts at 0
  int32 volatile waitingList_; // amount of readers that have to wait, negative value indicate free lunch

  Push<T, _S, P> *_push;
  Pop<T, _S, P> *_pop;

  void shrink(); // deallocates the first block when head_ reaches its end
  void grow(); // allocate a new last block when tail_ reaches its end

  Pipe();
public:
  ~Pipe();
  void push(T &t);
  T pop();
};

template<class Pipe> class PipeFunctor {
protected:
  Pipe &const pipe;
  PipeFunctor(Pipe &p);
};

template<typename T, uint32 _S, class Pipe> class Push1 :
  public PipeFunctor<Pipe> {
public:
  Push1(Pipe &p);
  void operator ()(T &t);
};

template<typename T, uint32 _S, class Pipe> class PushN :
  public PipeFunctor<Pipe>,
  public Semaphore {
public:
  PushN(Pipe &p);
  void operator ()(T &t);
};

template<typename T, uint32 _S, class Pipe> class Pop1 :
  public PipeFunctor<Pipe> {
public:
  Pop1(Pipe &p);
  T operator ()();
};

template<typename T, uint32 _S, class Pipe> class PopN :
  public PipeFunctor<Pipe>,
  public Semaphore {
public:
  PopN(Pipe &p);
  T operator ()();
};

template<typename T, uint32 S> class Pipe11 :
  public Pipe<T, S, int32, int32, Pipe11<T, S>, Push1, Pop1> {
public:
  Pipe11();
  ~Pipe11();
};

template<typename T, uint32 S> class Pipe1N :
  public Pipe<T, S, int32, int32 volatile, Pipe1N<T, S>, Push1, PopN> {
public:
  Pipe1N();
  ~Pipe1N();
};

template<typename T, uint32 S> class PipeN1 :
  public Pipe<T, S, int32 volatile, int32, PipeN1<T, S>, PushN, Pop1> {
public:
  PipeN1();
  ~PipeN1();
};

template<typename T, uint32 S> class PipeNN :
  public Pipe<T, S, int32 volatile, int32 volatile, PipeNN<T, S>, PushN, PopN> {
public:
  PipeNN();
  ~PipeNN();
};
#endif
}


#include "pipe.tpl.cpp"


#endif
