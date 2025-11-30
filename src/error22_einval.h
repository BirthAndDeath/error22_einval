#pragma once

#include <stdint.h>
#include <stdio.h>
#include <sys/syslog.h>

#if _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

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
// A very short-lived native function.
//
// For very short-lived functions, it is fine to call them on the main isolate.
// They will block the Dart execution while running the native function, so
// only do this for native functions which are guaranteed to be short-lived.
FFI_PLUGIN_EXPORT int sum(int a, int b);

// A longer lived native function, which occupies the thread calling it.
//
// Do not call these kind of native functions in the main isolate. They will
// block Dart execution. This will cause dropped frames in Flutter applications.
// Instead, call these native functions on a separate isolate.
FFI_PLUGIN_EXPORT int sum_long_running(int a, int b);

FFI_PLUGIN_EXPORT int load_model(const char *path);
FFI_PLUGIN_EXPORT int generate(int dialogue_id, int n_predict);
FFI_PLUGIN_EXPORT int dialogue_new(int model_id);
FFI_PLUGIN_EXPORT void shutdown();
FFI_PLUGIN_EXPORT void dialogue_free(int dialogue_id);

FFI_PLUGIN_EXPORT void init();

FFI_PLUGIN_EXPORT void set_log_callback(void (*callback)(const char *message));
typedef void (*LogCallback)(const char *message);

// 保存回调函数指针
extern LogCallback g_log_callback;
//@brief log日志函数
inline void my_log(const char *message) {

  LogCallback callback = g_log_callback;

  if (callback) {
    callback(message);
  } else {
    printf("INFO->:%s\n", message);
  }
}
