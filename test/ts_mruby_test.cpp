#include "gmock/gmock.h"
#include "gtest/gtest.h"

#define MOCKING // switch to testing mode
#include "../src/ts_mruby.cpp"
#include "libs/mocks.hpp"

using testing::_;
using namespace ts_mruby::utils;

// FIXME enable to link, for now ...
void ts_mrb_class_init(mrb_state *mrb) {}

namespace {

TEST(ThreadLocalMRubyStates, getMrb) {
  ThreadLocalMRubyStates state;
  EXPECT_NE(nullptr, state.getMrb());
}

// Reset global and thread-shared variables
// FIXME It may exist a more better way than this, I believe ...
void reset_global_shared() { scriptsCache = nullptr; }

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
  reset_global_shared();
}

} // anonymous namespace
