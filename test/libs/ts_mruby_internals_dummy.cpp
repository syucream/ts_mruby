#include "../../src/ts_mruby_internal.hpp"

/*
 * Emply implementations not to link ts_mruby_internals
 *
 */
namespace ts_mruby {

MrubyScriptsCache* getInitializedGlobalScriptCache(const std::string&) {}

ThreadLocalMRubyStates *getThreadLocalMrubyStates() {}
RProc* ThreadLocalMRubyStates::getRProc(const std::string &key) {}

}; // namespace atscppapi
