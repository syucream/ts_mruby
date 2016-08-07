/*
// ts_mruby_records.hpp - ts_mruby mruby module header
//
*/

#ifndef TS_MRUBY_RECORDS_H
#define TS_MRUBY_RECORDS_H

#include <mruby.h>
#include "ts_mruby_config_keys.hpp"

template<int K> // Expanding mrb_define_const() while TSOverridableConfigKey
inline void define_configkey_const(mrb_state *mrb, struct RClass *rclass) {
  define_configkey_const<K - 1>(mrb, rclass);
  mrb_define_const(mrb, rclass, OVERRIDABLE_CONFIG_KEY[K], mrb_fixnum_value(K));
};
template<> // the end of define_configkey_const
inline void define_configkey_const<0>(mrb_state *mrb, struct RClass *rclass) {
};

void ts_mrb_records_class_init(mrb_state *mrb, struct RClass *rclass);

#endif // TS_MRUBY_RECORDS_H
