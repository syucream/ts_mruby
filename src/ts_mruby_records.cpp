/*
// ts_mruby_records.cpp - ts_mruby mruby module
//
*/

#include "ts_mruby_records.hpp"
#include "ts_mruby_internal.hpp"

#include <atscppapi/Transaction.h>
#include <atscppapi/utils.h>

#include <mruby.h>
#include <mruby/class.h>
#include <mruby/compile.h>
#include <mruby/data.h>
#include <mruby/hash.h>
#include <mruby/proc.h>
#include <mruby/string.h>
#include <mruby/variable.h>

using namespace atscppapi;
using std::string;

namespace {

// Simplified _conf_to_memberp() in proxy/InkAPI.cc

TSRecordDataType
get_record_data_type(TSOverridableConfigKey key) {
  TSRecordDataType type = TS_RECORDDATATYPE_INT;

  switch(key) {
  case TS_CONFIG_HTTP_CACHE_HEURISTIC_LM_FACTOR:
  case TS_CONFIG_HTTP_CACHE_FUZZ_PROBABILITY:
  case TS_CONFIG_HTTP_BACKGROUND_FILL_COMPLETED_THRESHOLD:
    type = TS_RECORDDATATYPE_FLOAT;
    break;
  case TS_CONFIG_HTTP_RESPONSE_SERVER_STR:
  case TS_CONFIG_HTTP_GLOBAL_USER_AGENT_HEADER:
  case TS_CONFIG_BODY_FACTORY_TEMPLATE_BASE:
    type = TS_RECORDDATATYPE_STRING;
    break;
  case TS_CONFIG_NULL:
  case TS_CONFIG_LAST_ENTRY:
    type = TS_RECORDDATATYPE_NULL;
    break;
  }

  return type;
}

} // anonymous namespace


static mrb_value ts_mrb_get_records(mrb_state *mrb, mrb_value self) {
  mrb_int index;
  mrb_get_args(mrb, "i", &index);

  const auto config_key = static_cast<TSOverridableConfigKey>(index);
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  auto *transaction = context->transaction;

  bool successed = false;
  mrb_value rv = mrb_nil_value();
  switch(get_record_data_type(config_key)) {
  case TS_RECORDDATATYPE_INT:
    {
      int value_i = 0;
      successed = transaction->configIntGet(config_key, &value_i);
      rv = mrb_fixnum_value(value_i);
    }
    break;
  case TS_RECORDDATATYPE_FLOAT:
    {
      float value_f = 0.0f;
      successed = transaction->configFloatGet(config_key, &value_f);
      rv = mrb_float_value(mrb, value_f);
    }
    break;
  case TS_RECORDDATATYPE_STRING:
    {
      string value_s("");
      successed = transaction->configStringGet(config_key, value_s);
      rv = mrb_str_new_cstr(mrb, value_s.c_str());
    }
    break;
  default:
    successed = false;
    break;
  }

  return successed ? rv : mrb_nil_value();
}

static mrb_value ts_mrb_set_records(mrb_state *mrb, mrb_value self) {
  mrb_int index;
  mrb_value value;
  mrb_get_args(mrb, "io", &index, &value);

  const auto config_key = static_cast<TSOverridableConfigKey>(index);
  auto *context = reinterpret_cast<TSMrubyContext *>(mrb->ud);
  auto *transaction = context->transaction;

  bool successed = false;
  switch(get_record_data_type(config_key)) {
  case TS_RECORDDATATYPE_INT:
    successed = transaction->configIntSet(config_key, mrb_fixnum(value));
    break;
  case TS_RECORDDATATYPE_FLOAT:
    successed = transaction->configFloatSet(config_key, mrb_float(value));
    break;
  case TS_RECORDDATATYPE_STRING:
    {
      const string value_s(RSTRING_PTR(value), RSTRING_LEN(value));
      successed = transaction->configStringSet(config_key, value_s);
    }
    break;
  default:
    successed = false;
    break;
  }

  return successed ? self : mrb_nil_value();
}

void ts_mrb_records_class_init(mrb_state *mrb, struct RClass *rclass) {
  struct RClass *class_records;

  // Records::
  class_records =
      mrb_define_class_under(mrb, rclass, "Records", mrb->object_class);

  // Generate definitions of constants
  define_configkey_const<TS_CONFIG_LAST_ENTRY>(mrb, class_records);

  // getter/setter
  mrb_define_method(mrb, class_records, "get", ts_mrb_get_records,
                    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, class_records, "set", ts_mrb_set_records,
                    MRB_ARGS_REQ(2));
}
