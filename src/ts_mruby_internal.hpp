/*
// ts_mruby.hpp - ts_mruby internal functions
//
*/

#ifndef TS_MRUBY_H
#define TS_MRUBY_H

#include <vector>
#include <map>
#include <atscppapi/Transaction.h>
#include <atscppapi/InterceptPlugin.h>

#define MODULE_NAME "ts_mruby"
#define MODULE_VERSION "0.1"

#define MODULE_AUTHOR "Ryo Okubo"
#define MODULE_EMAIL ""

typedef std::vector<std::pair<std::string, std::string>> HeaderVec;

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

  void appendMessage(const std::string msg);

  void appendHeader(const std::pair<std::string, std::string> entry);

  void appendHeaders(const HeaderVec &h);

  void handleInputComplete();
};

class HeaderRewritePlugin : public atscppapi::TransactionPlugin {
private:
  HeaderVec _headers;

public:
  HeaderRewritePlugin(atscppapi::Transaction &transaction)
    : atscppapi::TransactionPlugin(transaction) {
      atscppapi::TransactionPlugin::registerHook(HOOK_SEND_RESPONSE_HEADERS);
  }

  void addRewriteRule(const std::pair<std::string, std::string> entry);
  void handleSendResponseHeaders(atscppapi::Transaction &transaction);
};

struct TSMrubyContext {
  atscppapi::Transaction *transaction;
  RputsPlugin *rputs;
  HeaderRewritePlugin *header_rewrite;
};

#endif // TS_MRUBY_CORE_H
