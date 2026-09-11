// GUI/QueryPlayerForm.h - 查询玩家表单
#pragma once

#include <ll/api/form/CustomForm.h>
#include <ll/api/form/SimpleForm.h>
#include <string>
#include <functional>

namespace FishScore {

class QueryPlayerForm {
public:
    using ResultCallback = std::function<void(uint64_t userId, const std::string& queryType)>;

    QueryPlayerForm();

    // 构建输入表单
    ll::form::CustomForm buildInputForm();

    // 构建确认表单（显示玩家信息）
    ll::form::SimpleForm buildConfirmForm(uint64_t userId, const std::string& nickname);

    // 设置结果回调
    void setResultCallback(ResultCallback callback);

    // 显示输入表单
    void showInput(class Player& player);

    // 显示确认表单
    void showConfirm(class Player& player, uint64_t userId, const std::string& nickname);

private:
    ResultCallback mCallback;
};

} // namespace FishScore
