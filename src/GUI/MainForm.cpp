// MainForm.cpp - 主菜单表单实现
#include "MainForm.h"

#include <ll/api/form/SimpleForm.h>

namespace FishScore {

MainForm::MainForm() = default;

void MainForm::setCallbacks(const MainMenuCallbacks& callbacks) {
    mCallbacks = callbacks;
}

ll::form::SimpleForm MainForm::build() {
    using namespace ll::form;

    SimpleForm form("§l§b水鱼查分器 §r§7v1.0", "§f请选择操作：");

    form.appendButton("§a§l🔍 查询玩家§r\n§7输入UID查询玩家信息", [this](Player& player) {
        if (mCallbacks.onQueryPlayer) mCallbacks.onQueryPlayer();
    });

    form.appendButton("§b§l🎵 搜索歌曲§r\n§7搜索歌曲ID或名称", [this](Player& player) {
        if (mCallbacks.onSearchSong) mCallbacks.onSearchSong();
    });

    form.appendButton("§e§l📊 我的分数§r\n§7查看我的最佳成绩", [this](Player& player) {
        if (mCallbacks.onMyScores) mCallbacks.onMyScores();
    });

    form.appendButton("§6§l⚙ 设置§r\n§7API设置和偏好", [this](Player& player) {
        if (mCallbacks.onSettings) mCallbacks.onSettings();
    });

    form.appendButton("§d§lℹ 关于§r\n§7插件信息和帮助", [this](Player& player) {
        if (mCallbacks.onAbout) mCallbacks.onAbout();
    });

    return form;
}

void MainForm::show(class Player& player) {
    auto form = build();
    form.sendTo(player);
}

} // namespace FishScore
