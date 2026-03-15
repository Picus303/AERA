

#include <iostream>

#if defined (WINDOWS)
#elif defined (LINUX)
#include <dlfcn.h>
#else
#error "Not yet ported to your platform"
#endif

namespace core {

template<class T> T *Thread::New(thread_function f, void *args) {

  T *t = new T();
#if defined WINDOWS
  t->thread_ = CreateThread(NULL, 0, f, args, 0, NULL);
  if (t->thread_)
#elif defined LINUX
  if (pthread_create(&t->thread_, NULL, f, args) == 0)
#endif
    return t;
  delete t;
  return NULL;
}
}
