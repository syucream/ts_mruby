# test executed on test/
export CPPFLAGS CXXFLAGS LDFLAGS LIBS MRUBY_ROOT COVFLAGS
SUBDIRS = test

.PHONY: all test clean

TARGET = ts_mruby.so
SOURCES = src/ts_mruby_*.cpp src/ts_mruby.cpp

# support mrbgems
MRUBY_MAK_FILE = $(MRUBY_ROOT)/build/host/lib/libmruby.flags.mak
-include $(MRUBY_MAK_FILE)

# configure settings related to ATS and mruby
CPPFLAGS += -I$(TS_PREFIX_ROOT)/include -I$(MRUBY_ROOT)/include
LDFLAGS  += -L$(TS_PREFIX_ROOT)/lib $(MRUBY_LDFLAGS)
LIBS     =  -latscppapi $(MRUBY_LIBS)


all: ts_mruby.so

ts_mruby.so:
	$(TS_PREFIX_ROOT)bin/tsxs $(CPPFLAGS) $(LDFLAGS) $(LIBS) -o $(TARGET) $(SOURCES)

install:
	$(TS_PREFIX_ROOT)bin/tsxs -i -o $(TARGET)

build_mruby:
	MRUBY_CONFIG=./build_config.rb $(MRUBY_ROOT)/minirake -f $(MRUBY_ROOT)/Rakefile

test:
	./test/ts_mruby_test

coverage:
	mkdir -p test/coverage
	cd test/coverage && gcov ../ts_mruby_test.cpp && find . -type f -not -name "*ts_mruby*" -or -name "*ts_mruby_test*" | xargs rm && cd ../../

clean:
	cd test/ && make clean && cd ../
	rm -f **/*.gcov
	rm -f **/*.gcda
	rm -f **/*.gcno
	rm -f **/*.lo
	rm -f **/*.o
	rm -f $(TARGET)
