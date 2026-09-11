// FishScoreAPI.cpp
#include "FishScoreAPI.h"
#include <nlohmann/json.hpp>
#include <thread>

namespace FishScore {

// ============================================================
// 构造函数
// ============================================================

FishScoreAPI::FishScoreAPI()
    : mHttp(std::make_unique<HttpCpr>())
    , mApiBaseUrl(FISHSCORE_API_BASE_DEFAULT)
    , mApiV2Url(FISHSCORE_API_V2_DEFAULT) {
}

FishScoreAPI::FishScoreAPI(const ApiConfig& config)
    : mHttp(std::make_unique<HttpCpr>())
    , mApiBaseUrl(config.baseUrl)
    , mApiV2Url(config.baseUrl + config.apiVersion) {
    if (!config.apiKey.empty()) {
        setApiKey(config.apiKey);
    }
    mHttp->setTimeout(config.timeout);
}

void FishScoreAPI::loadConfig(const ApiConfig& config) {
    mApiBaseUrl = config.baseUrl;
    mApiV2Url   = config.baseUrl + config.apiVersion;
    if (!config.apiKey.empty()) {
        setApiKey(config.apiKey);
    }
    mHttp->setTimeout(config.timeout);
}

void FishScoreAPI::setApiKey(const std::string& key) {
    mApiKey = key;
    mHttp->setDefaultHeader("X-API-Key", key);
}

void FishScoreAPI::setTimeout(int ms) {
    mHttp->setTimeout(ms);
}

std::string FishScoreAPI::getApiBaseUrl() const {
    return mApiBaseUrl;
}

std::string FishScoreAPI::getApiV2Url() const {
    return mApiV2Url;
}

// ============================================================
// JSON 反序列化
// ============================================================

bool FishScoreAPI::parsePlayerInfo(const nlohmann::json& json, PlayerInfo& out) {
    try {
        if (!json.contains("data") || !json["data"].is_object()) return false;

        auto& d = json["data"];
        out.userId      = d.value("userId", uint64_t{0});
        out.nickname    = d.value("nickname", "");
        out.rank        = d.value("rank", int32_t{0});
        out.rating      = d.value("rating", float{0.0f});
        out.bestRating  = d.value("bestRating", float{0.0f});
        out.recentRating= d.value("recentRating", float{0.0f});
        out.totalSongs  = d.value("totalSongs", uint32_t{0});
        out.totalFullCombo    = d.value("totalFullCombo", uint32_t{0});
        out.totalAllPerfect   = d.value("totalAllPerfect", uint32_t{0});
        return true;
    } catch (...) {
        return false;
    }
}

bool FishScoreAPI::parseSongScore(const nlohmann::json& json, SongScore& out) {
    try {
        if (!json.is_object()) return false;

        out.songId       = json.value("songId", uint64_t{0});
        out.songName     = json.value("songName", "");
        out.artist       = json.value("artist", "");
        out.coverUrl     = json.value("coverUrl", "");
        out.difficulty   = json.value("difficulty", uint8_t{0});
        out.difficultyName = json.value("difficultyName", "");
        out.level        = json.value("level", float{0.0f});
        out.score        = json.value("score", uint32_t{0});
        out.maxCombo     = json.value("maxCombo", uint32_t{0});
        out.perfectCount = json.value("perfectCount", uint32_t{0});
        out.greatCount   = json.value("greatCount", uint32_t{0});
        out.goodCount    = json.value("goodCount", uint32_t{0});
        out.badCount     = json.value("badCount", uint32_t{0});
        out.missCount    = json.value("missCount", uint32_t{0});
        out.grade        = json.value("grade", "");
        out.fullCombo    = json.value("fullCombo", false);
        out.allPerfect   = json.value("allPerfect", false);
        out.playedAt     = json.value("playedAt", int64_t{0});
        return true;
    } catch (...) {
        return false;
    }
}

bool FishScoreAPI::parseScoreListResponse(const nlohmann::json& json, PlayerScoreListResponse& out) {
    try {
        out.code    = json.value("code", int32_t{-1});
        out.message = json.value("message", "");

        if (out.code != 0) return false;

        if (json.contains("data") && json["data"].contains("playerInfo")) {
            parsePlayerInfo(json["data"]["playerInfo"], out.playerInfo);
        }

        out.records.clear();
        if (json.contains("data") && json["data"].contains("records") && json["data"]["records"].is_array()) {
            for (auto& r : json["data"]["records"]) {
                SongScore s;
                if (parseSongScore(r, s)) {
                    out.records.push_back(std::move(s));
                }
            }
        }

        if (json.contains("data") && json["data"].contains("pagination")) {
            auto& p = json["data"]["pagination"];
            out.total      = p.value("total", uint32_t{0});
            out.page       = p.value("page", uint32_t{1});
            out.pageSize   = p.value("pageSize", uint32_t{DEFAULT_PAGE_SIZE});
            out.totalPages = p.value("totalPages", uint32_t{0});
        }

        return true;
    } catch (...) {
        return false;
    }
}

// ============================================================
// HTTP GET + JSON 解析
// ============================================================

std::optional<nlohmann::json> FishScoreAPI::httpGetJson(
    const std::string& path,
    const cpr::Parameters& params,
    std::string& error) {
    try {
        auto resp = mHttp->get(path, params);
        if (resp.status_code != 200) {
            error = "HTTP " + std::to_string(resp.status_code) + ": " + resp.text;
            return std::nullopt;
        }
        return nlohmann::json::parse(resp.text);
    } catch (const std::exception& e) {
        error = std::string("Parse error: ") + e.what();
        return std::nullopt;
    } catch (...) {
        error = "Unknown error";
        return std::nullopt;
    }
}

// ============================================================
// 异步查询接口（使用 mApiV2Url 而非硬编码宏）
// ============================================================

void FishScoreAPI::fetchPlayerInfo(uint64_t userId, PlayerInfoCallback callback) {
    std::thread([this, userId, cb = std::move(callback)]() {
        std::string err;
        std::string url = mApiV2Url + "/player/" + std::to_string(userId);
        auto json = httpGetJson(url, {}, err);

        if (!json) {
            PlayerInfo empty{};
            cb(false, empty, err);
            return;
        }

        PlayerInfo info{};
        if (!parsePlayerInfo(*json, info)) {
            cb(false, info, "Failed to parse player info");
            return;
        }
        cb(true, info, "");
    }).detach();
}

void FishScoreAPI::fetchPlayerScores(uint64_t userId, const std::string& type, int page, int pageSize, ScoreListCallback callback) {
    std::thread([this, userId, type, page, pageSize, cb = std::move(callback)]() {
        std::string err;
        cpr::Parameters params{
            {"type", type},
            {"page", std::to_string(page)},
            {"pageSize", std::to_string(pageSize)}
        };
        std::string url = mApiV2Url + "/player/" + std::to_string(userId) + "/scores";
        auto json = httpGetJson(url, params, err);

        if (!json) {
            PlayerScoreListResponse empty{};
            cb(false, empty, err);
            return;
        }

        PlayerScoreListResponse resp;
        if (!parseScoreListResponse(*json, resp)) {
            cb(false, resp, "Failed to parse score list");
            return;
        }
        cb(true, resp, "");
    }).detach();
}

void FishScoreAPI::fetchSingleScore(uint64_t userId, uint64_t songId,
    std::function<void(bool, const SongScore&, const std::string&)> callback) {
    std::thread([this, userId, songId, cb = std::move(callback)]() {
        std::string err;
        std::string url = mApiV2Url + "/player/" + std::to_string(userId) + "/score/" + std::to_string(songId);
        auto json = httpGetJson(url, {}, err);

        if (!json) {
            SongScore empty{};
            cb(false, empty, err);
            return;
        }

        SongScore s{};
        if (!parseSongScore(json->contains("data") ? (*json)["data"] : *json, s)) {
            cb(false, s, "Failed to parse single score");
            return;
        }
        cb(true, s, "");
    }).detach();
}

void FishScoreAPI::searchSong(const std::string& keyword, SongSearchCallback callback) {
    std::thread([this, keyword, cb = std::move(callback)]() {
        std::string err;
        cpr::Parameters params{{"keyword", keyword}};
        std::string url = mApiV2Url + "/song/search";
        auto json = httpGetJson(url, params, err);

        if (!json) {
            cb(false, {}, err);
            return;
        }

        std::vector<SongScore> results;
        if (json->contains("data") && (*json)["data"].contains("items") && (*json)["data"]["items"].is_array()) {
            for (auto& item : (*json)["data"]["items"]) {
                SongScore s;
                if (parseSongScore(item, s)) {
                    results.push_back(std::move(s));
                }
            }
        }
        cb(true, results, "");
    }).detach();
}

// ============================================================
// 同步查询接口
// ============================================================

std::optional<PlayerInfo> FishScoreAPI::fetchPlayerInfoSync(uint64_t userId, std::string& error) {
    std::string url = mApiV2Url + "/player/" + std::to_string(userId);
    auto json = httpGetJson(url, {}, error);
    if (!json) return std::nullopt;

    PlayerInfo info{};
    if (!parsePlayerInfo(*json, info)) {
        error = "Parse error";
        return std::nullopt;
    }
    return info;
}

std::optional<PlayerScoreListResponse> FishScoreAPI::fetchPlayerScoresSync(
    uint64_t userId, const std::string& type, int page, int pageSize, std::string& error) {
    cpr::Parameters params{
        {"type", type},
        {"page", std::to_string(page)},
        {"pageSize", std::to_string(pageSize)}
    };
    std::string url = mApiV2Url + "/player/" + std::to_string(userId) + "/scores";
    auto json = httpGetJson(url, params, error);
    if (!json) return std::nullopt;

    PlayerScoreListResponse resp;
    if (!parseScoreListResponse(*json, resp)) {
        error = "Parse error";
        return std::nullopt;
    }
    return resp;
}

} // namespace FishScore
