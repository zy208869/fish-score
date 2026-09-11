// Config.h - 插件配置加载
#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include <optional>
#include <fstream>

namespace FishScore {

// API 配置
struct ApiConfig {
    std::string baseUrl;        // API 基础地址
    std::string apiVersion;     // API 版本路径
    int timeout;                // 超时时间（毫秒）
    std::string apiKey;         // API 密钥（可选）
};

// 缓存配置
struct CacheConfig {
    int durationMinutes;        // 缓存时长（分钟）
    bool enabled;               // 是否启用缓存
};

// UI 配置
struct UiConfig {
    int songsPerPage;           // 每页显示歌曲数
    bool showGrade;             // 是否显示评级
    bool showAccuracy;          // 是否显示准确率
    bool autoRefreshOnJoin;     // 玩家加入时是否自动刷新
};

// 完整配置
struct PluginConfig {
    ApiConfig api;
    CacheConfig cache;
    UiConfig ui;
    std::string language;       // 语言
};

// 配置加载器
class ConfigLoader {
public:
    // 从文件加载配置
    static std::optional<PluginConfig> load(const std::string& filePath);

    // 保存默认配置到文件
    static bool saveDefault(const std::string& filePath);

    // 获取默认配置文件路径
    static std::string getDefaultConfigPath();

private:
    static PluginConfig parseJson(const nlohmann::json& json);
    static PluginConfig getDefault();
};

} // namespace FishScore
