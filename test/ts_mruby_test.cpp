#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define MOCKING // switch to testing mode
#include "libs/mocks.hpp"
#include "../src/ts_mruby.cpp"

using testing::_;
using namespace ts_mruby::utils;

// FIXME enable to link, for now ...
void ts_mrb_class_init(mrb_state *mrb){}

namespace {

TEST(ThreadLocalMRubyStates, getMrb) {
  ThreadLocalMRubyStates state;
  EXPECT_NE(nullptr, state.getMrb());
}

// Reset global and thread-shared variables
// FIXME It may exist a more better way than this, I believe ...
void reset_global_shared() {
  scriptsCache = nullptr;
}

TEST(TSPluginInit, ts_mruby_main) {
  Singleton<MrubyScriptsCacheMock> singleton;
  singleton.create();

  auto* cacheMock = singleton.get_instance();
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

  auto* cacheMock = singleton.get_instance();
  EXPECT_CALL(*cacheMock, store(_));

  // TODO replace constants with googletest fixtures
  char *argv[] = {"", "ts_mruby.so", "filepath"};
  EXPECT_EQ(TS_SUCCESS, TSRemapNewInstance(3, argv, nullptr, "", 0));
  EXPECT_EQ(TS_ERROR, TSRemapNewInstance(-1, argv, nullptr, "", 0));

  singleton.destroy();
  reset_global_shared();
}

} // anonymous namespace

