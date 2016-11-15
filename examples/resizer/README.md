# Image resizer example

Resize a response image from origin server by using ImageMagick in-memory.

## requirements

* ImageMagick(and `mruby-mrmagick`)

## How to use

* 1. build mruby with [mruby-mrmagick](https://github.com/kjunichi/mruby-mrmagick)
* 2. build ts_mruby with `-lMagick++`
* 3. prepare mruby script and ATS config. For example:

remap.config
```
map /resizer http://127.0.0.1:80/images/test.jpg @plugin=ts_mruby.so @pparam=/Users/ryo/local/etc/trafficserver/resizer.rb
```

* 4. Run ATS
* 5. Request to ATS. For example:

```
$ curl -v 'http://localhost:8080/resizer?scale=0.5' > /dev/null
# you can get 0.5 scaled test.jpg
```
