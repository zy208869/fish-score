// main.cpp - 水鱼查分器 LeviMina 插件入口
// 版本: 26.40.0
// 描述: 游戏内对接水鱼查分器，支持查询玩家分数、搜索歌曲、分页显示（每页10首歌）

#include <ll/api/plugin/PluginManager.h>
#include <ll/api/command/CommandRegistrar.h>
#include <ll/api/event/EventBus.h>
#include <ll/api/event/player/PlayerJoinEvent.h>
#include <ll/api/event/player/PlayerUseItemEvent.h>
#include <ll/api/utils/Hash.h>

#include "ScoreData.h"
#include "FishScoreAPI.h"
#include "HttpCpr.h"
#include "Paginator.h"
#include "Config.h"  // 配置加载

// GUI 表单
#include "GUI/MainForm.h"
#include "GUI/QueryPlayerForm.h"
#include "GUI/ScoreListForm.h"
#include "GUI/ScoreDetailForm.h"

#include <ll/api/base/Global.h>
#include <ll/api/form/Form.h>
#include <ll/api/command/CommandHandle.h>

#include <memory>
#include <string>
#include <thread>
#include <shared_mutex>

// ============================================================
// 全局状态
// ============================================================

namespace FishScore {

// 全局配置
static PluginConfig gConfig;

// 全局 API 实例
static std::unique_ptr<FishScoreAPI> gApi;

// 缓存的玩家分数响应 (避免重复请求)
static PlayerScoreListResponse gCachedResponse;
static uint64_t gCachedUserId = 0;
static std::string gCachedType;
static std::chrono::steady_clock::time_point gCacheTime;
static std::shared_mutex gCacheMutex;

// 表单实例
static std::unique_ptr<MainForm> gMainForm;
static std::unique_ptr<QueryPlayerForm> gQueryForm;
static std::unique_ptr<ScoreListForm> gScoreListForm;
static std::unique_ptr<ScoreDetailForm> gScoreDetailForm;

// 当前正在查看的歌曲ID
static uint64_t gCurrentSongId = 0;

// ============================================================
// 配置加载
// ============================================================

void loadPluginConfig() {
    // 尝试加载配置文件
    std::string configPath = ConfigLoader::getDefaultConfigPath();
    auto loaded = ConfigLoader::load(configPath);
    
    if (loaded) {
        gConfig = *loaded;
    } else {
        // 使用默认配置并保存
        gConfig = ConfigLoader::getDefault();
        ConfigLoader::saveDefault(configPath);
    }
}

// ============================================================
// 缓存工具
// ============================================================

bool isCacheValid(uint64_t userId, const std::string& type) {
    if (!gConfig.cache.enabled) return false;
    std::shared_lock lock(gCacheMutex);
    if (gCachedUserId != userId || gCachedType != type) return false;
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::minutes(gConfig.cache.durationMinutes);
    return (now - gCacheTime) < duration;
}

void updateCache(uint64_t userId, const std::string& type, const PlayerScoreListResponse& resp) {
    std::unique_lock lock(gCacheMutex);
    gCachedUserId = userId;
    gCachedType = type;
    gCachedResponse = resp;
    gCacheTime = std::chrono::steady_clock::now();
}

// ============================================================
// 主菜单导航
// ============================================================

void showMainMenu(Player& player) {
    MainMenuCallbacks callbacks;
    callbacks.onQueryPlayer = [&player]() {
        gQueryForm->showInput(player);
    };
    callbacks.onSearchSong = [&player]() {
        player.sendMessage("§e搜索功能开发中...");
    };
    callbacks.onMyScores = [&player]() {
        // 使用玩家自己的 UID（这里简化处理，实际需要从登录信息获取）
        uint64_t selfId = player.getUuid(); // 示例，实际需要真实 UID
        gApi->fetchPlayerScores(selfId, "best", 1, gConfig.ui.songsPerPage,
            [&player](bool success, const PlayerScoreListResponse& resp, const std::string& error) {
                if (!success) {
                    player.sendMessage("§c查询失败: " + error);
                    return;
                }
                gScoreListForm->setData(resp);
                gScoreListForm->show(player);
            });
    };
    callbacks.onSettings = [&player]() {
        player.sendMessage("§e设置功能开发中...");
    };
    callbacks.onAbout = [&player]() {
        player.sendMessage("§b§l水鱼查分器 §r§7v1.0\n"
            "§7作者: YourName\n"
            "§7每页显示 §a" + std::to_string(gConfig.ui.songsPerPage) + " §7首歌\n"
            "§7API地址: §b" + gApi->getApiBaseUrl() + "\n"
            "§7使用 §a/fishscore §7打开菜单");
    };

    gMainForm->setCallbacks(callbacks);
    gMainForm->show(player);
}

// ============================================================
// 分数列表导航
// ============================================================

void showScoreList(Player& player, uint64_t userId, const std::string& type, int page) {
    // 检查缓存
    if (!isCacheValid(userId, type)) {
        player.sendMessage("§7正在查询数据...");
        gApi->fetchPlayerScores(userId, type, 1, gConfig.ui.songsPerPage,
            [&player, userId, type, page](bool success, const PlayerScoreListResponse& resp, const std::string& error) {
                if (!success) {
                    player.sendMessage("§c查询失败: " + error);
                    return;
                }
                updateCache(userId, type, resp);
                gScoreListForm->setData(resp);

                ScoreListCallbacks callbacks;
                callbacks.onPageChanged = [&player, userId, type](int newPage) {
                    showScoreList(player, userId, type, newPage);
                };
                callbacks.onSongSelected = [&player, userId](uint64_t songId) {
                    showScoreDetail(player, userId, songId);
                };
                callbacks.onBack = [&player]() {
                    showMainMenu(player);
                };
                callbacks.onRefresh = [&player, userId, type]() {
                    showScoreList(player, userId, type, 1);
                };

                gScoreListForm->setCallbacks(callbacks);
                gScoreListForm->showPage(player, page);
            });
    } else {
        // 使用缓存数据
        {
            std::shared_lock lock(gCacheMutex);
            gScoreListForm->setData(gCachedResponse);
        }

        ScoreListCallbacks callbacks;
        callbacks.onPageChanged = [&player, userId, type](int newPage) {
            showScoreList(player, userId, type, newPage);
        };
        callbacks.onSongSelected = [&player, userId](uint64_t songId) {
            showScoreDetail(player, userId, songId);
        };
        callbacks.onBack = [&player]() {
            showMainMenu(player);
        };
        callbacks.onRefresh = [&player, userId, type]() {
            showScoreList(player, userId, type, 1);
        };

        gScoreListForm->setCallbacks(callbacks);
        gScoreListForm->showPage(player, page);
    }
}

// ============================================================
// 单曲详情
// ============================================================

void showScoreDetail(Player& player, uint64_t userId, uint64_t songId) {
    player.sendMessage("§7正在加载歌曲详情...");
    gApi->fetchSingleScore(userId, songId,
        [&player, userId, songId](bool success, const SongScore& score, const std::string& error) {
            if (!success) {
                player.sendMessage("§c加载失败: " + error);
                return;
            }
            gScoreDetailForm->setData(score);

            ScoreDetailCallbacks callbacks;
            callbacks.onBack = [&player, userId]() {
                // 返回列表
                showScoreList(player, userId, "best", 1);
            };
            callbacks.onRefresh = [&player, userId, songId]() {
                showScoreDetail(player, userId, songId);
            };

            gScoreDetailForm->setCallbacks(callbacks);
            gScoreDetailForm->show(player);
        });
}

// ============================================================
// 查询玩家
// ============================================================

void showQueryResult(Player& player, uint64_t userId, const std::string& queryType) {
    if (queryType == "single") {
        // 单曲查询 - 需要歌曲ID
        player.sendMessage("§e请输入歌曲ID：");
        // 这里简化处理，实际需要另一个输入表单
        return;
    }

    player.sendMessage("§7正在查询玩家 " + std::to_string(userId) + "...");
    showScoreList(player, userId, queryType, 1);
}

} // namespace FishScore

