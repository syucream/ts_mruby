ts_mruby
========

Enchant Apache Traffic Server with mruby power.

Requirements
============

* C++11

* Apache Traffic Server >= 4.2.x with atscppapi

  * To enable atscppapi, you should give the below option when you execute ./configure

```
./configure --enable-cppapi
```

* mruby

Acknowledgement
===============
Some components for mruby in this repository and sample scripts refer to [mod_mruby](https://github.com/matsumoto-r/mod_mruby) and [ngx_mruby](https://github.com/matsumoto-r/ngx_mruby)'s one under the MIT License.
