#ifdef MOCKING

#include "../../src/ts_mruby.hpp"
#include "gmock/gmock.h"

class MrubyScriptsCacheMock : public MrubyScriptsCache {
public:
  MOCK_METHOD1(store, void(const std::string &));
  MOCK_METHOD1(load, const std::string &(const std::string &));
};

#endif // MOCKING
