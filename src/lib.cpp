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
Ort::Env &App::GetEnv() {
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

/**
 * @brief 从指定路径加载模型文件
 *
 * 此函数执行安全验证以防止路径遍历攻击，验证文件扩展名，
 * 检查文件是否存在、是否为常规文件以及文件大小限制，
 * 最后使用ONNX Runtime加载模型。
 *
 * @param path 模型文件的UTF-8编码路径
 * @return int 返回状态码，0表示成功，1表示失败
 */
int load_model(const char *path) {
  std::string path_str(path);

  // 防止路径遍历攻击 - 更严格的检查
  size_t pos = 0;
  while ((pos = path_str.find("..", pos)) != std::string::npos) {
    // 检查..是否是独立路径段（前后是路径分隔符或开头/结尾）
    bool is_path_traversal = false;
    if (pos > 0 && (path_str[pos - 1] == '/' ||
                    path_str[pos - 1] == '\\')) { // 前面是路径分隔符
      is_path_traversal =
          (pos + 2 == path_str.length() || path_str[pos + 2] == '/' ||
           path_str[pos + 2] == '\\'); // 后面是路径分隔符或结尾
    } else {
      is_path_traversal =
          (pos + 2 < path_str.length() && path_str[pos + 2] == '/' ||
           path_str[pos + 2] == '\\'); // 后面是路径分隔符
    }

    if (is_path_traversal) {
      LOG_E("Invalid path: %s, contains '..' as directory traversal", path);
      return 1;
    }
    pos++;
  }

  // 验证文件扩展名是否为允许的模型格式
  std::string allowed_extensions[] = {".onnx", ".ort", ".onnxmodel"};
  bool valid_extension = false;
  for (const auto &ext : allowed_extensions) {
    if (path_str.length() >= ext.length() &&
        path_str.substr(path_str.length() - ext.length()) == ext) {
      valid_extension = true;
      break;
    }
  }

  if (!valid_extension) {
    LOG_E("Invalid model file extension for path: %s", path);
    return 1;
  }

  // 检查文件是否存在
  std::error_code ec;
  if (!std::filesystem::exists(path_str, ec)) {
    if (ec) {
      LOG_E("Error accessing file %s: %s", path, ec.message().c_str());
    } else {
      LOG_E("Failed to load model from %s, the file does not exist", path);
    }
    return 1;
  }

  // 检查是否为常规文件
  if (!std::filesystem::is_regular_file(path_str, ec)) {
    if (ec) {
      LOG_E("Error accessing file %s: %s", path, ec.message().c_str());
    } else {
      LOG_E("Failed to load model from %s, not a regular file", path);
    }
    return 1;
  }

  // 检查文件大小，防止加载过大的模型文件
  auto file_size = std::filesystem::file_size(path_str, ec);
  if (ec) {
    LOG_E("Failed to get file size for %s: %s", path, ec.message().c_str());
    return 1;
  }

  const size_t max_file_size = 1UL * 1024 * 1024 * 1024; // 1GB limit
  if (file_size > max_file_size) {
    LOG_E("Model file too large: %s, size: %zu bytes, max allowed: %zu bytes",
          path, file_size, max_file_size);
    return 1;
  }

  try {
    // 获取全局App实例并使用其session options加载模型
    Ort::Env &env = App::instance().GetEnv();

    // 释放之前可能存在的session以避免内存泄漏
    if (App::instance().session != nullptr) {
      App::instance().session = nullptr;
    }
    // 创建新的session
    App::instance().session =
        std::make_unique<Ort::Session>(env, path, App::instance().opts);

    LOG_I("Model loaded successfully from %s", path);
    return 0;
  } catch (const Ort::Exception &e) {
    LOG_E("Failed to load model from %s: %s", path, e.what());
    return 1;
  } catch (const std::exception &e) {
    LOG_E("Failed to load model from %s: %s", path, e.what());
    return 1;
  }
}

extern "C" {
// c api here
void c_init() { app_init(); }
int c_load_model(const char *path) { return load_model(path); };
}