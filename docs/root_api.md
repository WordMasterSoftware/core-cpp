# 首页接口说明

## 接口地址

- `GET /`

## 作用

首页接口仅用于说明当前服务是 `WordMaster Next API`，不承载业务数据。

## 成功响应

```json
{
  "name": "WordMaster Next API",
  "status": "ok",
  "version": "v1"
}
```

## 使用建议

- 将首页接口作为服务探活和基础识别入口。
- 新增业务能力时，不要继续往首页接口堆叠说明字段，业务说明应拆到独立文档和独立 API 中。
