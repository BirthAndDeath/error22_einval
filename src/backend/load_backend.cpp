#include "load_backend.h"
void load_backend() {
  App::instance().opts.SetGraphOptimizationLevel(
      GraphOptimizationLevel::ORT_ENABLE_ALL);
}