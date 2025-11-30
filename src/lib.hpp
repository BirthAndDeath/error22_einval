#pragma once

#include "llama.h"

#include <cstdio>
#include <cstring>
#include <syslog.h>
#include <vector>

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
