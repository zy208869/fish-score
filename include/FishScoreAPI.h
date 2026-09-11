// FishScoreAPI.h - 水鱼查分器 API 封装
#pragma once

#include "ScoreData.h"
#include "HttpCpr.h"
#include "Config.h"
#include <nlohmann/json.hpp>
#include <string>
#include <functional>
#include <memory>

namespace FishScore {

// 默认 API 配置常量（可被配置文件覆盖）
constexpr const char* FISHSCORE_API_BASE_DEFAULT = "https://fish-score.com/api";
constexpr const char* FISHSCORE_API_V2_DEFAULT   = "https://fish-score.com/api/v2";
constexpr int          DEFAULT_PAGE_SIZE          = 10;

class FishScoreAPI {
public:
    FishScoreAPI();
    explicit FishScoreAPI(const ApiConfig& config);
    ~FishScoreAPI() = default;

    // 从配置加载
    void loadConfig(const ApiConfig& config);
    // 设置 API 密钥（如果需要）
    void setApiKey(const std::string& key);
    // 设置超时
    void setTimeout(int ms);
    // 获取当前 API 基础地址
    std::string getApiBaseUrl() const;
    std::string getApiV2Url() const;

    // ============================================================
    // 异步查询接口（回调方式，适合在游戏主线程中异步调用）
    // ============================================================

    // 查询玩家基本信息
    using PlayerInfoCallback = std::function<void(bool success, const PlayerInfo& info, const std::string& error)>;
    void fetchPlayerInfo(uint64_t userId, PlayerInfoCallback callback);

    // 查询玩家分数列表（分页）
    // type: "best" 最佳成绩 / "recent" 最近成绩 / "all" 全部
    using ScoreListCallback = std::function<void(bool success, const PlayerScoreListResponse& resp, const std::string& error)>;
    void fetchPlayerScores(uint64_t userId, const std::string& type, int page, int pageSize, ScoreListCallback callback);

    // 查询单曲分数
    void fetchSingleScore(uint64_t userId, uint64_t songId, std::function<void(bool, const SongScore&, const std::string&)> callback);

    // 搜索歌曲
    using SongSearchCallback = std::function<void(bool success, const std::vector<SongScore>& results, const std::string& error)>;
    void searchSong(const std::string& keyword, SongSearchCallback callback);

    // ============================================================
    // 同步查询接口（仅用于离线/测试，会阻塞线程）
    // ============================================================
    std::optional<PlayerInfo> fetchPlayerInfoSync(uint64_t userId, std::string& error);
    std::optional<PlayerScoreListResponse> fetchPlayerScoresSync(uint64_t userId, const std::string& type, int page, int pageSize, std::string& error);

private:
    // JSON 反序列化辅助
    static bool parsePlayerInfo(const nlohmann::json& json, PlayerInfo& out);
    static bool parseSongScore(const nlohmann::json& json, SongScore& out);
    static bool parseScoreListResponse(const nlohmann::json& json, PlayerScoreListResponse& out);

    // 发起 HTTP GET 请求并解析 JSON
    std::optional<nlohmann::json> httpGetJson(const std::string& path, const cpr::Parameters& params, std::string& error);

private:
    std::unique_ptr<HttpCpr> mHttp;
    std::string mApiKey;
    std::string mApiBaseUrl;     // 从配置读取的 API 基础地址
    std::string mApiV2Url;       // 从配置读取的 V2 API 地址
};

} // namespace FishScore
