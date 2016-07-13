# test executed on test/
export CPPFLAGS LDFLAGS LIBS MRUBY_LDFLAGS
SUBDIRS = test

.PHONY: all test clean

TARGET = ts_mruby.so
SOURCES = src/ts_mruby_*.cpp src/ts_mruby.cpp


# support mrbgems
MRUBY_MAK_FILE = $(MRUBY_ROOT)/build/host/lib/libmruby.flags.mak
include $(MRUBY_MAK_FILE)

# configure settings related to ATS and mruby
CPPFLAGS += -I$(TS_PREFIX_ROOT)include -I$(MRUBY_ROOT)include
LDFLAGS  += -L$(TS_PREFIX_ROOT)lib $(MRUBY_LDFLAGS)
LIBS     =  -latscppapi $(MRUBY_LIBS)


all: ts_mruby.so

ts_mruby.so:
	$(TS_PREFIX_ROOT)bin/tsxs $(CPPFLAGS) $(LDFLAGS) $(LIBS) -o $(TARGET) $(SOURCES)

install:
	$(TS_PREFIX_ROOT)bin/tsxs -i -o $(TARGET)

# build mruby
build_mruby:
	cp build_config.rb $(MRUBY_ROOT)/build_config.rb && cd $(MRUBY_ROOT) && ./minirake && cd $(MRUBY_ROOT) && git checkout build_config.rb

test:
	cd test/ && ./ts_mruby_test && cd ../

clean:
	cd test/ && make clean && cd ../
	rm -f src/*.lo
	rm -f $(TARGET)
