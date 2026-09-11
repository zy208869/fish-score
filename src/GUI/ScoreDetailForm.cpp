// ScoreDetailForm.cpp - 单曲分数详情表单实现
#include "ScoreDetailForm.h"
#include "Paginator.h"

#include <ll/api/form/SimpleForm.h>
#include <sstream>
#include <iomanip>

namespace FishScore {

ScoreDetailForm::ScoreDetailForm()
    : mHasData(false) {
}

void ScoreDetailForm::setData(const SongScore& score) {
    mScore = score;
    mHasData = true;
}

void ScoreDetailForm::setCallbacks(const ScoreDetailCallbacks& callbacks) {
    mCallbacks = callbacks;
}

ll::form::SimpleForm ScoreDetailForm::build() {
    using namespace ll::form;

    if (!mHasData) {
        return SimpleForm("§l§c无数据§r", "§7无法加载分数信息。");
    }

    std::string title = "§l§b" + mScore.songName + " §r§7| §f单曲详情";

    std::ostringstream content;
    content << "§7═══════════════════════════════════════\n\n";

    // 基本信息
    content << "§l§f歌曲信息\n";
    content << "§7  歌名: §f" << mScore.songName << "\n";
    if (!mScore.artist.empty()) {
        content << "§7  作曲: §f" << mScore.artist << "\n";
    }
    content << "§7  歌曲ID: §b" << mScore.songId << "\n\n";

    // 难度信息
    content << "§l§f难度信息\n";
    content << "  " << Paginator::formatDifficulty(mScore.difficulty, mScore.level) << "\n\n";

    // 分数信息
    content << "§l§f分数信息\n";
    content << "§7  分数: §b" << Paginator::formatScore(mScore.score) << "\n";
    content << "§7  评级: " << Paginator::formatGrade(mScore.grade, mScore.fullCombo, mScore.allPerfect) << "\n";

    if (mScore.fullCombo) {
        content << "§7  连击: §a§lFC §r§7(" << mScore.maxCombo << ")\n";
    } else {
        content << "§7  最大连击: §f" << mScore.maxCombo << "\n";
    }
    content << "\n";

    // 判定详情
    content << "§l§f判定详情\n";
    content << "§b  Perfect: §f" << mScore.perfectCount << "\n";
    content << "§a  Great:   §f" << mScore.greatCount << "\n";
    content << "§e  Good:    §f" << mScore.goodCount << "\n";
    content << "§c  Bad:     §f" << mScore.badCount << "\n";
    content << "§7  Miss:    §f" << mScore.missCount << "\n";
    content << "\n";

    // 总计判定
    uint32_t totalNotes = mScore.perfectCount + mScore.greatCount + mScore.goodCount + mScore.badCount + mScore.missCount;
    content << "§7  总物量: §f" << totalNotes << "\n\n";

    // 准确度
    if (totalNotes > 0) {
        double accuracy = 100.0 * (mScore.perfectCount + 0.7 * mScore.greatCount + 0.4 * mScore.goodCount) / totalNotes;
        content << "§7  准确率: §e" << std::fixed << std::setprecision(2) << accuracy << "%%\n\n";
    }

    // 其他信息
    content << "§l§f其他\n";
    if (mScore.allPerfect) {
        content << "§b  ✅ All Perfect!\n";
    }
    if (mScore.fullCombo) {
        content << "§a  ✅ Full Combo\n";
    }
    content << "§7  游玩时间: §f" << Paginator::formatTime(mScore.playedAt) << "\n";

    content << "\n§7═══════════════════════════════════════\n";

    SimpleForm form(title, content.str());

    form.appendButton("§6§l🔄 刷新§r\n§7重新加载", [this](Player& player) {
        if (mCallbacks.onRefresh) mCallbacks.onRefresh();
    });

    form.appendButton("§c§l🔙 返回§r\n§7返回列表", [this](Player& player) {
        if (mCallbacks.onBack) mCallbacks.onBack();
    });

    return form;
}

ll::form::SimpleForm ScoreDetailForm::buildError(const std::string& error) {
    using namespace ll::form;

    SimpleForm form("§l§c出错了§r", "§c" + error);

    form.appendButton("§e§l重试§r", [this](Player& player) {
        if (mCallbacks.onRefresh) mCallbacks.onRefresh();
    });

    form.appendButton("§c§l返回§r", [this](Player& player) {
        if (mCallbacks.onBack) mCallbacks.onBack();
    });

    return form;
}

void ScoreDetailForm::show(class Player& player) {
    build().sendTo(player);
}

} // namespace FishScore
