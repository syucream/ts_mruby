/*
// ts_mruby.hpp - ts_mruby internal functions
//
*/

#ifndef TS_MRUBY_INTERNAL_H
#define TS_MRUBY_INTERNAL_H

#include <pthread.h>

#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <atscppapi/InterceptPlugin.h>
#include <atscppapi/Transaction.h>
#include <atscppapi/TransformationPlugin.h>
#include <mruby.h>
#include <mruby/value.h>

// Enable to unit test
#ifdef MOCKING
#define MOCKABLE_ATTR virtual
#else
#define MOCKABLE_ATTR
#endif // MOCKING


#define TS_MRUBY_PLUGIN_NAME "ts_mruby"
const static char *TS_MRUBY_PLUGIN_VERSION = "0.1";
const static char *TS_MRUBY_PLUGIN_AUTHOR = "Ryo Okubo";
const static char *TS_MRUBY_PLUGIN_EMAIL = "";
const int FILTER_RESERVED_BUFFER_SIZE = 1024;
const int MAX_LENDABLE_VALUES = 128;

bool judge_tls(const std::string &scheme);

std::pair<std::string, uint16_t>
get_authority_pair(const std::string &authority, bool is_tls = false);

/**
 * Represent lent mrb_value from any thread-local mrb_state
 *
 * NOTE I believe it can replace with shared_ptr with custom deleter.
 *
 */
class LentMrbValue {
private:
  using DisposalCallback = std::function<void(mrb_value)>;

  mrb_value value_;
  DisposalCallback callback_;

public:
  LentMrbValue(mrb_value value, DisposalCallback cb)
    : value_(value), callback_(cb) {}

  ~LentMrbValue() { callback_(value_); }

  mrb_value getValue() { return value_; }
};

/**
 * Object Manager for lendable mrb_value's to any other mrb_state's
 *
 * NOTE: It must ensure release function thread-safe.
 *
 */
class LendableMrbValueManager {
private:
  mrb_state* mrb_;
  std::vector<mrb_value> returnedValues_;

  // Should is it replaced with TS structure?
  pthread_mutex_t mutex_;

public:
  LendableMrbValueManager() 
    : mrb_(nullptr) {
    returnedValues_.reserve(MAX_LENDABLE_VALUES);
    mutex_ = PTHREAD_MUTEX_INITIALIZER;
  }

  ~LendableMrbValueManager() { pthread_mutex_destroy(&mutex_); }

  void set_mrb_state(mrb_state* mrb) { mrb_ = mrb; }

  // Thread-safe cleanup function
  void cleanup_if_needed();

  // Generate thread-safe callback to return mrb_value
  std::shared_ptr<LentMrbValue> lend_mrb_value(mrb_value value);
  void disposal_callback(mrb_value value);
};

namespace ts_mruby {

/*
 * Thread local mrb_state and RProc*'s
 */
class ThreadLocalMRubyStates {
public:
  ThreadLocalMRubyStates();
  ~ThreadLocalMRubyStates();

  mrb_state *getMrb() { return state_; }
  RProc *getRProc(const std::string &key);
  LendableMrbValueManager& getManager() { return manager_; }

private:
  mrb_state *state_;
  std::map<std::string, RProc *> procCache_;
  LendableMrbValueManager manager_;
};

/*
 * Getter for thread-shared mruby script cache
 */
class MrubyScriptsCache;
MrubyScriptsCache* getInitializedGlobalScriptCache(const std::string& filepath);

/* 
 * Getter for thread-local mrb_state*
 */
ThreadLocalMRubyStates *getThreadLocalMrubyStates();

} // ts_mruby namespace


/*
 * rputs/echo intercept plugin
 *
 * It enables users specify response header/body.
 * If the plugin is used, ATS doesn't fetch to origin server.
 * 
 */
class RputsPlugin : public atscppapi::InterceptPlugin {
private:
  int status_code_;
  std::vector<std::pair<std::string, std::string>> headers_;
  std::string message_;

public:
  RputsPlugin(atscppapi::Transaction &transaction, int code = 200)
      : atscppapi::InterceptPlugin(
            transaction, atscppapi::InterceptPlugin::TRANSACTION_INTERCEPT),
        status_code_(code), message_("") {}

  void consume(const std::string &data,
               atscppapi::InterceptPlugin::RequestDataType type){};

  void setStatusCode(int code);

