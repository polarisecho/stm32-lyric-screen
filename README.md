# stm32-lyric-screen
基于上位机发送字模的可添加特效的歌词显示器
按照[江协科技的stm32启动文件配置方式](https://www.bilibili.com/video/BV1th411z7sn/?share_source=copy_web&vd_source=5e17989c487ce7173c18acfa7a34fa30)添加start和library


# 依赖
- STM32F10x Standard Peripheral Library **v3.5.0**
- Download: [ST Official Archive](https://www.st.com/en/embedded-software/stsw-stm32054.html)

# Third-Party Code

This project uses code from the following open-source project:

- **Project**: [oledlib](https://gitee.com/jiezhuonew/oledlib)  
- **Author**: jiezhuonew  
- **License**: [Mulan Permissive Software License, Version 1 (MulanPSL-1.0)](LICENSE-MulanPSL)  
- **Used Components**:  
  - OLED initialization routine (`OLED_Init()`)  
  - Line drawing function (`OLED_DrawLine()`)

The original project is licensed under MulanPSL-1.0. We thank the author for their contribution to the open-source community.

# 上位机与歌词获取（示例）
通过 **BetterNCM + LyricsToPort** 插件从网易云音乐实时获取歌词，并转发至本地 HTTP 服务。

1. 安装 [网易云音乐](https://music.163.com/#/download)
2. 安装 [BetterNCM](https://github.com/MicroCBer/BetterNCM)
3. 在 BetterNCM 插件市场中安装 **[LyricsToPort](https://github.com/Yueby/LyricsToPort)**
4. 在插件设置中选择端口

⚠️ 本方案仅用于个人学习与非商业用途。请遵守网易云音乐用户协议