// ============================================================
// 插件入口
// ============================================================

using namespace FishScore;
using namespace ll::plugin;

// 插件加载
void PluginInit(PluginManager& manager, ll::plugin::Plugin& plugin) {
    // 1. 加载配置文件
    loadPluginConfig();

    // 2. 使用配置初始化 API
    gApi = std::make_unique<FishScoreAPI>(gConfig.api);
    gApi->setTimeout(gConfig.api.timeout);

    // 初始化表单（使用配置中的每页数量）
    gMainForm = std::make_unique<MainForm>();
    gQueryForm = std::make_unique<QueryPlayerForm>();
    gScoreListForm = std::make_unique<ScoreListForm>(gConfig.ui.songsPerPage);
    gScoreDetailForm = std::make_unique<ScoreDetailForm>();

    // 4. 设置查询回调
    gQueryForm->setResultCallback([](uint64_t userId, const std::string& queryType) {
        // 这里的 Player 引用需要从当前上下文中获取
        // 实际实现中需要将 Player 对象传递给回调
    });

    // 5. 注册命令
    auto& registrar = ll::command::CommandRegistrar::getInstance();

    // /fishscore - 打开主菜单
    registrar.registerCommand("fishscore", "打开水鱼查分器主菜单",
        [](CommandOrigin& origin, CommandOutput& output) {
            auto* player = origin.getPlayer();
            if (!player) {
                output.error("该命令只能由玩家执行");
                return;
            }
            showMainMenu(*player);
        });

    // /fishscore query <uid> - 快速查询玩家
    registrar.registerCommand("fishscore_query", "查询指定玩家的分数",
        [](CommandOrigin& origin, CommandOutput& output) {
            auto* player = origin.getPlayer();
            if (!player) {
                output.error("该命令只能由玩家执行");
                return;
            }
            uint64_t targetUid = 0; // 从参数获取
            if (targetUid == 0) {
                output.error("用法: /fishscore query <UID>");
                return;
            }
            showQueryResult(*player, targetUid, "best");
        });

    // /fishscore best <uid> - 查询最佳成绩
    registrar.registerCommand("fishscore_best", "查询玩家最佳成绩",
        [](CommandOrigin& origin, CommandOutput& output) {
            auto* player = origin.getPlayer();
            if (!player) {
                output.error("该命令只能由玩家执行");
                return;
            }
            uint64_t targetUid = 0;
            showQueryResult(*player, targetUid, "best");
        });

    // /fishscore recent <uid> - 查询最近成绩
    registrar.registerCommand("fishscore_recent", "查询玩家最近成绩",
        [](CommandOrigin& origin, CommandOutput& output) {
            auto* player = origin.getPlayer();
            if (!player) {
                output.error("该命令只能由玩家执行");
                return;
            }
            uint64_t targetUid = 0;
            showQueryResult(*player, targetUid, "recent");
        });

    // 6. 监听玩家加入事件（可选：自动提示）
    auto& eventBus = ll::event::EventBus::getInstance();
    eventBus.emplaceListener<ll::event::PlayerJoinEvent>([](ll::event::PlayerJoinEvent& event) {
        auto& player = event.player();
        if (gConfig.ui.autoRefreshOnJoin) {
            player.sendMessage("§b输入 §a/fishscore §b打开水鱼查分器");
        }
    });

    // 7. 注册插件信息
    plugin.setName("FishScore");
    plugin.setVersion("1.0.0");
    plugin.setAuthor("YourName");
    plugin.setDescription("水鱼查分器 - 游戏内查分插件");
    
    // 输出配置信息到控制台
    // 实际实现中可以使用 LeviMina 的日志系统
    // ll::log::info("FishScore 插件已加载，API地址: {}", gApi->getApiBaseUrl());
}

// 插件卸载
void PluginUnload() {
    gApi.reset();
    gMainForm.reset();
    gQueryForm.reset();
    gScoreListForm.reset();
    gScoreDetailForm.reset();
}

// ============================================================
// 导出符号
// ============================================================

extern "C" {
    __declspec(dllexport) void on_load(ll::plugin::PluginManager& manager, ll::plugin::Plugin& plugin) {
        PluginInit(manager, plugin);
    }

    __declspec(dllexport) void on_unload() {
        PluginUnload();
    }

    __declspec(dllexport) const char* get_version() {
        return "1.0.0";
    }

    __declspec(dllexport) const char* get_api_version() {
        return LEVILAMINA_API_VERSION; // 26.40.0
    }
}
