TARGETS=src/ts_mruby_*.cpp src/ts_mruby.cpp

CXXFLAGS = -std=c++11

all: ts_mruby.so

ts_mruby.so: $(TARGETS)
	$(CXX) $(CXXFLAGS) -shared $(CPPFLAGS) $(LDFLAGS) $(LIBS) $(TARGETS) -o ts_mruby.so
