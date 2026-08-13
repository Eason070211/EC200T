# EC200T-CN PCB 设计方案（台式机PCIe/USB插卡4G上网）
适配EC20 / EC200T-CN Cat4 4G通信模块，专为台式电脑插卡上网场景优化，参考旧版仓库：https://gitcode.com/u013985601/EC20.git

## 一、核心硬件规格升级点
1. **模块选型**
主芯片：EC200T-CN（Cat4，国内4G全网通），兼容引脚同封装EC20系列，硬件可兼容替换。
2. **宽幅增强供电系统（重点优化）**
输入电压范围：**5V–15V** 宽压输入，适配台式机机箱12V硬盘供电/USB 5V双取电；
增设多级滤波、防反接PMOS、TVS浪涌防护、大容量储能钽电容，解决4G模块发射峰值电流压降、掉线问题；
内置低压差降压芯片，稳定输出模块标准3.8V核心供电。
3. **射频天线链路优化**
外置标准SMA-K射频座，直连4G LTE全频段天线；
射频走线严格50Ω阻抗控制，缩短微带线长度，减少射频损耗；预留ESD/TVS射频防护，隔离数字噪声干扰天线接收。
4. **台式机适配输出接口（新增）**
双输出方案可选，满足台式插卡需求：
- 方案A：USB2.0 Type-A（直插主板后置USB口，免驱动串口/网络RNDIS拨号上网）
- 方案B：PCIe转USB子卡布局（适配机箱PCIe插槽固定，机箱外置SMA天线）
5. **SIM卡座电路优化**
独立3.3V SIM电源，信号线双向低压ESD防护；SIM区域与射频天线分区布局，做地层隔离，降低射频干扰造成的读卡失败。

## 二、PCB布局关键改进（对比旧版）
1. 电源分区：数字电源、模块3.8V核心电源、射频电源分割独立地层，单点接地，杜绝地环路噪声；
2. 功率回路缩短：输入滤波、降压芯片、模块电源引脚紧密排布，加粗电源铜箔，降低导通压降；
3. 射频隔离：SMA射频座、模块射频引脚集中PCB单边，与SIM卡、USB数据口拉开间距，用地层隔离屏蔽；
4. 散热优化：EC200T模块底部铺大面积接地铜皮，辅助发射工况散热，长时间满载不降速；
5. 防护完整性：输入防反接、电源浪涌TVS、射频ESD、SIM信号线ESD全链路防护，适配机箱静电、插拔浪涌场景。

## 三、功能说明
1. 供电兼容台式机12V大电流供电（推荐首选，峰值发射不掉网）、5V USB备用供电；
2. 标准RNDIS网卡模式，Windows/Linux免驱识别，即插即用4G上网；
3. 预留调试串口排针，支持AT指令调试、信号强度查询、APN配置；
4. SMA外置天线可搭配高增益吸盘天线，台式机放置窗边提升4G信号；
5. 硬件向下兼容旧版EC20模块，仅需更换贴片物料无需改版PCB。

## 四、精简版标题摘要（适合README头部）
# EC200T-CN PCB Project
Desktop PC PCIe/USB adapter for 4G LTE internet access
Compatible with EC20 / EC200T-CN Cat4 wireless module
Wide 5–15V enhanced power supply circuit, SMA external LTE antenna interface
Reference legacy design: https://gitcode.com/u013985601/EC20.git
