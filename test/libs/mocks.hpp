#ifdef MOCKING

#include "../../src/ts_mruby_internal.hpp"
#include "gmock/gmock.h"

class MrubyScriptsCacheMock : public ts_mruby::MrubyScriptsCache {
public:
  MOCK_METHOD1(store, void(const std::string &));
  MOCK_METHOD1(load, const std::string &(const std::string &));
};

#endif // MOCKING
