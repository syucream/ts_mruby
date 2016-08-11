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
const static char* TS_MRUBY_PLUGIN_VERSION = "0.1";
const static char* TS_MRUBY_PLUGIN_AUTHOR = "Ryo Okubo";
const static char* TS_MRUBY_PLUGIN_EMAIL = "";
const int FILTER_RESERVED_BUFFER_SIZE = 1024;

bool
judge_tls(const std::string& scheme);

std::pair<std::string, uint16_t>
get_authority_pair(const std::string& authority, bool is_tls = false);

class RputsPlugin : public atscppapi::InterceptPlugin {
private:
  int _statusCode;
  std::vector<std::pair<std::string, std::string>> _headers;
  std::string _message;

public:
  RputsPlugin(atscppapi::Transaction &transaction, int code = 200)
      : atscppapi::InterceptPlugin(
            transaction, atscppapi::InterceptPlugin::TRANSACTION_INTERCEPT),
        _statusCode(code), _message("") {}

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
  std::string _origBuffer;
  std::string _transformedBuffer;
  mrb_value _block;

public:
  FilterPlugin(atscppapi::Transaction &transaction)
      : atscppapi::TransformationPlugin(transaction, RESPONSE_TRANSFORMATION) {
    registerHook(HOOK_SEND_RESPONSE_HEADERS);

    _origBuffer.reserve(FILTER_RESERVED_BUFFER_SIZE);
    _transformedBuffer.reserve(FILTER_RESERVED_BUFFER_SIZE);
    _block = mrb_nil_value();
  }

  void appendBody(const std::string &data);
  void appendBlock(const mrb_value block);

  void consume(const std::string &data);
  void handleInputComplete();
};

struct TSMrubyResult {
  bool isRemapped = false;
};

struct TSMrubyContext {
  atscppapi::Transaction *transaction;

  RputsPlugin *rputs;
  HeaderRewritePlugin *header_rewrite;
  FilterPlugin *filter;

  TSMrubyResult result;
};

#endif // TS_MRUBY_INTERNAL_H
