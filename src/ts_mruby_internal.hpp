/*
// ts_mruby.hpp - ts_mruby internal functions
//
*/

#ifndef TS_MRUBY_INTERNAL_H
#define TS_MRUBY_INTERNAL_H

#include <atscppapi/InterceptPlugin.h>
#include <atscppapi/Transaction.h>
#include <atscppapi/TransformationPlugin.h>
#include <map>
#include <tuple>
#include <utility>
#include <vector>

#include <mruby.h>
#include <mruby/value.h>

#define TS_MRUBY_PLUGIN_NAME "ts_mruby"
const static char *TS_MRUBY_PLUGIN_VERSION = "0.1";
const static char *TS_MRUBY_PLUGIN_AUTHOR = "Ryo Okubo";
const static char *TS_MRUBY_PLUGIN_EMAIL = "";
const int FILTER_RESERVED_BUFFER_SIZE = 1024;

bool judge_tls(const std::string &scheme);

std::pair<std::string, uint16_t>
get_authority_pair(const std::string &authority, bool is_tls = false);

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
 * ts_mruby context
 *
 * It has a lifecycle similar to TS txn.
 * And some pointer member will be released by atscppapi
 *
 */
class TSMrubyContext {
private:
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
  atscppapi::Transaction *getTransaction() { return transaction_; }
  void setTransaction(atscppapi::Transaction *transaction) {
    transaction_ = transaction;
  }

  const TSMrubyResult getResult() { return result_; }
  void setResult(TSMrubyResult r) { result_ = r; }

  RputsPlugin *getRputsPlugin() { return rputs_; }
  HeaderRewritePlugin *getHeaderRewritePlugin() { return header_rewrite_; }
  FilterPlugin *getFilterPlugin() { return filter_; }

  void registerRputsPlugin() { createAndAddPlugin_<RputsPlugin>(&rputs_); }
  void registerHeaderRewritePlugin() {
    createAndAddPlugin_<HeaderRewritePlugin>(&header_rewrite_);
  }
  void registerFilterPlugin() { createAndAddPlugin_<FilterPlugin>(&filter_); }
};

#endif // TS_MRUBY_INTERNAL_H
