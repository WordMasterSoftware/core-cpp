# TTS 接口说明

## 接口地址

- `GET /api/tts`

## 请求参数

使用 query 参数传递请求数据：

- `language`：发音语言代码，例如 `en-US`、`en-GB`、`ja-JP`
- `word`：要获取发音的单词或短语

请求示例：

```text
GET /api/tts?language=en-US&word=apple
```

## 成功响应

```json
{
  "status": true,
  "audio_url": "/audio_cache/7a55fb4387f62d53.wav",
  "cached": false,
  "language": "en-US",
  "word": "apple",
  "message": "Flite pronunciation audio generated successfully.",
  "provider": "flite"
}
```

## 错误响应

以下情况返回 `400`：

- 缺少 `language`
- 缺少 `word`
- `language` 不是合法语言代码
- 参数在去除空白后为空字符串

错误响应示例：

```json
{
  "error": "Query parameters 'language' and 'word' are required, and language must be a valid code."
}
```

## 说明

- 第一版接口返回音频元数据，不直接返回音频流。
- `audio_url` 返回的是后端可访问路径，不是外部 HTTP/HTTPS 绝对地址。
- 音频文件实际生成并缓存到服务端的 `./audio_cache` 目录下。
- `./audio_cache/index.txt` 会维护哈希到音频文件名的索引，优先通过索引定位缓存，而不是每次扫描目录。
- 缓存文件名不使用单词原文，而是基于 `language + word` 的稳定哈希生成。
- 如果命中相同请求，接口会直接复用缓存文件，并将 `cached` 返回为 `true`。
- `status` 表示当前请求是否成功拿到了可用音频文件。
- 当前版本改为在构建阶段拉取并静态编译 `Flite` 源码，运行时不依赖系统 `flite` 命令。
- 当前嵌入式 `Flite` 配置优先支持英语请求。
- 每次成功调用后，服务端会通过 WebSocket 推送 `tts.pronunciation.requested` 事件。
