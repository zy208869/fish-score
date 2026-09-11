// Paginator.h - 分页逻辑封装
#pragma once

#include "ScoreData.h"
#include <vector>
#include <cstdint>
#include <string>
#include <sstream>
#include <iomanip>

namespace FishScore {

// 每页显示的歌曲数（全局常量）
constexpr int SONGS_PER_PAGE = 10;

// 分页数据
struct Page {
    std::vector<SongScore> songs;       // 当前页的歌曲
    int currentPage;                     // 当前页码 (1-based)
    int totalPages;                      // 总页数
    int totalSongs;                      // 总歌曲数
    int pageSize;                        // 每页大小
    bool hasNext;                        // 是否有下一页
    bool hasPrev;                        // 是否有上一页
};

// 分页管理器
class Paginator {
public:
    Paginator() : mTotalSongs(0), mCurrentPage(1), mPageSize(SONGS_PER_PAGE) {}

    // 设置总歌曲数
    void setTotalSongs(int total);
    // 设置当前页
    void setCurrentPage(int page);
    // 设置每页大小
    void setPageSize(int size);

    // 计算总页数
    int calculateTotalPages() const;

    // 获取当前页数据（从完整列表中切片）
    Page getPage(const std::vector<SongScore>& allSongs, int page) const;

    // 获取指定页的歌曲索引范围 [start, end)
    std::pair<int, int> getPageRange(int page) const;

    // 格式化分数显示
    static std::string formatScore(uint32_t score);
    // 格式化评级显示
    static std::string formatGrade(const std::string& grade, bool fc, bool ap);
    // 格式化难度显示
    static std::string formatDifficulty(uint8_t diff, float level);
    // 格式化时间戳
    static std::string formatTime(int64_t timestamp);

    // Getters
    int getTotalSongs() const { return mTotalSongs; }
    int getCurrentPage() const { return mCurrentPage; }
    int getPageSize() const { return mPageSize; }
    int getTotalPages() const { return calculateTotalPages(); }

private:
    int mTotalSongs;
    int mCurrentPage;
    int mPageSize;
};

// ============================================================
// 实现
// ============================================================

inline void Paginator::setTotalSongs(int total) {
    mTotalSongs = total;
}

inline void Paginator::setCurrentPage(int page) {
    mCurrentPage = page;
}

inline void Paginator::setPageSize(int size) {
    mPageSize = size > 0 ? size : SONGS_PER_PAGE;
}

inline int Paginator::calculateTotalPages() const {
    if (mTotalSongs <= 0) return 0;
    return (mTotalSongs + mPageSize - 1) / mPageSize;
}

inline Page Paginator::getPage(const std::vector<SongScore>& allSongs, int page) const {
    Page p;
    p.pageSize = mPageSize;
    p.totalSongs = mTotalSongs;
    p.totalPages = calculateTotalPages();
    p.currentPage = page;
    p.hasNext = page < p.totalPages;
    p.hasPrev = page > 1;

    auto [start, end] = getPageRange(page);
    if (start >= 0 && start < (int)allSongs.size()) {
        end = std::min(end, (int)allSongs.size());
        p.songs.assign(allSongs.begin() + start, allSongs.begin() + end);
    }
    return p;
}

inline std::pair<int, int> Paginator::getPageRange(int page) const {
    int start = (page - 1) * mPageSize;
    int end = start + mPageSize;
    return {start, end};
}

inline std::string Paginator::formatScore(uint32_t score) {
    // 格式: 1,000,000 或 999,999
    std::string s = std::to_string(score);
    std::string result;
    int count = 0;
    for (int i = (int)s.size() - 1; i >= 0; --i) {
        if (count > 0 && count % 3 == 0) result = "," + result;
        result = s[i] + result;
        count++;
    }
    return result;
}

inline std::string Paginator::formatGrade(const std::string& grade, bool fc, bool ap) {
    if (ap) return "§b§lAP §r§f" + grade;
    if (fc) return "§a§lFC §r§f" + grade;
    return grade;
}

inline std::string Paginator::formatDifficulty(uint8_t diff, float level) {
    static const char* diffNames[] = {"", "EASY", "NORMAL", "HARD", "EXPERT", "MASTER"};
    static const char* diffColors[] = {"", "§a", "§b", "§e", "§c", "§d"};

    const char* name = (diff <= 5) ? diffNames[diff] : "UNKNOWN";
    const char* color = (diff <= 5) ? diffColors[diff] : "§f";

    std::ostringstream oss;
    oss << color << name << " " << std::fixed << std::setprecision(1) << level;
    return oss.str();
}

inline std::string Paginator::formatTime(int64_t timestamp) {
    if (timestamp <= 0) return "未知";
    time_t t = timestamp / 1000;
    struct tm tm_buf;
    localtime_s(&tm_buf, &t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", &tm_buf);
    return std::string(buf);
}

} // namespace FishScore
