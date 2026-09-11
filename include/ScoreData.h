// ScoreData.h - 水鱼查分器分数数据结构
#pragma once

#include <string>
#include <cstdint>

namespace FishScore {

// 单曲分数记录
struct SongScore {
    uint64_t songId;            // 歌曲 ID
    std::string songName;       // 歌曲名称
    std::string artist;         // 艺术家/作曲
    std::string coverUrl;       // 封面 URL

    // 难度信息
    uint8_t difficulty;         // 难度等级 (1-5 对应 EASY/NORMAL/HARD/EXPERT/MASTER)
    std::string difficultyName; // 难度名称
    float level;                // 定数 (如 13.5, 14.2)

    // 分数信息
    uint32_t score;             // 分数 (0-1000000)
    uint32_t maxCombo;          // 最大连击数
    uint32_t perfectCount;      // Perfect 数
    uint32_t greatCount;        // Great 数
    uint32_t goodCount;         // Good 数
    uint32_t badCount;          // Bad 数
    uint32_t missCount;         // Miss 数

    // 评价
    std::string grade;          // 评级 (S+, S, A, B, C, D)
    bool fullCombo;             // 是否 FC
    bool allPerfect;            // 是否 AP

    // 时间戳
    int64_t playedAt;           // 游玩时间戳 (毫秒)
};

// 玩家基本信息
struct PlayerInfo {
    uint64_t userId;             // 用户 ID
    std::string nickname;       // 昵称
    int32_t rank;               // 排名
    float rating;               // 总 Rating
    float bestRating;           // Bxx 最佳 Rating
    float recentRating;         // 近期 Rating
    uint32_t totalSongs;        // 已打歌曲总数
    uint32_t totalFullCombo;    // FC 总数
    uint32_t totalAllPerfect;   // AP 总数
};

// API 响应：单曲查询
struct SingleScoreResponse {
    int32_t code;               // 状态码 (0=成功)
    std::string message;        // 提示信息
    SongScore data;             // 分数数据
};

// API 响应：玩家分数列表
struct PlayerScoreListResponse {
    int32_t code;
    std::string message;
    PlayerInfo playerInfo;      // 玩家信息
    std::vector<SongScore> records; // 分数记录
    uint32_t total;             // 总记录数
    uint32_t page;              // 当前页
    uint32_t pageSize;          // 每页条数
    uint32_t totalPages;        // 总页数
};

} // namespace FishScore
