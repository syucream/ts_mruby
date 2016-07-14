#include <fstream>
#include <iostream>
#include <map>
#include <pthread.h>
#include <string>

#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/PluginInit.h>
#include <atscppapi/RemapPlugin.h>

#include <mruby.h>
#include <mruby/compile.h>
#include <mruby/proc.h>

#include "ts_mruby.hpp"
#include "ts_mruby_internal.hpp"
#include "ts_mruby_init.hpp"
#include "ts_mruby_internal.hpp"
#include "ts_mruby_request.hpp"
#include "utils.hpp"

using namespace std;
using namespace atscppapi;

namespace {

// Global mruby scripts cache
static MrubyScriptsCache *scriptsCache = NULL;

// key specifying thread local data
pthread_key_t threadKey = 0;

// Initialize thread key when this plugin is loaded
__attribute__((constructor)) void create_thread_keys() {
  if (threadKey == 0) {
    if (pthread_key_create(&threadKey, NULL) != 0) {
      // XXX fatal error
    }
  }
}

/*
 * Thread local mrb_state and RProc*'s
 */
class ThreadLocalMRubyStates {
public:
  ThreadLocalMRubyStates() {
    state_ = mrb_open();
    ts_mrb_class_init(state_);
  }

  ~ThreadLocalMRubyStates() {
    mrb_close(state_);
    state_ = NULL;
  }

  mrb_state *getMrb() { return state_; }

  RProc *getRProc(const string &key) {
    RProc *proc = procCache_[key];
    if (!proc) {
      const string &code = scriptsCache->load(key);

      // compile
      mrbc_context *context = mrbc_context_new(state_);
      auto *st = mrb_parse_string(state_, code.c_str(), context);
      proc = mrb_generate_code(state_, st);
      mrb_pool_close(st->pool);

      // store to cache
      procCache_.insert(make_pair(key, proc));
    }

    return proc;
  }

private:
  mrb_state *state_;
  map<string, RProc *> procCache_;
};

// Note: Use pthread API's directly to have thread local parameters
ThreadLocalMRubyStates *getMrubyStates() {
  auto *state =
      static_cast<ThreadLocalMRubyStates *>(pthread_getspecific(threadKey));

  if (!state) {
    state = new ThreadLocalMRubyStates();
    if (pthread_setspecific(threadKey, state)) {
      // XXX fatal error
    }
  }

  return state;
}

} // anonymous namespace

class MRubyPluginBase {
protected:
  MRubyPluginBase(const string &fpath) : filepath_(fpath) {}

  void executeMrubyScript(Transaction &transaction) {
    // get or initialize thread local mruby VM
    ThreadLocalMRubyStates *states = getMrubyStates();
    mrb_state *mrb = states->getMrb();

    // get or compile mruby script
    RProc *proc = states->getRProc(filepath_);

    // set execution context
    TSMrubyContext *context = new TSMrubyContext();
    context->transaction = &transaction;
    context->rputs = NULL;
    mrb->ud = reinterpret_cast<void *>(context);

    // execute mruby script when ATS pre-remap hook occurs.
    mrb_run(mrb, proc, mrb_nil_value());
  }

private:
  string filepath_;
};

class MRubyPlugin : public GlobalPlugin, MRubyPluginBase {
public:
  MRubyPlugin(const string &fpath) : MRubyPluginBase(fpath) {
    registerHook(HOOK_READ_REQUEST_HEADERS_PRE_REMAP);
  }

  virtual void handleReadRequestHeadersPreRemap(Transaction &transaction) {
    executeMrubyScript(transaction);

    transaction.resume();
  }
};

class MRubyRemapPlugin : public RemapPlugin, MRubyPluginBase {
public:
  MRubyRemapPlugin(void **instance_handle, const string &fpath)
      : RemapPlugin(instance_handle), MRubyPluginBase(fpath) {}

  Result doRemap(const Url &map_from_url, const Url &map_to_url,
                 Transaction &transaction, bool &redirect) {
    executeMrubyScript(transaction);

    return RESULT_NO_REMAP;
  }
};

// As global plugin
void TSPluginInit(int argc, const char *argv[]) {
  if (argc == 2) {
    RegisterGlobalPlugin(MODULE_NAME, MODULE_AUTHOR, MODULE_EMAIL);

    if (!scriptsCache) {
      scriptsCache = ts_mruby::utils::mockable_ptr<MrubyScriptsCache>();
    }
    scriptsCache->store(argv[1]);

    new MRubyPlugin(argv[1]);
  }
}

// As remap plugin
TSReturnCode TSRemapNewInstance(int argc, char *argv[], void **ih,
                                char * /* ATS_UNUSED */, int /* ATS_UNUSED */) {
  if (argc == 3) {
    if (!scriptsCache) {
      scriptsCache = ts_mruby::utils::mockable_ptr<MrubyScriptsCache>();
    }
    scriptsCache->store(argv[2]);

    new MRubyRemapPlugin(ih, argv[2]);

    return TS_SUCCESS;
  } else {
    return TS_ERROR;
  }
}
