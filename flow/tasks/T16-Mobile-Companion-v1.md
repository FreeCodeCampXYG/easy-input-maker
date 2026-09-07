# T16 · Mobile Companion v1

- 目标功能 ID：`mobile-companion`；不是槽位功能。
- 保留：USB/BLE PC HID、HostAction、legacy AppCommand、配置 Feature Report 和现有 HID Report Map。
- 风险控制：独立 connection generation；超时/断线/代际变化 fail-closed；Exclusive 仅在无 held keyboard 与 bridge hotkey 时批准。
- 验证：宿主 67/67；目标 ESP-IDF CI、App 联调和实板待验证。
