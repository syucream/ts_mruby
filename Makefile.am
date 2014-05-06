TARGET = ts_mruby.so
SOURCES = src/ts_mruby_*.cpp src/ts_mruby.cpp

CPPFLAGS += -I$(TS_PREFIX_ROOT)include
LDFLAGS  += -L$(TS_PREFIX_ROOT)lib
LIBS     =  -lmruby -latscppapi

all: ts_mruby.la ts_mruby.so

ts_mruby.la:
	$(TS_PREFIX_ROOT)bin/tsxs $(CPPFLAGS) $(LDFLAGS) $(LIBS) $(SOURCES)

ts_mruby.so:
	$(CXX) -bundle -flat_namespace -undefined suppress $(LDFLAGS) -o $(TARGET) src/*.lo $(LIBS)

clean:
	rm -f src/*.lo
	rm -f $(TARGET)
