#include <iostream>
#include <fstream>
#include <map>
#include <string>

#include <atscppapi/GlobalPlugin.h>
#include <atscppapi/PluginInit.h>

#include <mruby.h>
#include <mruby/proc.h>
#include <mruby/compile.h>

#include "ts_mruby.hpp"
#include "ts_mruby_init.hpp"
#include "ts_mruby_request.hpp"

using namespace std;
using namespace atscppapi;

class MrubyScriptsCache {
public:
  void store(const string& filepath) {
    ifstream ifs(filepath);
    string code((istreambuf_iterator<char>(ifs)),
                 istreambuf_iterator<char>());

    scripts_.insert(make_pair(filepath, code));
  }

  const string& load(const string& filepath) {
    return scripts_[filepath];
  }

private:
  map<string, string> scripts_;
};

// Global mruby scripts cache
static MrubyScriptsCache* scriptsCache = NULL;

class MRubyPlugin : public GlobalPlugin {
public:
  MRubyPlugin(const string& fpath) : filepath(fpath) {
    const string& code = scriptsCache->load(fpath);

    _mrb = mrb_open();
    ts_mrb_class_init(_mrb);
    mrbc_context *context = mrbc_context_new(_mrb);
    struct mrb_parser_state* st = mrb_parse_string(_mrb, code.c_str(), context);
    _proc = mrb_generate_code(_mrb, st);
    mrb_pool_close(st->pool);

    registerHook(HOOK_READ_REQUEST_HEADERS_PRE_REMAP);
  }

  ~MRubyPlugin() {
    mrb_close(_mrb);
  }

  virtual void handleReadRequestHeadersPreRemap(Transaction &transaction) {
    // set variables related to this transaction.
    ts_mrb_set_transaction(&transaction);

    // execute mruby script when ATS pre-remap hook occurs.
    mrb_run(_mrb, _proc, mrb_nil_value());

    transaction.resume();
  }

private:
  mrb_state *_mrb;
  RProc *_proc;
  string filepath;

};

void TSPluginInit(int argc, const char *argv[]) {
  if ( argc == 2 ) {
    RegisterGlobalPlugin(MODULE_NAME, MODULE_AUTHOR, MODULE_EMAIL);

    scriptsCache = new MrubyScriptsCache;
    scriptsCache->store(argv[1]);

    new MRubyPlugin(argv[1]);
  }
}
