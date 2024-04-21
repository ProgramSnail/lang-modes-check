#pragma once

#include <source_location>
#include <string>

namespace utils {

using namespace std;

// C++ 23
[[noreturn]] inline void unreachable() {
  // Uses compiler specific extensions if possible.
  // Even if no extension is used, undefined behavior is still raised by
  // an empty function body and the noreturn attribute.
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
  __assume(false);
#else // GCC, Clang
  __builtin_unreachable();
#endif
}

// -----------------

// visitor helper
template <typename... Ts> struct overloaded : Ts... {
  using Ts::operator()...;
};

struct Error {
  string message;
  source_location location;
};

inline void throw_error(string message,
                        source_location location = source_location::current()) {
  throw Error{std::move(message), location};
}

} // namespace utils
