# 水鱼查分器 LeviMina 插件

> **版本**: 26.40.0  
> **构建工具**: xmake  
> **语言**: C++20

## 功能

- 🔍 查询玩家分数信息
- 🎵 搜索歌曲
- 📊 最佳成绩 (B30) / 最近成绩 / 全部记录
- 📄 分页显示（每页 **10 首歌**）
- 📋 单曲详情（判定、评级、准确率）
- 🔄 数据缓存（5分钟）

## 命令

| 命令 | 说明 |
|------|------|
| `/fishscore` | 打开主菜单 |
| `/fishscore query <UID>` | 快速查询玩家 |
| `/fishscore best <UID>` | 查询最佳成绩 |
| `/fishscore recent <UID>` | 查询最近成绩 |

## 构建

```bash
# 安装 xmake (如果未安装)
# 确保已安装 LeviMina SDK

# 构建
xmake build

# 构建并运行
xmake run
```

## 安装

1. 将编译生成的 `fish-score.dll` 复制到 BDS 服务器的 `plugins/` 目录
2. 重启服务器或使用 `ll load fish-score` 加载插件

## 项目结构

```
fish-score/
├── xmake.lua              # 构建配置
├── include/               # 公共头文件
│   ├── ScoreData.h        # 数据结构定义
│   ├── FishScoreAPI.h     # API 封装接口
│   ├── HttpCpr.h          # HTTP 客户端
│   └── Paginator.h        # 分页逻辑
└── src/                   # 源代码
    ├── main.cpp           # 插件入口
    ├── API/               # API 实现
    │   ├── FishScoreAPI.cpp
    │   └── HttpCpr.cpp
    └── GUI/               # 游戏内表单
        ├── MainForm.h/.cpp
        ├── QueryPlayerForm.h/.cpp
        ├── ScoreListForm.h/.cpp
        └── ScoreDetailForm.h/.cpp
```

## API 说明

本项目对接水鱼查分器 API，需要替换为实际可用的 API 地址和端点。当前使用的占位地址：

- 基础地址: `https://fish-score.com/api`
- V2 地址: `https://fish-score.com/api/v2`

### 主要 API 端点

```
GET /v2/{userId}                  - 获取玩家信息
GET /v2/{userId}/scores           - 获取玩家分数列表（支持分页）
  ?type=best|recent|all           - 查询类型
  ?page=1                         - 页码
  ?pageSize=10                    - 每页大小（默认10）
GET /v2/{userId}/score/{songId}   - 获取单曲分数
GET /v2/song/search?keyword=xxx   - 搜索歌曲
```

## 配置

编辑 `plugins/fish-score/config.json`：

```json
{
    "api": {
        "baseUrl": "https://fish-score.com",
        "apiVersion": "/api/v2",
        "timeout": 15000,
        "apiKey": ""
    },
    "cache": {
        "enabled": true,
        "durationMinutes": 5
    },
    "ui": {
        "songsPerPage": 10,
        "showGrade": true,
        "showAccuracy": true,
        "autoRefreshOnJoin": false
    },
    "language": "zh_CN"
}
```

| 配置项 | 说明 | 默认值 |
|--------|------|--------|
| `api.baseUrl` | 水鱼查分器 API 地址 | `https://fish-score.com` |
| `api.apiVersion` | API 版本路径 | `/api/v2` |
| `api.timeout` | 请求超时（毫秒） | `15000` |
| `api.apiKey` | API 密钥（可选） | 空 |
| `cache.enabled` | 是否启用缓存 | `true` |
| `cache.durationMinutes` | 缓存时长（分钟） | `5` |
| `ui.songsPerPage` | 每页显示歌曲数 | `10` |
| `ui.showGrade` | 显示评级 | `true` |
| `ui.showAccuracy` | 显示准确率 | `true` |
| `ui.autoRefreshOnJoin` | 加入游戏时自动提示 | `false` |
