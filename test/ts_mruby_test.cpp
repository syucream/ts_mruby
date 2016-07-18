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

TEST(TSRemapNewInstance, ts_mruby_main) {
  auto cacheMock = shared_ptr<MrubyScriptsCacheMock>(
      static_cast<MrubyScriptsCacheMock*>(mockable_ptr<MrubyScriptsCache>()));
  EXPECT_CALL(*cacheMock.get(), store(_));

  // TODO replace constants with googletest fixtures
  char *argv[] = {"", "ts_mruby.so", "filepath"};
  EXPECT_EQ(TS_SUCCESS, TSRemapNewInstance(3, argv, nullptr, "", 0));
  EXPECT_EQ(TS_ERROR, TSRemapNewInstance(-1, argv, nullptr, "", 0));
}

} // anonymous namespace

