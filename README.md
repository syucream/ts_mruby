ts_mruby
========

[![Build Status](https://travis-ci.org/syucream/ts_mruby.svg?branch=ci)](https://travis-ci.org/syucream/ts_mruby)
[![Coverage Status](https://coveralls.io/repos/github/syucream/ts_mruby/badge.svg?branch=master)](https://coveralls.io/github/syucream/ts_mruby?branch=master)

Enchant Apache Traffic Server with mruby power.

what's ts_mruby?
================

**ts_mruby is an Apache Traffic Server plugin that provides more flexible and extensible server configuration.** You can write the configuration by mruby, and use mrbgems modules. ... And maybe the plugin support common syntax as part of the mod_mruby and ngx_mruby.

Requirements
============

* C++11

* Apache Traffic Server >= 4.2.x with atscppapi

  * To enable atscppapi, you should give the below option when you execute ./configure

```
./configure --enable-cppapi
```

* mruby

Examples
========

For example, you can write an ip filtering logic by using mruby like DSL:

```ruby
whitelist = [
  "127.0.0.1"
]

# deny if client IP is listed in whitelist
conn = ATS::Connection.new
unless whitelist.include?(conn.remote_ip)
  ATS::echo "Your access is not allowed ..."
  ATS::return ATS::HTTP_FORBIDDEN
end
```

Acknowledgement
===============
Some components for mruby in this repository and sample scripts refer to [mod_mruby](https://github.com/matsumoto-r/mod_mruby) and [ngx_mruby](https://github.com/matsumoto-r/ngx_mruby)'s one under the MIT License.
