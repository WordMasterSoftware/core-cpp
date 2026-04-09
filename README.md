# WordMaster Backend C++
WordMaster 智能背单词系统后端 API

C++ 重构版本，下一代的 WM 后端系统

# 说明
- 本项目优先采用 GCC 15.2.0 进行编译

[下载](https://mirrors.aliyun.com/gnu/gcc/gcc-15.2.0/) 或 手动编译 最新版本编译器

- TTS 当前改为构建阶段自动拉取并静态编译 `Flite` 源码，运行时不依赖系统 `flite` 命令。
- 第一次构建需要可访问 GitHub，以便拉取 `festvox/flite` 源码。
- 项目已经接入 PostgreSQL 与 Redis 的客户端依赖，后续可直接在数据层实现数据库和缓存访问。

- 项目编译：

```bash
bash core-cpp/build.sh
```

- API 文档

[总API文档手册](./docs/README.md)
