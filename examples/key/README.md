# Key processor with Redis

[Key](https://tools.ietf.org/html/draft-ietf-httpbis-key-01) processor

## Requirements

* hiredis(and `mruby-redis`)

## How to use

* 1. build mruby with [mruby-redis](https://github.com/matsumoto-r/mruby-redis)
* 2. build ts_mruby with `-lhiredis`
* 3. prepare mruby script and ATS config. For example:

remap.config

```
map /key http://127.0.0.1:80/ @plugin=ts_mruby.so @pparam=/Users/ryo/local/etc/trafficserver/key.rb
```

* 4. Run Redis server

```
$ redis-server --port 6789
```

* 5. Setup your origin server to respond 'key' header field.
* 6. Run ATS.
* 7. Request with some header fields, and confirm cache status.
  - You can show cache status by [Via header](http://trafficserver.readthedocs.io/en/latest/appendices/faq.en.html?highlight=via#how-do-i-interpret-the-via-header-code)

## Examples

These examples are results of using nginx as an origin server.

### div

* nginx.conf

```
location / {
  ...
  add_header Cache-Control 'public, max-age=60';
  add_header key 'bar;div=5';
}
```

* Responses

```
$ curl -s -H "bar: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "bar: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "bar: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "bar: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '0'. At this time, ATS doesn't have a cache.

$ curl -s -H "bar: 3 , 42" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "bar: 4, 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also '0'. Its already cached by the request with 'bar: 1'.

$ curl -s -H "bar: 12" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "bar: 12" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "bar: 12" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
# Its result is '2'. At this time, ATS doesn't have a cache.

Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "bar: 10" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "bar: 14, 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also '2' so cache hit.
```

### partition

* nginx.conf

```
location / {
  ...
  add_header Cache-Control 'public, max-age=60';
  add_header key 'foo;partition=20:30:40';
}
```

* Responses

```
$ curl -s -H "foo: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "foo: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "foo: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "foo: 1" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
# Its result is 'the first segment(0)'. At this time, ATS doesn't have a cache.

Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "foo: 0" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "foo: 4, 54" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "foo: 19.9" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also 'the first segment(0)'. Its already cached by the request with 'foo: 1'.

$ curl -s -H "foo: 20" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "foo: 20" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "foo: 20" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is 'the second segment(1)'. At this time, ATS doesn't have a cache.

$ curl -s -H "foo: 29.999" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "foo: 24   , 10" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also 'the second segment(1)' so cache hit.
```

### match

* nginx.conf

```
location / {
  ...
  add_header Cache-Control 'public, max-age=60';
  add_header key 'baz;match="charlie"';
}
```

* Responses

```
$ curl -s -H "baz: charlie" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "baz: charlie" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "baz: charlie" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "baz: charlie" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '1'. At this time, ATS doesn't have a cache.

$ curl -s -H "baz: foo, charlie" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "baz: bar, charlie     , abc" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also '1'. Its already cached by the request with 'baz: charlie'.

$ curl -s -H "baz: theodore" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "baz: theodore" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "baz: theodore" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '0'. At this time, ATS doesn't have a cache.

$ curl -s -H "baz: joe, sam" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "baz: Charlie" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "baz: cha rlie" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "baz: charlie2" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also '0' so cache hit.

$ curl -s -H "baz: \"charlie\"" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# XXX Its expected '0', but currently returned '1' ...
```

### substr

* nginx.conf

```
location / {
  ...
  add_header Cache-Control 'public, max-age=60';
  add_header key 'abc;substr=bennet';
}
```

* Responses

```
$ curl -s -H "abc: bennet" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "abc: bennet" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "abc: bennet" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "abc: bennet" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '1'. At this time, ATS doesn't have a cache.

$ curl -s -H "abc: foo, bennet" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "abc: abennet00" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "abc: bar, 99bennet     , abc" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "abc: \"bennet\"" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also '1'. Its already cached by the request with 'abc: bennet'.

$ curl -s -H "abc: theodore" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "abc: theodore" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "abc: theodore" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '0'. At this time, ATS doesn't have a cache.

$ curl -s -H "abc: joe, sam" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "abc: Bennet" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "abc: Ben net" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# These result is also '0' so cache hit.
```

### param

* nginx.conf

```
location / {
  ...
  add_header Cache-Control 'public, max-age=60';
  add_header key 'def;param=liam';
}
```

* Responses

```
$ curl -s -H "def: liam=123" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "def: liam=123" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "def: liam=123" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "def: liam=123" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '123'.

$ curl -s -H "def: mno=456" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "def: mno=456" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "def: mno=456" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
$ curl -s -H "def: " http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is ''.

$ curl -s -H "def: abc=123; liam=890" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "def: abc=123; liam=890" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '890'.

$ curl -s -H "def: liam=\"678\"" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "def: liam=\"678\"" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '"678"'.
```

### mixed

* nginx.conf

```
location / {
  ...
  add_header Cache-Control 'public, max-age=60';
  add_header key 'user-agent;substr=MSIE;substr=mobile, cookie;param=ID';
}
```

* Responses

```
$ curl -s -H "user-agent: foo" -H "cookie: foo=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "user-agent: foo" -H "cookie: foo=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
$ curl -s -H "user-agent: foo" -H "cookie: foo=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cHs f ])
$ curl -s -H "user-agent: foo" -H "cookie: foo=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cRs f ])
# Its result is '0\0000\000'.

$ curl -s -H "user-agent: MSIE" -H "cookie: foo=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '1\0000\000'.

$ curl -s -H "user-agent: mobile" -H "cookie: foo=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '0\0001\000'.

$ curl -s -H "user-agent: MSIE-mobile" -H "cookie: foo=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '1\0001\000'.

$ curl -s -H "user-agent: foo" -H "cookie: ID=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '0\0000\000bar'.

$ curl -s -H "user-agent: foo" -H "cookie: ID=baz" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '0\0000\000baz'.

$ curl -s -H "user-agent: MSIE" -H "cookie: ID=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '1\0000\000bar'.

$ curl -s -H "user-agent: mobile" -H "cookie: ID=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '0\0001\000bar'.

$ curl -s -H "user-agent: MSIE-mobile" -H "cookie: ID=bar" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '1\0001\000bar'.

$ curl -s -H "user-agent: MSIE" -H "cookie: ID=baz" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '1\0000\000baz'.

$ curl -s -H "user-agent: mobile" -H "cookie: ID=baz" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '0\0001\000baz'.

$ curl -s -H "user-agent: MSIE-mobile" -H "cookie: ID=baz" http://127.0.0.1:8080/ -o /dev/null --dump-header - | grep Via
Via: http/1.1 Macintosh.local (ApacheTrafficServer/6.0.0 [cMsSfW])
# Its result is '1\0001\000baz'.
```
