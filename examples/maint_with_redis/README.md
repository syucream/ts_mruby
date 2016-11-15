# Maintainance mode example

Show maintainance message if the Redis value is set.

## requirements

* hiredis(and `mruby-redis`)

## How to use

* 1. build mruby with [mruby-redis](https://github.com/matsumoto-r/mruby-redis)
* 2. build ts_mruby with `-lhiredis`
* 3. prepare mruby script and ATS config. For example:

remap.config
```
map /maint http://127.0.0.1:80/ @plugin=ts_mruby.so @pparam=/Users/ryo/local/etc/trafficserver/maint.rb
```

* 4. Run Redis server

```
$ redis-server --port 6789
```

* 5. Run ATS
* 6. Control the maintainance mode value by setting to the Redis value.

```
$ redis-cli
127.0.0.1:6789> set ts_maint 1
$ curl -v 'http://localhost:8080/maint'
# you'll get maintainance message.

$ redis-cli
127.0.0.1:6789> del ts_maint
$ curl -v 'http://localhost:8080/maint'
# you'll get normal(not maintainance) message.
```
