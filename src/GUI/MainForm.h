// GUI/MainForm.h - 主菜单表单
#pragma once

#include <ll/api/form/Form.h>
#include <ll/api/form/CustomForm.h>
#include <ll/api/form/SimpleForm.h>
#include <string>
#include <functional>

namespace FishScore {

// 主菜单按钮回调
struct MainMenuCallbacks {
    std::function<void()> onQueryPlayer;       // 查询玩家
    std::function<void()> onSearchSong;        // 搜索歌曲
    std::function<void()> onMyScores;          // 我的分数
    std::function<void()> onSettings;          // 设置
    std::function<void()> onAbout;             // 关于
};

class MainForm {
public:
    MainForm();

    // 构建表单
    ll::form::SimpleForm build();

    // 设置回调
    void setCallbacks(const MainMenuCallbacks& callbacks);

    // 显示表单给玩家
    void show(class Player& player);

private:
    MainMenuCallbacks mCallbacks;
};

} // namespace FishScore
