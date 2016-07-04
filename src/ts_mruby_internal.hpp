/*
// ts_mruby.hpp - ts_mruby internal functions
//
*/

#ifndef TS_MRUBY_H
#define TS_MRUBY_H

#include <atscppapi/InterceptPlugin.h>
#include <atscppapi/Transaction.h>
#include <atscppapi/TransformationPlugin.h>
#include <map>
#include <tuple>
#include <vector>

#define MODULE_NAME "ts_mruby"
#define MODULE_VERSION "0.1"

#define MODULE_AUTHOR "Ryo Okubo"
#define MODULE_EMAIL ""

const int FILTER_RESERVED_BUFFER_SIZE = 1024;

using HeaderVec = std::vector<std::pair<std::string, std::string>>;

class RputsPlugin : public atscppapi::InterceptPlugin {
private:
  int _statusCode;
  HeaderVec _headers;
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

  void appendHeaders(const HeaderVec &h);

  void handleInputComplete();
};

class HeaderRewritePlugin : public atscppapi::TransactionPlugin {
public:
  HeaderRewritePlugin(atscppapi::Transaction &transaction)
      : atscppapi::TransactionPlugin(transaction) {
    atscppapi::TransactionPlugin::registerHook(HOOK_SEND_RESPONSE_HEADERS);
  }

  enum class Operator : int {
    ASSIGN,
    DELETE
  };

  void addRewriteRule(const std::pair<std::string, std::string> &entry, Operator op);
  void handleSendResponseHeaders(atscppapi::Transaction &transaction);

private:
  using Modifiers = std::vector<std::tuple<std::pair<std::string, std::string>, Operator>>;
  Modifiers modifiers_;
};

class FilterPlugin : public atscppapi::TransformationPlugin {
private:
  std::string _bodyBuffer;

public:
  FilterPlugin(atscppapi::Transaction &transaction)
      : atscppapi::TransformationPlugin(transaction, RESPONSE_TRANSFORMATION) {
    registerHook(HOOK_SEND_RESPONSE_HEADERS);
    _bodyBuffer.reserve(FILTER_RESERVED_BUFFER_SIZE);
  }

  void appendBody(const std::string &data);

  void consume(const std::string &data) { /* drop */
  }
  void handleInputComplete();
};

struct TSMrubyContext {
  atscppapi::Transaction *transaction;

  RputsPlugin *rputs;
  HeaderRewritePlugin *header_rewrite;
  FilterPlugin *filter;
};

#endif // TS_MRUBY_CORE_H
