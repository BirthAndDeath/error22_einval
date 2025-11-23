#include "lib.hpp"
#include "error22_einval.h"
#include <iostream>

#include <string>
App &App::instance() {
  static App inst; // 线程安全、延迟构造
  return inst;
}
#include "c_api.h"
#include "error22_einval.h"
#include "ggml-backend.h"

#include "lib.hpp"

void app_init() {

  llama_log_set(
      [](enum ggml_log_level level, const char *text, void * /* user_data */) {
        // 设置日志级别，只打印错误信息
        if (level >= GGML_LOG_LEVEL_ERROR) {
          log_func(text);
          LOGE("%s", text);
          fprintf(stderr, "%s", text);
        }
      },
      nullptr);
  // 加载动态后端 todo:在运行时加载检测vulkan等显存是否足够？
  ggml_backend_load_all();
  log_func("Hello World! world.execute(me);\n");
  // App::instance();
}
extern "C" {
void c_init() { app_init(); }
}
/*
 * 主函数，实现了一个基于LLaMA模型的交互式聊天程序
 * 支持通过命令行参数指定模型文件、上下文大小和GPU层的数量
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序执行结果，0表示成功，非0表示失败
 */
FFI_PLUGIN_EXPORT int test() {
  std::cout << "Hello World!" << std::endl;

  std::string model_path = "~/projects/error22_einval/model/"
                           "SmallThinker-4B-A0.6B-Instruct.Q4_K_S.gguf";
  int ngl = 0; // 99
  int n_ctx = 2048;

  if (model_path.empty()) {

    return 1;
  }

  // 初始化模型参数并加载模型
  llama_model_params model_params = llama_model_default_params();
  model_params.n_gpu_layers = ngl;

  llama_model *model =
      llama_model_load_from_file(model_path.c_str(), model_params);

  if (!model) {
    fprintf(stderr, "%s: error:无法加载模型\n", __func__);
    llama_model_free(model);

    return 1;
  }

  const llama_vocab *vocab = llama_model_get_vocab(model);

  // 初始化上下文参数并创建上下文
  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_ctx = n_ctx;
  ctx_params.n_batch = n_ctx;

  llama_context *ctx = llama_init_from_model(model, ctx_params);
  if (!ctx) {
    fprintf(stderr, "%s: error: failed to create the llama_context\n",
            __func__);
    return 1;
  }

  // 初始化采样器链
  llama_sampler *smpl =
      llama_sampler_chain_init(llama_sampler_chain_default_params());
  llama_sampler_chain_add(smpl, llama_sampler_init_min_p(0.05f, 1));
  llama_sampler_chain_add(smpl, llama_sampler_init_temp(0.8f));
  llama_sampler_chain_add(smpl, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));

  // 定义生成响应的lambda函数
  auto generate = [&](const std::string &prompt) {
    std::string response;

    const bool is_first =
        llama_memory_seq_pos_max(llama_get_memory(ctx), 0) == -1;

    // 对提示词进行分词处理
    const int n_prompt_tokens = -llama_tokenize(
        vocab, prompt.c_str(), prompt.size(), NULL, 0, is_first, true);
    std::vector<llama_token> prompt_tokens(n_prompt_tokens);
    if (llama_tokenize(vocab, prompt.c_str(), prompt.size(),
                       prompt_tokens.data(), prompt_tokens.size(), is_first,
                       true) < 0) {
      GGML_ABORT("failed to tokenize the prompt\n");
    }

    // 准备批处理数据
    llama_batch batch =
        llama_batch_get_one(prompt_tokens.data(), prompt_tokens.size());
    llama_token new_token_id;
    while (true) {
      // 检查上下文空间是否足够
      int n_ctx = llama_n_ctx(ctx);
      int n_ctx_used = llama_memory_seq_pos_max(llama_get_memory(ctx), 0) + 1;
      if (n_ctx_used + batch.n_tokens > n_ctx) {
        printf("\033[0m\n");
        fprintf(stderr, "context size exceeded\n");
        exit(0);
      }

      int ret = llama_decode(ctx, batch);
      if (ret != 0) {
        GGML_ABORT("failed to decode, ret = %d\n", ret);
      }

      // 采样下一个token
      new_token_id = llama_sampler_sample(smpl, ctx, -1);

      // 判断是否结束生成
      if (llama_vocab_is_eog(vocab, new_token_id)) {
        break;
      }

      // 将token转换为字符串并输出
      char buf[256];
      int n =
          llama_token_to_piece(vocab, new_token_id, buf, sizeof(buf), 0, true);
      if (n < 0) {
        GGML_ABORT("failed to convert token to piece\n");
      }
      std::string piece(buf, n);
      printf("%s", piece.c_str());
      fflush(stdout);
      response += piece;

      // 准备下一批处理数据
      batch = llama_batch_get_one(&new_token_id, 1);
    }

    return response;
  };

  // 聊天循环相关变量初始化
  std::vector<llama_chat_message> messages;
  std::vector<char> formatted(llama_n_ctx(ctx));
  int prev_len = 0;
  while (true) {
    // 获取用户输入
    printf("\033[32m> \033[0m");
    std::string user;
    std::getline(std::cin, user);

    if (user.empty()) {
      break;
    }

    const char *tmpl = llama_model_chat_template(model, /* name */ nullptr);

    // 添加用户消息并格式化
    messages.push_back({"user", strdup(user.c_str())});
    int new_len =
        llama_chat_apply_template(tmpl, messages.data(), messages.size(), true,
                                  formatted.data(), formatted.size());
    if (new_len > (int)formatted.size()) {
      formatted.resize(new_len);
      new_len =
          llama_chat_apply_template(tmpl, messages.data(), messages.size(),
                                    true, formatted.data(), formatted.size());
    }
    if (new_len < 0) {
      fprintf(stderr, "failed to apply the chat template\n");
      return 1;
    }

    // 提取用于生成响应的提示词
    std::string prompt(formatted.begin() + prev_len,
                       formatted.begin() + new_len);

    // 生成并打印响应
    printf("\033[33m");
    std::string response = generate(prompt);
    printf("\n\033[0m");

    // 将响应添加到消息列表中
    messages.push_back({"assistant", strdup(response.c_str())});
    prev_len = llama_chat_apply_template(tmpl, messages.data(), messages.size(),
                                         false, nullptr, 0);
    if (prev_len < 0) {
      fprintf(stderr, "failed to apply the chat template\n");
      return 1;
    }
  }

  // 释放资源
  for (auto &msg : messages) {
    free(const_cast<char *>(msg.content));
  }
  llama_sampler_free(smpl);
  llama_free(ctx);
  llama_model_free(model);

  return 0;
}