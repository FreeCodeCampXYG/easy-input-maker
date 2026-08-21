# 决策日志（decisions）

> 这里只追加适合公开、对后续协作确有价值的过程决策。

## 2026-08-22 · 编码器文字选择使用原生键盘语义
- 背景：EasyInput App 已使用 `cursor + text_caret_select` 配置编码器文字选择；旧 Maker 固件缺少该动作，导致整份配置以 `unknown_action` 被拒绝。
- 决定：规范动作名固定为 `text_caret_select`，旧名称 `mouse_drag_select` 解析为同一动作；该动作只允许位于编码器按压位并配合 `cursor` 模式。短按切换选择状态，旋转发送完整的标准键盘 `Shift+ArrowRight/Left`，3 秒长按继续进入既有系统配置。默认仍为 `scroll_axis_toggle`。
- 原因：复用标准键盘报告可以兼容现有 USB/BLE 输入通道和真实按键状态，不需要新增鼠标拖拽、App Command 或宿主权限协议；严格槽位和模式校验可以继续保持配置 fail-closed。

<!-- 新记录模板：
## YYYY-MM-DD · <决策标题>
- 背景：
- 决定：
- 原因：
-->
