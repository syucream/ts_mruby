namespace ts_mruby {

namespace utils {

template<typename T, typename... Args>
T* mockable_ptr(Args... args) {
#ifndef MOCKING
  return new T(args...);
#else
  // FIXME it needs mutual exclusion and calling delete ...
  using T_MOCK = typename T::mock_type;
  static T_MOCK* ptr = nullptr;
  if (!ptr) {
    ptr = new T_MOCK(args...);
  }
  return ptr;
#endif
}

} // utils namespace

} // ts_mruby namespace
