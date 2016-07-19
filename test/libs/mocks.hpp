#ifdef MOCKING

#include "gmock/gmock.h"
#include "../../src/ts_mruby.hpp"

class MrubyScriptsCacheMock : public MrubyScriptsCache {
public:
  MOCK_METHOD1(store, void(const std::string&));
  MOCK_METHOD1(load, const std::string&(const std::string&));
};

#endif // MOCKING

