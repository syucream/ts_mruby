/*
// ts_mruby.cpp - ts_mruby internal functions
//
*/

#include "ts_mruby_init.hpp"
#include "ts_mruby_internal.hpp"
#include "utils.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include <mruby/compile.h>
#include <mruby/string.h>
#include <ts/ts.h>

using namespace std;
using namespace atscppapi;

namespace {

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

vector<string> split(const string &str, char delimiter) {
  vector<string> splitted;

  string item;
  istringstream ss(str);
  while (getline(ss, item, delimiter)) {
    if (!item.empty()) {
      splitted.push_back(item);
    }
  }

  return splitted;
}

} // anonymous namespace

namespace ts_mruby {

/*
 * mruby scripts cache
 */
class MrubyScriptsCache {
public:
  MOCKABLE_ATTR
  void store(const std::string &filepath) {
    std::ifstream ifs(filepath);
    const std::string code((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());

    scripts_.insert(make_pair(filepath, code));
  }

  MOCKABLE_ATTR
  const std::string &load(const std::string &filepath) {
    return scripts_[filepath];
  }

#ifdef MOCKING
  using mock_type = class MrubyScriptsCacheMock;
#endif // MOCKING

private:
  std::map<std::string, std::string> scripts_;
};

/*
 * Global mruby scripts cache
 */
static MrubyScriptsCache *scriptsCache = NULL;
MrubyScriptsCache* getInitializedGlobalScriptCache(const string& filepath) {
  if (!scriptsCache) {
    scriptsCache = ts_mruby::utils::mockable_ptr<MrubyScriptsCache>();
  }
  scriptsCache->store(filepath);
  
  return scriptsCache;
}

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

// Note: Use pthread API's directly to have thread local parameters
ThreadLocalMRubyStates *getThreadLocalMrubyStates() {
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

} // ts_mruby namespace

bool judge_tls(const string &scheme) {
  if (scheme == "https") {
    return true;
  }
  return false;
}

pair<string, uint16_t> get_authority_pair(const string &authority,
                                          bool is_tls) {
  const vector<string> &splitted = split(authority, ':');

  uint16_t port = 80;
  if (splitted.size() == 2) {
    port = static_cast<uint16_t>(stoul(splitted[1]));
  } else {
    port = (is_tls) ? 443 : 80;
  }

  return make_pair(splitted[0], port);
}

void RputsPlugin::setStatusCode(int code) { status_code_ = code; }

void RputsPlugin::appendMessage(const string &msg) { message_ += msg; }

void RputsPlugin::appendHeader(const pair<string, string> &entry) {
  headers_.push_back(entry);
}

void RputsPlugin::appendHeaders(const vector<pair<string, string>> &h) {
  headers_.insert(headers_.end(), h.begin(), h.end());
}

void RputsPlugin::handleInputComplete() {
  string response(
      "HTTP/1.1 " + std::to_string(status_code_) + " " +
      TSHttpHdrReasonLookup(static_cast<TSHttpStatus>(status_code_)) + "\r\n");

  // make response header
  if (!message_.empty()) {
    response += "Content-Length: " + std::to_string(message_.size()) + "\r\n";
  }

  for_each(headers_.begin(), headers_.end(),
           [&response](pair<string, string> entry) {
             response += entry.first + ": " + entry.second + "\r\n";
           });

  // make response body
  response += "\r\n";
  produce(response);
  response = message_ + "\r\n";
  produce(response);
  setOutputComplete();
}

void HeaderRewritePlugin::addRewriteRule(const std::string &key,
                                         const std::string &value,
                                         Operator op) {
  modifiers_.push_back(make_tuple(key, value, op));
}

void HeaderRewritePlugin::handleSendResponseHeaders(Transaction &transaction) {
  Response &resp = transaction.getClientResponse();

  Headers &resp_headers = resp.getHeaders();
  for_each(modifiers_.begin(), modifiers_.end(),
           [&resp_headers](tuple<string, string, Operator> modifier) {
             switch (get<2>(modifier)) {
             case Operator::ASSIGN: {
               resp_headers[get<0>(modifier)] = get<1>(modifier);
               break;
             }
             case Operator::DELETE: {
               resp_headers.erase(get<0>(modifier));
               break;
             }
             default: { break; }
             }
           });

  transaction.resume();
}

void FilterPlugin::appendBody(const string &data) {
  transformedBuffer_.append(data);
}

void FilterPlugin::appendBlock(const mrb_value block) { block_ = block; }

void FilterPlugin::consume(const std::string &data) {
  origBuffer_.append(data);
}

void FilterPlugin::handleInputComplete() {
  if (!mrb_nil_p(block_)) {
    mrb_state *state = mrb_open();
    mrb_value rv =
        mrb_yield(state, block_, mrb_str_new(state, origBuffer_.c_str(),
                                             origBuffer_.length()));

    // Convert to_s if the value isn't Ruby string
    if (mrb_type(rv) != MRB_TT_STRING) {
      rv = mrb_funcall(state, rv, "to_s", 0, NULL);
    }

    transformedBuffer_ = string(RSTRING_PTR(rv), RSTRING_LEN(rv));
  }

  produce(transformedBuffer_);
  setOutputComplete();
}
