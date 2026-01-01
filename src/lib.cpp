#include "lib.hpp"
#include "./backend/load_backend.h"
#include "error22_einval.h"
#include <iostream>
#include <string>
App &App::instance() {
  static App inst; // 线程安全、延迟构造
  return inst;
}

#include "c_api.h"
#include "error22_einval.h"

void app_init() {

  // 加载动态后端
  const char *what_string;
  try {
    my_log("initing...");
    load_backend();

    what_string = "<init successfully>";
  } catch (const std::exception &e) {
    what_string = e.what();
  } catch (const char *s) {
    what_string = s;
  } catch (...) {
    what_string = "unknown exception";
  }
  my_log(what_string);

  my_log("Hello World! world.execute(me);\n");
  // App::instance();
}
#include <filesystem>
int load_model(const char *path) {
  if (!std::filesystem::exists(path)) {
    LOG_E("failed to load model from %s,the file not exits", path);
    return 1;
  }
};
extern "C" {
// c api here
void c_init() { app_init(); }
int c_load_model(const char *path) { return load_model(path); };
}
