#pragma once

// Helper functions for error handling
// 2.5.6 Error handling
// 3.6 Error handling

#include "exception.h"
#include "detail/common.h"
#include "detail/debug.h"
#include "detail/error_code.h"

namespace cl {
namespace sycl {

// Forward declaration
class context;

namespace detail {

// 3.6.1, paragraph 4
// If an asynchronous error occurs in a queue
// that has no user-supplied asynchronous error handler object,
// then no exception is thrown
// and the error is not available to the user in any specified way.
// Implementations may provide extra debugging information to users
// to trap and handle asynchronous errors.
static const async_handler default_async_handler = [](cl::sycl::exception_list list) {
  debug() << "Number of asynchronous errors during queue execution:" << list.size();
  for(auto& e : list) {
    debug("SYCL_ERROR::", e.what());
  }
};

namespace error {

struct thrower {
  static exception get(::cl_int error_code, context* thrower) {
    return cl_exception(error_code, thrower);
  }
  static exception get(code::value_t error_code, context* thrower) {
    return exception((*error::codes.find(error_code)).second, thrower);
  }
  static void report(exception& error) {
    debug("SYCL_ERROR::", error.what());
    throw error;
  }
  static void report_async(context* thrower, exception_list& list);
};

// Synchronous error reporting
static void report(::cl_int error_code, context* thrower = nullptr) {
  if(error_code != CL_SUCCESS) {
    auto e = thrower::get(error_code, thrower);
    thrower::report(e);
  }
}

// Synchronous error reporting
static void report(code::value_t error_code, context* thrower = nullptr) {
  auto e = thrower::get(error_code, thrower);
  thrower::report(e);
}

} // namespace error
} // namespace detail

} // namespace sycl
} // namespace cl