  void appendMessage(const std::string &msg);

  void appendHeader(const std::pair<std::string, std::string> &entry);

  void appendHeaders(const std::vector<std::pair<std::string, std::string>> &h);

  void handleInputComplete();
};

/*
 * HeaderRewrite plugin
 *
 * It enables users specify response headers with low cost.
 * 
 */
class HeaderRewritePlugin : public atscppapi::TransactionPlugin {
public:
  HeaderRewritePlugin(atscppapi::Transaction &transaction)
      : atscppapi::TransactionPlugin(transaction) {
    atscppapi::TransactionPlugin::registerHook(HOOK_SEND_RESPONSE_HEADERS);
  }

  enum class Operator : int { ASSIGN, DELETE };

  void addRewriteRule(const std::string &key, const std::string &value,
                      Operator op);
  void handleSendResponseHeaders(atscppapi::Transaction &transaction);

private:
  using Modifiers = std::vector<std::tuple<std::string, std::string, Operator>>;
  Modifiers modifiers_;
};

/*
 * Output Filter plugin
 *
 * It enables users modify response body with their mruby block.
 * NOTE It does buffering response body.
 * 
 */
class FilterPlugin : public atscppapi::TransformationPlugin {
private:
  std::string origBuffer_;
  std::string transformedBuffer_;
  mrb_value block_;

public:
  FilterPlugin(atscppapi::Transaction &transaction)
      : atscppapi::TransformationPlugin(transaction, RESPONSE_TRANSFORMATION) {
    registerHook(HOOK_SEND_RESPONSE_HEADERS);

    origBuffer_.reserve(FILTER_RESERVED_BUFFER_SIZE);
    transformedBuffer_.reserve(FILTER_RESERVED_BUFFER_SIZE);
    block_ = mrb_nil_value();
  }

  void appendBody(const std::string &data);
  void appendBlock(const mrb_value block);

  void consume(const std::string &data);
  void handleInputComplete();
};

struct TSMrubyResult {
  bool isRemapped = false;
};

/*
 * Current Transaction state.
 * It nearly equal atscppapi::Plugin::HookType
 *
 */
enum class TransactionStateTag : int {
  READ_REQUEST_HEADERS,
  SEND_REQUEST_HEADERS,
  READ_RESPONSE_HEADERS,
  SEND_RESPONSE_HEADERS,
};

/*
 * ts_mruby context
 *
 * It has a lifecycle similar to TS txn.
 * And some pointer member will be released by atscppapi
 *
 */
class TSMrubyContext {
private:
  TransactionStateTag state_tag_;
  TSMrubyResult result_;

  // NOTE: these resource's owner is atscppai
  atscppapi::Transaction *transaction_;
  RputsPlugin *rputs_;
  HeaderRewritePlugin *header_rewrite_;
  FilterPlugin *filter_;

  template <typename T> void createAndAddPlugin_(T **ptr) {
    if (transaction_ && !*ptr) {
      *ptr = new T(*transaction_);
      transaction_->addPlugin(*ptr);
    }
  }

public:
  TSMrubyContext() 
    : state_tag_(TransactionStateTag::READ_REQUEST_HEADERS),
      transaction_(nullptr), rputs_(nullptr), header_rewrite_(nullptr),
      filter_(nullptr) {}

  atscppapi::Transaction *getTransaction() { return transaction_; }
  void setTransaction(atscppapi::Transaction *transaction) {
    transaction_ = transaction;
  }

  const TSMrubyResult getResult() const { return result_; }
  void setResult(TSMrubyResult r) { result_ = r; }

  RputsPlugin *getRputsPlugin() { return rputs_; }
  HeaderRewritePlugin *getHeaderRewritePlugin() { return header_rewrite_; }
  FilterPlugin *getFilterPlugin() { return filter_; }

  void registerRputsPlugin() { createAndAddPlugin_<RputsPlugin>(&rputs_); }
  void registerHeaderRewritePlugin() {
    createAndAddPlugin_<HeaderRewritePlugin>(&header_rewrite_);
  }
  void registerFilterPlugin() { createAndAddPlugin_<FilterPlugin>(&filter_); }

  TransactionStateTag getStateTag() const { return state_tag_; }
  void setStateTag(TransactionStateTag tag) { state_tag_ = tag; }
};

#endif // TS_MRUBY_INTERNAL_H
