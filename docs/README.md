# 文档总目录

本目录用于维护 `core-cpp/` 相关接口与能力文档。

维护规则：

- 后续新增任何接口、服务能力或对外使用方式说明时，都应在 `docs/` 下新增独立 Markdown 文件。
- 每次新增文档后，都必须同步更新本文件，补充对应链接与简要说明。
- 文档文件名应尽量直接反映主题，例如 `websocket_notifier.md`、`tts_api.md`、`root_api.md`。

## 文档列表

- [首页接口说明](./root_api.md)
  说明根路径 `GET /` 的用途和返回结构。

- [TTS 接口说明](./tts_api.md)
  说明 `GET /api/tts` 的查询参数、返回结构和错误处理。

- [WebSocketNotifier 使用说明](./websocket_notifier.md)
  说明服务端如何通过 `WebSocketNotifier` 主动向 WebSocket 客户端推送消息。
