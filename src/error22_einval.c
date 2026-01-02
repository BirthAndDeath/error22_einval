#include "error22_einval.h"
#include "c_api.h"
LogCallback g_log_callback = NULL;
// A very short-lived native function.
//
// For very short-lived functions, it is fine to call them on the main isolate.
// They will block the Dart execution while running the native function, so
// only do this for native functions which are guaranteed to be short-lived.
FFI_PLUGIN_EXPORT int sum(int a, int b) { return a + b; }

// A longer-lived native function, which occupies the thread calling it.
//
// Do not call these kind of native functions in the main isolate. They will
// block Dart execution. This will cause dropped frames in Flutter applications.
// Instead, call these native functions on a separate isolate.
FFI_PLUGIN_EXPORT int sum_long_running(int a, int b) {
  // Simulate work.
#if _WIN32
  Sleep(5000);
#else
  usleep(5000 * 1000);
#endif
  return a + b;
}
FFI_PLUGIN_EXPORT void set_log_callback(LogCallback callback) {
  g_log_callback = callback;
}
FFI_PLUGIN_EXPORT void init(void) { c_init(); }
FFI_PLUGIN_EXPORT int loadmodel(const char *model_path) {
  return c_load_model(model_path);
}
