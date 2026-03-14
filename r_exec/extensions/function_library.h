#ifndef function_library_h
#define function_library_h

#include "dll.h"
#include "utils.h"

namespace r_exec {

class FunctionLibrary {
public:
  virtual void* getFunction(const char* function_name) = 0;
};

class SharedFunctionLibrary : public FunctionLibrary {
public:
  SharedFunctionLibrary() : getUserOperatorFunction_(0) {}

  core::SharedLibrary* load(const char* file_name) {
    core::SharedLibrary* library = sharedLibrary_.load(file_name);
    getUserOperatorFunction_ = (void* (*)(const char*))sharedLibrary_.getFunction("GetUserOperatorFunction");
    return library;
  }

  void* getFunction(const char* function_name) override {
    if (getUserOperatorFunction_) {
      void* result = getUserOperatorFunction_(function_name);
      if (result)
        return result;
    }

    return sharedLibrary_.getFunction(function_name);
  }
private:
  core::SharedLibrary sharedLibrary_;
  void* (*getUserOperatorFunction_)(const char* function_name);
};

}  // namespace r_exec

#endif
