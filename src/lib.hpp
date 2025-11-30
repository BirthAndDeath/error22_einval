#pragma once

#include "llama.h"
#include "load_backend.h"
#include <cstdio>
#include <cstring>
#include <syslog.h>
#include <vector>

#ifdef __ANDROID__
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "my_so", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "my_so", __VA_ARGS__)
#else
#define LOGI(...)                                                              \
  do {                                                                         \
    syslog(LOG_INFO, __VA_ARGS__);                                             \
    printf("I ");                                                              \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  } while (0)
#define LOGE(...)                                                              \
  do {                                                                         \
    syslog(LOG_ERR, __VA_ARGS__);                                              \
    printf("E ");                                                              \
    printf(__VA_ARGS__);                                                       \
    printf("\n");                                                              \
  } while (0)
#endif

class Dialogue {};
class ChatInstance {};
class App {
public:
  static App &instance(); // 唯一入口
  App(const App &) = delete;
  void operator=(const App &) = delete;
  std::vector<Dialogue> dialogues;
  std::vector<llama_model *> model;

private:
  App() = default;
  ~App() {
    for (auto &m : model) {
      llama_model_free(m);
    }
  }
};
