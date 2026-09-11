// GUI/ScoreDetailForm.h - 单曲分数详情表单
#pragma once

#include "ScoreData.h"
#include <ll/api/form/SimpleForm.h>
#include <functional>

namespace FishScore {

// 单曲详情回调
struct ScoreDetailCallbacks {
    std::function<void()> onBack;     // 返回
    std::function<void()> onRefresh;  // 刷新
};

class ScoreDetailForm {
public:
    ScoreDetailForm();

    // 设置分数数据
    void setData(const SongScore& score);

    // 设置回调
    void setCallbacks(const ScoreDetailCallbacks& callbacks);

    // 构建详情表单
    ll::form::SimpleForm build();

    // 构建错误表单
    ll::form::SimpleForm buildError(const std::string& error);

    // 显示表单
    void show(class Player& player);

private:
    SongScore mScore;
    ScoreDetailCallbacks mCallbacks;
    bool mHasData;
};

} // namespace FishScore
