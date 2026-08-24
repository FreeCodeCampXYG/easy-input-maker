# T03 · Host Action v1 纯逻辑实现

- **背景**：T02 已冻结协议；当前 starter 尚未实现 Host Action。
- **目标**：先用宿主测试固定规范小写 UUID 校验、完整配置值解析、Keymap 按下／松开语义和共享报告编码，再做最小纯逻辑实现。
- **输入**：`flow/tasks/T02-Host-Action-v1-固定兼容协议.md`、现有配置／Keymap／App Command 容器。
- **产出路径**：由学员根据现有分层选择 `components/keyboard/` 与 `host_test/` 中的最小文件范围。
- **验收标准**：合法 UUID 接受；大写、长度、连字符或字符错误 fail closed；不增加 version／nil 限制；按下一个事件、松开无事件；固定 wire 字段通过测试；本节点不接 USB／BLE。
- **状态**：待办
