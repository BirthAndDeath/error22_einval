#include "lib.hpp"
#include "./backend/load_backend.h"
#include "error22_einval.h"
#include <string>

// 使用onnxruntime的小尝试
#include "c_api.h"
#include "error22_einval.h"
App &App::instance() {
  static App inst; // 线程安全、延迟构造

  return inst;
}
Ort::Env &GetEnv() {
  static Ort::Env env(ORT_LOGGING_LEVEL_WARNING, "env");
  return env;
}
void app_init() {

  // 加载动态后端
  const char *what_string;
  try {
    my_log("initing...");

    unsigned int num_cores = std::thread::hardware_concurrency();
    App::instance().opts.SetIntraOpNumThreads(
        num_cores); // todo 自动测试最佳性能

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
}
#include <filesystem>

int load_model(const char *path) {
  // 防止路径遍历攻击
  std::string path_str(path);
  if (path_str.find("..") != std::string::npos) {
    LOG_E("Invalid path: %s, contains '..'", path);
    return 1;
  }

  std::error_code ec;
  if (!std::filesystem::exists(path, ec)) {
    if (ec) {
      LOG_E("Error accessing file %s: %s", path, ec.message().c_str());
    } else {
      LOG_E("Failed to load model from %s, the file does not exist", path);
    }
    return 1;
  }

  // 检查是否为常规文件
  if (!std::filesystem::is_regular_file(path, ec)) {
    if (ec) {
      LOG_E("Error accessing file %s: %s", path, ec.message().c_str());
    } else {
      LOG_E("Failed to load model from %s, not a regular file", path);
    }
    return 1;
  }

  try {
    // 获取全局App实例并使用其session options加载模型
    Ort::Env &env = App::instance().GetEnv();
    Ort::SessionOptions session_options;

    // 创建session
    App::instance().session = new Ort::Session(env, path, App::instance().opts);

    LOG_I("Model loaded successfully from %s", path);
    return 0;
  } catch (const Ort::Exception &e) {
    LOG_E("Failed to load model from %s: %s", path, e.what());
    return 1;
  } catch (const std::exception &e) {
    LOG_E("Failed to load model from %s: %s", path, e.what());
    return 1;
  }
};
extern "C" {
// c api here
void c_init() { app_init(); }
int c_load_model(const char *path) { return load_model(path); };
}