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
  "language": "en-US",
  "word": "apple",
  "audio_url": "https://tts.wordmaster.next/v1/audio/en-US/apple.mp3",
  "provider": "mock-tts",
  "cached": false,
  "message": "Mock pronunciation metadata generated successfully."
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
- 当前底层使用可替换的 `TtsService` 抽象，便于后续接入真实 TTS 提供方。
- 每次成功调用后，服务端会通过 WebSocket 推送 `tts.pronunciation.requested` 事件。
