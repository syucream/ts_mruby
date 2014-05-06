TARGET = ts_mruby.so
SOURCES = src/ts_mruby_*.cpp src/ts_mruby.cpp


# support mrbgems
MRUBY_MAK_FILE = $(MRUBY_ROOT)/build/host/lib/libmruby.flags.mak
include $(MRUBY_MAK_FILE)

# configure settings related to ATS and mruby
CPPFLAGS += -I$(TS_PREFIX_ROOT)include -I$(MRUBY_ROOT)include
LDFLAGS  += -L$(TS_PREFIX_ROOT)lib $(MRUBY_LDFLAGS)
LIBS     =  -latscppapi $(MRUBY_LIBS)


all: ts_mruby.la ts_mruby.so

ts_mruby.la:
	$(TS_PREFIX_ROOT)bin/tsxs $(CPPFLAGS) $(LDFLAGS) $(LIBS) $(SOURCES)

ts_mruby.so:
	$(CXX) -bundle -flat_namespace -undefined suppress $(LDFLAGS) -o $(TARGET) src/*.lo $(LIBS)

# build mruby
build_mruby:
	cp build_config.rb $(MRUBY_ROOT)/build_config.rb && cd $(MRUBY_ROOT) && ruby ./minirake && cd $(MRUBY_ROOT) && git checkout build_config.rb


clean:
	rm -f src/*.lo
	rm -f $(TARGET)
