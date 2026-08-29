# EasyInput Maker 开发状态

## 当前目标

- 维护 `my-host-action-practice` 的可复现宿主测试与 ESP-IDF 5.5.5 构建门禁。

## 已完成

- `read_source()` 使用二进制模式读取资源，并对文本源码统一 CRLF/LF，消除 Windows 平台差异。
- 新增 `.github/workflows/firmware-validation.yml`，执行宿主 CMake 编译、CTest 全量测试和 ESP32-S3 固件构建。
- 已创建个人 Fork：`FreeCodeCampXYG/easy-input-maker`；官方仓库保留为 `upstream`。
- 已创建并推送注释标签 `my-host-action-practice-v1`，指向提交 `9daf462`。
- 工作流补充上传同一提交对应的 ESP32-S3 固件 bundle，供统一硬件直接下载烧录。

## 验证

- 本地 `ctest --test-dir build-host --output-on-failure`：57/57 通过。
- GitHub Actions run `33275780034`：宿主编译、57 项测试和 ESP-IDF 固件构建均通过。

## 已知边界

- Actions 日志有 Node.js 20 弃用提示；当前不阻断工作流。
- 尚未执行烧录或实板验证；构建与实板观察仍是不同证据。

## 下一步

- 从 `flow/plan.md` 的 T03 节点继续课程实现；每个节点先补宿主测试，再实现和回归。
