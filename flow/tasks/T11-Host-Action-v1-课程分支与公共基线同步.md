# T11 · Host Action v1 课程分支与公共基线同步

- **背景**：Maker 完整基线已经包含 Host Action v1 与 BLE 配置持久化修复；学员需要在不退回旧固件的前提下，按课程节点自行实现 Host Action。产品专属 OTA 不属于本课程或 Maker 公开同步范围。
- **目标**：建立完整 `main` 与 `course/host-action-v1-starter` 的可验证关系；starter 只缺 Host Action，其他公共固件能力保持一致；准备产品侧公共代码同步时隔离现有 OTA 工作现场。
- **输入**：冻结的 Host Action v1 合同、当前 Maker `main`、进入 OTA 前的公共固件事实、EasyInput V2.0 硬件边界、ESP-IDF 5.5.5。
- **产出路径**：`AGENTS.md`、`README.md`、`docs/teaching/ai-vibe-coding.md`、本任务卡、两个本地课程标签和课程分支；产品侧同步只使用独立工作树／分支，不覆盖进行中的工作区。
- **验收标准**：
  1. Maker `main` 明确为完整公共基线，starter 明确只移除 Host Action 的生产实现与答案型测试；
  2. BLE 持久化修复、编码器、音频、电源、GPIO、BOOT、GPIO8、设备身份、HID／GATT 和既有分区在 starter 中保持；
  3. 产品专属 OTA、签名、发布与部署文件不进入 Maker 任一分支；
  4. `main` 与 starter 分别重新运行全部宿主测试和 ESP-IDF 5.5.5 默认构建；
  5. 用差异清单和禁止项扫描证明课程差异，不以删除／跳过无关测试制造通过；
  6. 建立 `course-host-action-v1-complete-v1` 与 `course-host-action-v1-start-v1` 本地固定标签；未经另行授权不推送，不访问设备或烧录。
- **完成证据**：
  - Maker 完成版基线提交 `294ca11`，`course-host-action-v1-complete-v1` 固定指向该课程完成版；宿主测试 `60/60`，ESP-IDF 5.5.5 / v2 / ESP32-S3 默认构建通过，App 镜像 1,640,528 bytes。
  - `course/host-action-v1-starter@743368a` 只移除 Host Action 生产实现与 3 项答案型独立测试，`course-host-action-v1-start-v1` 固定课程起点；宿主测试 `57/57`，默认构建通过，App 镜像 1,639,168 bytes。
  - starter 与完整基线的 GPIO、BOOT、GPIO8、板级引脚、设备身份、HID／GATT、BLE 持久化、Flash 分区和 sdkconfig 受保护文件零差异；固定示例 UUID 没有进入生产配置或生产源码。
  - 产品侧在隔离工作树完成公共代码同步验证：公共 Host Action、状态预算和隐私日志边界对齐，生产专属 OTA 原样保留；宿主测试与锁定 IDF 构建通过。产品仓路径、内部发布现场和设备信息不写入本公开任务卡。
  - 两个仓库均只形成本地分支／提交；未合并生产同步分支，未推送、未签名发布、未连接设备或烧录。
- **状态**：已完成
