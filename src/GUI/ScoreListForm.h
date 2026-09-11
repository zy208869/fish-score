// GUI/ScoreListForm.h - 分数列表表单（分页显示，每页10首歌）
#pragma once

#include "Paginator.h"
#include "FishScoreAPI.h"
#include "Config.h"
#include <ll/api/form/SimpleForm.h>
#include <ll/api/form/CustomForm.h>
#include <vector>
#include <memory>
#include <string>

namespace FishScore {

// 分数列表页的回调
struct ScoreListCallbacks {
    std::function<void(int page)> onPageChanged;
    std::function<void(uint64_t songId)> onSongSelected;
    std::function<void()> onBack;
    std::function<void()> onRefresh;
};

class ScoreListForm {
public:
    ScoreListForm();
    explicit ScoreListForm(int pageSize);

    // 设置每页显示的歌曲数（覆盖默认值）
    void setPageSize(int size);

    // 设置分页数据
    void setData(PlayerScoreListResponse response);

    // 设置当前页
    void setCurrentPage(int page);

    // 设置回调
    void setCallbacks(const ScoreListCallbacks& callbacks);

    // 构建当前页的表单
    ll::form::SimpleForm buildPageForm();

    // 构建翻页控件（下拉菜单）
    ll::form::CustomForm buildPageSelector();

    // 构建空状态表单
    ll::form::SimpleForm buildEmptyForm();

    // 构建加载表单
    ll::form::SimpleForm buildLoadingForm();

    // 构建错误表单
    ll::form::SimpleForm buildErrorForm(const std::string& error);

    // 显示当前页
    void showPage(class Player& player, int page);

    // 显示指定页
    void show(class Player& player);

    // 获取分页信息
    int getCurrentPage() const;
    int getTotalPages() const;
    int getTotalSongs() const;
    int getPageSize() const;

private:
    // 生成单行歌曲的文本
    std::string formatSongEntry(const SongScore& song, int index) const;

private:
    PlayerScoreListResponse mResponse;
    Paginator mPaginator;
    ScoreListCallbacks mCallbacks;
    int mCurrentPage;
    bool mHasData;
    int mPageSize;
};

} // namespace FishScore
