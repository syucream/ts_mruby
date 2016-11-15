#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define MOCKING // switch to testing mode
#include "../src/ts_mruby.cpp"
#include "../src/utils.hpp"

#include "libs/mocks.hpp"

using testing::_;
using namespace ts_mruby::utils;

namespace {

// FIXME Resolve mocking and test target scope ...
/*  
TEST(ThreadLocalMRubyStates, getMrb) {
  ThreadLocalMRubyStates state;
  EXPECT_NE(nullptr, state.getMrb());
}
*/

/*
TEST(TSPluginInit, ts_mruby_main) {
  Singleton<MrubyScriptsCacheMock> singleton;
  singleton.create();

  auto *cacheMock = singleton.get_instance();
  EXPECT_CALL(*cacheMock, store(_));

  // TODO replace constants with googletest fixtures
  const char *argv[] = {"", "ts_mruby.so", "filepath"};
  TSPluginInit(2, argv);

  singleton.destroy();
  reset_global_shared();
}

TEST(TSRemapNewInstance, ts_mruby_main) {
  Singleton<MrubyScriptsCacheMock> singleton;
  singleton.create();

  auto *cacheMock = singleton.get_instance();
  EXPECT_CALL(*cacheMock, store(_));

  // TODO replace constants with googletest fixtures
  const char *argv[] = {"", "ts_mruby.so", "filepath"};
  EXPECT_EQ(TS_SUCCESS, TSRemapNewInstance(3, const_cast<char **>(argv),
                                           nullptr, const_cast<char *>(""), 0));
  EXPECT_EQ(TS_ERROR, TSRemapNewInstance(-1, const_cast<char **>(argv), nullptr,
                                         const_cast<char *>(""), 0));

  singleton.destroy();

  // TODO reset global states
}
*/

TEST(TSMrubyTestStub, ts_mruby_main) {
  EXPECT_EQ(true, true);
}

} // anonymous namespace
