// QueryPlayerForm.cpp - 查询玩家表单实现
#include "QueryPlayerForm.h"

#include <ll/api/form/CustomForm.h>
#include <ll/api/form/SimpleForm.h>

namespace FishScore {

QueryPlayerForm::QueryPlayerForm() = default;

void QueryPlayerForm::setResultCallback(ResultCallback callback) {
    mCallback = std::move(callback);
}

ll::form::CustomForm QueryPlayerForm::buildInputForm() {
    using namespace ll::form;

    CustomForm form("§l§b查询玩家§r");

    form.appendInput("userId", "§f玩家 UID §7(数字ID)", "请输入UID");
    form.appendStepSlider("queryType", "§f查询类型§r",
        {"最佳成绩 (B30)", "最近成绩", "全部记录", "单曲查询"});
    form.appendInput("songId", "§f歌曲ID §7(单曲查询时填写)", "可选");

    form.setCallback([this](Player& player, CustomFormResult const& result, FormCancelReason reason) {
        if (reason == FormCancelReason::UserClosed) return;

        auto userIdOpt = result.get<uint64_t>("userId");
        auto queryTypeOpt = result.get<int>("queryType");
        auto songIdOpt = result.get<uint64_t>("songId");

        if (!userIdOpt) {
            player.sendMessage("§c错误：请输入有效的玩家UID！");
            return;
        }

        uint64_t userId = *userIdOpt;
        int queryType = queryTypeOpt ? *queryTypeOpt : 0;
        uint64_t songId = songIdOpt ? *songIdOpt : 0;

        static const char* typeNames[] = {"best", "recent", "all", "single"};
        std::string type = (queryType < 4) ? typeNames[queryType] : "best";

        if (mCallback) mCallback(userId, type);
    });

    return form;
}

ll::form::SimpleForm QueryPlayerForm::buildConfirmForm(uint64_t userId, const std::string& nickname) {
    using namespace ll::form;

    std::string content = "§f玩家: §a" + nickname + "\n§fUID: §b" + std::to_string(userId);

    SimpleForm form("§l§b确认查询§r", content);

    form.appendButton("§a§l确认查询§r\n§7点击查询该玩家", [this, userId](Player& player) {
        if (mCallback) mCallback(userId, "best");
    });

    form.appendButton("§c§l返回§r\n§7返回主菜单", [](Player& player) {
        // 返回主菜单逻辑由外部处理
    });

    return form;
}

void QueryPlayerForm::showInput(class Player& player) {
    auto form = buildInputForm();
    form.sendTo(player);
}

void QueryPlayerForm::showConfirm(class Player& player, uint64_t userId, const std::string& nickname) {
    auto form = buildConfirmForm(userId, nickname);
    form.sendTo(player);
}

} // namespace FishScore
