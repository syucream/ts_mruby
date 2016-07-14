#include <type_traits>

namespace ts_mruby {

namespace utils {


namespace {

extern void *enabler;

/*
 * meta functions check 'type T' has mock_type or not
 */
struct has_mock_impl {
  template<typename T>
  static std::true_type check(typename T::mock_type*);

  template<typename T>
  static std::false_type check(...);
};
template<typename T>
class has_mock : public decltype(has_mock_impl::check<T>(nullptr)) {};

} // anonymous namespace


// Allocate non-mocked object
template<typename T, typename... Args, typename std::enable_if<!has_mock<T>::value>::type*& = enabler>
T* mockable_ptr(Args... args) {
  return new T(args...);
}

// Allocate or get mocked object
// FIXME it needs mutual exclusion and calling delete ...
template<typename T, typename... Args, typename std::enable_if<has_mock<T>::value>::type*& = enabler>
T* mockable_ptr(Args... args) {
  using T_MOCK = typename T::mock_type;
  static T_MOCK* ptr = nullptr;
  if (!ptr) {
    ptr = new T_MOCK(args...);
  }
  return ptr;
}

} // utils namespace

} // ts_mruby namespace