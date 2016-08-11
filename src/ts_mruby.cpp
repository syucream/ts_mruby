#include <fstream>
#include <iostream>
#include <map>
#include <pthread.h>
#include <string>

#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/PluginInit.h>
#include <atscppapi/RemapPlugin.h>

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

  TSMrubyResult executeMrubyScript(Transaction &transaction) {
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

    return context->result;
  }

private:
  string filepath_;
};

ThreadLocalMRubyStates::ThreadLocalMRubyStates() {
  state_ = mrb_open();
  ts_mrb_class_init(state_);
}

ThreadLocalMRubyStates::~ThreadLocalMRubyStates() {
  mrb_close(state_);
  state_ = NULL;
}

RProc *ThreadLocalMRubyStates::getRProc(const std::string &key) {
  RProc *proc = procCache_[key];
  if (!proc) {
    const std::string &code = scriptsCache->load(key);

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

class MRubyPlugin : public GlobalPlugin, MRubyPluginBase {
public:
  MRubyPlugin(const string &fpath) : MRubyPluginBase(fpath) {
    registerHook(HOOK_READ_REQUEST_HEADERS);
  }

  virtual void handleReadRequestHeaders(Transaction &transaction) {
    TSMrubyResult result = executeMrubyScript(transaction);

    if (result.isRemapped) {
      transaction.setSkipRemapping(1);
    }
    transaction.resume();
  }
};

class MRubyRemapPlugin : public RemapPlugin, MRubyPluginBase {
public:
  MRubyRemapPlugin(void **instance_handle, const string &fpath)
      : RemapPlugin(instance_handle), MRubyPluginBase(fpath) {}

  Result doRemap(const Url &map_from_url, const Url &map_to_url,
                 Transaction &transaction, bool &redirect) {
    TSMrubyResult result = executeMrubyScript(transaction);

    return (result.isRemapped) ? RESULT_DID_REMAP : RESULT_NO_REMAP;
  }
};

// As global plugin
void TSPluginInit(int argc, const char *argv[]) {
  if (argc == 2) {
    RegisterGlobalPlugin(TS_MRUBY_PLUGIN_NAME, TS_MRUBY_PLUGIN_AUTHOR, TS_MRUBY_PLUGIN_EMAIL);

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
