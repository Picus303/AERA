

#include <memory>
#include "base.h"
#include "utils.h"


namespace core {

#ifdef WITH_DETAIL_OID
// Start with a non-zero value so that it doesn't appear to track object OIDs.
static uint64 last_detail_oid = 10;
#endif

_Object::_Object() : refCount_(0) {
#ifdef WITH_DETAIL_OID
  detail_oid_ = ++last_detail_oid;
  if (detail_oid_ == 0)
    int set_breakpoint_here = 1;
#endif
}

_Object::~_Object() {
}

#ifdef WITH_DETAIL_OID
void _Object::set_detail_oid(uint64 detail_oid) {
  detail_oid_ = detail_oid;
  // Make sure the next assigned detail OID is higher.
  last_detail_oid = detail_oid;
}
#endif

void _Object::incRef() {

  ++refCount_;
}

void _Object::decRef() {

  if (--refCount_ == 0)
    delete this;
}
}
