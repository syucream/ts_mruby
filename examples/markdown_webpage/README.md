# Markdown Web page example

Auto-convert markdown to html when HTTP requests come.

## requirements

* `mruby-discount`

## How to use

* 1. build mruby with [mruby-discount](https://github.com/matsumoto-r/mruby-discount)
* 2. prepare mruby script and ATS config. For example:

remap.config
```
map /mdpage http://127.0.0.1:80/ @plugin=ts_mruby.so @pparam=/Users/ryo/local/etc/trafficserver/mdpage.rb
```

* 3. Run ATS
* 4. Responds body converted to html

```
$ curl -v 'http://localhost:8080/mdpage'
# you'll get formatted html.
```
