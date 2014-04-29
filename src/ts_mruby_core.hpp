/*
// ts_mruby_core.h - ts_mruby mruby module header
//
*/

#ifndef TS_MRUBY_CORE_H
#define TS_MRUBY_CORE_H

#include <iostream>
#include <sstream>

#include <atscppapi/Transaction.h>
#include <atscppapi/InterceptPlugin.h>

#define MODULE_NAME    "ts_mruby"
#define MODULE_VERSION "0.0.1"

namespace {

using namespace atscppapi;
using std::string;

class RputsPlugin : public InterceptPlugin {
private:
  string _message;

  template <typename T>
  string toString(T num){
    std::stringstream ss;
    ss << num;
    return ss.str();
  }

public:
  RputsPlugin(Transaction &transaction)
    : InterceptPlugin(transaction, InterceptPlugin::TRANSACTION_INTERCEPT),
      _message("") { }

  ~RputsPlugin();

  void consume(const string &data, InterceptPlugin::RequestDataType type) {}

  void appendMessage(const string msg) { _message += msg;  }

  void handleInputComplete();

};

}

#endif // TS_MRUBY_CORE_H
