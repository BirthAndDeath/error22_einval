#pragma once
#include <stdint.h>
#include <stdio.h>
#include <sys/syslog.h>
#ifdef __ANDROID__
#include <android/log.h>
#define LOG_D(fmt, ...)                                                        \
  __android_log_print(ANDROID_LOG_DEBUG, "error22_einval", "[%s:%d] " fmt,     \
                      __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_I(fmt, ...)                                                        \
  __android_log_print(ANDROID_LOG_INFO, "error22_einval", "[%s:%d] " fmt,      \
                      __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_W(fmt, ...)                                                        \
  __android_log_print(ANDROID_LOG_WARN, "error22_einval", "[%s:%d] " fmt,      \
                      __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG_E(fmt, ...)                                                        \
  __android_log_print(ANDROID_LOG_ERROR, "error22_einval", "[%s:%d] " fmt,     \
                      __FILE__, __LINE__, ##__VA_ARGS__)
#else
enum class LogLevel { D, I, W, E };
template <LogLevel L, typename... Args>
static inline void log_print(const char *file, int line, const char *fmt,
                             Args... args) {
  fprintf(stderr, "[%c %s:%d] ", "DIWE"[static_cast<int>(L)], file, line);
  fprintf(stderr, fmt, args...);
  fprintf(stderr, "\n");
}
#define LOG_D(fmt, ...)                                                        \
  log_print<LogLevel::D>(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...)                                                        \
  log_print<LogLevel::I>(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...)                                                        \
  log_print<LogLevel::W>(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...)                                                        \
  log_print<LogLevel::E>(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#endif

#include <cstdio>
#include <cstring>
#include <syslog.h>
#include <vector>

class App {
public:
  static App &instance(); // 唯一入口
  App(const App &) = delete;
  void operator=(const App &) = delete;

private:
  App() = default;
  ~App() { /*
     for (auto &m : model) {
       //llama_model_free(m);
     }*/
  }
};
