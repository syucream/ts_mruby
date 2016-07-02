#include <map>
#include <string>
#include <iostream>
#include <fstream>

#ifndef TS_MRUBY_H
#define TS_MRUBY_H

#ifdef MOCKING
#define MOCKABLE_ATTR virtual
#else
#define MOCKABLE_ATTR
#endif // MOCKING

class MrubyScriptsCache {
public:
  MOCKABLE_ATTR
  void store(const std::string &filepath) {
    std::ifstream ifs(filepath);
    const std::string code((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

    scripts_.insert(make_pair(filepath, code));
  }

  MOCKABLE_ATTR
  const std::string &load(const std::string &filepath) { return scripts_[filepath]; }

#ifdef MOCKING
  using mock_type = class MrubyScriptsCacheMock;
#endif // MOCKING

private:
  std::map<std::string, std::string> scripts_;
};

#endif // TS_MRUBY_H
