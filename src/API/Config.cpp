// Config.cpp - 配置加载实现
#include "Config.h"
#include <fstream>
#include <iostream>

namespace FishScore {

std::optional<PluginConfig> ConfigLoader::load(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // 配置文件不存在，返回默认配置
        return getDefault();
    }

    try {
        nlohmann::json json;
        file >> json;
        return parseJson(json);
    } catch (const std::exception& e) {
        // 解析失败，返回默认配置
        return getDefault();
    }
}

bool ConfigLoader::saveDefault(const std::string& filePath) {
    try {
        nlohmann::json json;
        
        json["api"]["baseUrl"]    = "https://fish-score.com";
        json["api"]["apiVersion"] = "/api/v2";
        json["api"]["timeout"]    = 15000;
        json["api"]["apiKey"]     = "";

        json["cache"]["enabled"]         = true;
        json["cache"]["durationMinutes"] = 5;

        json["ui"]["songsPerPage"]     = 10;
        json["ui"]["showGrade"]        = true;
        json["ui"]["showAccuracy"]     = true;
        json["ui"]["autoRefreshOnJoin"]= false;

        json["language"] = "zh_CN";

        std::ofstream file(filePath);
        if (!file.is_open()) return false;
        file << json.dump(4);
        return true;
    } catch (...) {
        return false;
    }
}

std::string ConfigLoader::getDefaultConfigPath() {
    return "plugins/fish-score/config.json";
}

PluginConfig ConfigLoader::parseJson(const nlohmann::json& json) {
    PluginConfig config;

    // API 配置
    if (json.contains("api")) {
        auto& api = json["api"];
        config.api.baseUrl    = api.value("baseUrl", "https://fish-score.com");
        config.api.apiVersion = api.value("apiVersion", "/api/v2");
        config.api.timeout    = api.value("timeout", 15000);
        config.api.apiKey     = api.value("apiKey", "");
    } else {
        config.api.baseUrl    = "https://fish-score.com";
        config.api.apiVersion = "/api/v2";
        config.api.timeout    = 15000;
        config.api.apiKey     = "";
    }

    // 缓存配置
    if (json.contains("cache")) {
        auto& cache = json["cache"];
        config.cache.enabled         = cache.value("enabled", true);
        config.cache.durationMinutes = cache.value("durationMinutes", 5);
    } else {
        config.cache.enabled         = true;
        config.cache.durationMinutes = 5;
    }

    // UI 配置
    if (json.contains("ui")) {
        auto& ui = json["ui"];
        config.ui.songsPerPage     = ui.value("songsPerPage", 10);
        config.ui.showGrade        = ui.value("showGrade", true);
        config.ui.showAccuracy     = ui.value("showAccuracy", true);
        config.ui.autoRefreshOnJoin = ui.value("autoRefreshOnJoin", false);
    } else {
        config.ui.songsPerPage      = 10;
        config.ui.showGrade         = true;
        config.ui.showAccuracy      = true;
        config.ui.autoRefreshOnJoin = false;
    }

    // 语言
    config.language = json.value("language", "zh_CN");

    return config;
}

PluginConfig ConfigLoader::getDefault() {
    PluginConfig config;
    config.api.baseUrl    = "https://fish-score.com";
    config.api.apiVersion = "/api/v2";
    config.api.timeout    = 15000;
    config.api.apiKey     = "";
    config.cache.enabled  = true;
    config.cache.durationMinutes = 5;
    config.ui.songsPerPage = 10;
    config.ui.showGrade    = true;
    config.ui.showAccuracy = true;
    config.ui.autoRefreshOnJoin = false;
    config.language = "zh_CN";
    return config;
}

} // namespace FishScore
