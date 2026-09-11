// ScoreListForm.cpp - 分数列表表单实现（分页显示）
#include "ScoreListForm.h"

#include <ll/api/form/SimpleForm.h>
#include <ll/api/form/CustomForm.h>
#include <sstream>
#include <iomanip>

namespace FishScore {

ScoreListForm::ScoreListForm()
    : mCurrentPage(1), mHasData(false), mPageSize(DEFAULT_PAGE_SIZE) {
    mPaginator.setPageSize(mPageSize);
}

ScoreListForm::ScoreListForm(int pageSize)
    : mCurrentPage(1), mHasData(false), mPageSize(pageSize > 0 ? pageSize : DEFAULT_PAGE_SIZE) {
    mPaginator.setPageSize(mPageSize);
}

void ScoreListForm::setPageSize(int size) {
    mPageSize = size > 0 ? size : DEFAULT_PAGE_SIZE;
    mPaginator.setPageSize(mPageSize);
}

void ScoreListForm::setData(PlayerScoreListResponse response) {
    mResponse = std::move(response);
    mHasData = !mResponse.records.empty();
    mPaginator.setTotalSongs(static_cast<int>(mResponse.records.size()));
    mPaginator.setPageSize(mPageSize);
    mPaginator.setCurrentPage(1);
    mCurrentPage = 1;
}

void ScoreListForm::setCurrentPage(int page) {
    mCurrentPage = page;
}

void ScoreListForm::setCallbacks(const ScoreListCallbacks& callbacks) {
    mCallbacks = callbacks;
}

ll::form::SimpleForm ScoreListForm::buildPageForm() {
    using namespace ll::form;

    // 玩家信息标题
    std::string playerName = mResponse.playerInfo.nickname.empty()
        ? "UID: " + std::to_string(mResponse.playerInfo.userId)
        : mResponse.playerInfo.nickname;

    std::ostringstream title;
    title << "§l§b" << playerName << " §r§7| §f" << mCurrentPage << "/" << getTotalPages() << "页";

    // 构建头部信息
    std::ostringstream content;
    content << "§fRating: §e" << std::fixed << std::setprecision(2) << mResponse.playerInfo.rating;
    content << "  §f排名: §b#" << mResponse.playerInfo.rank;
    content << "  §f已打: §a" << mResponse.playerInfo.totalSongs << "首\n";
    content << "§7─────────────────────────────────────\n\n";

    // 获取当前页的歌曲
    Page page = mPaginator.getPage(mResponse.records, mCurrentPage);

    int globalIndex = (mCurrentPage - 1) * mPageSize;

    SimpleForm form(title.str(), content.str());

    // 添加当前页的每一首歌作为按钮（每首歌一个按钮）
    for (size_t i = 0; i < page.songs.size(); ++i) {
        const auto& song = page.songs[i];
        int idx = globalIndex + static_cast<int>(i) + 1;

        std::ostringstream btnTitle;
        btnTitle << "§f§l#" << std::setw(2) << std::setfill('0') << idx << " §r§f" << song.songName << "\n";
        btnTitle << "§7  " << Paginator::formatDifficulty(song.difficulty, song.level);
        btnTitle << "  §b" << Paginator::formatScore(song.score);
        btnTitle << "  " << Paginator::formatGrade(song.grade, song.fullCombo, song.allPerfect);

        uint64_t songId = song.songId;
        form.appendButton(btnTitle.str(), [this, songId](Player& player) {
            if (mCallbacks.onSongSelected) mCallbacks.onSongSelected(songId);
        });
    }

    form.appendLabel("§7─────────────────────────────────────");

    // 控制按钮行
    if (mCurrentPage > 1) {
        form.appendButton("§e§l◀ 上一页", [this](Player& player) {
            mCurrentPage--;
            if (mCallbacks.onPageChanged) mCallbacks.onPageChanged(mCurrentPage);
        });
    }

    if (mCurrentPage < getTotalPages()) {
        form.appendButton("§a§l下一页 ▶", [this](Player& player) {
            mCurrentPage++;
            if (mCallbacks.onPageChanged) mCallbacks.onPageChanged(mCurrentPage);
        });
    }

    form.appendButton("§b§l📄 跳页", [this](Player& player) {
        buildPageSelector().sendTo(player);
    });

    form.appendButton("§6§l🔄 刷新", [this](Player& player) {
        if (mCallbacks.onRefresh) mCallbacks.onRefresh();
    });

    form.appendButton("§c§l🔙 返回", [this](Player& player) {
        if (mCallbacks.onBack) mCallbacks.onBack();
    });

    return form;
}

ll::form::CustomForm ScoreListForm::buildPageSelector() {
    using namespace ll::form;

    CustomForm form("§l§b跳页§r");

    std::vector<std::string> pageOptions;
    for (int i = 1; i <= getTotalPages(); ++i) {
        pageOptions.push_back("第 " + std::to_string(i) + " 页");
    }

    form.appendDropdown("targetPage", "§f选择页码§r", pageOptions);

    form.setCallback([this](Player& player, CustomFormResult const& result, FormCancelReason reason) {
        if (reason == FormCancelReason::UserClosed) return;
        auto pageOpt = result.get<int>("targetPage");
        if (pageOpt) {
            mCurrentPage = *pageOpt + 1;
            if (mCallbacks.onPageChanged) mCallbacks.onPageChanged(mCurrentPage);
        }
    });

    return form;
}

ll::form::SimpleForm ScoreListForm::buildEmptyForm() {
    using namespace ll::form;

    SimpleForm form("§l§e暂无数据§r", "§7该玩家还没有任何分数记录。");

    form.appendButton("§c§l返回§r", [this](Player& player) {
        if (mCallbacks.onBack) mCallbacks.onBack();
    });

    return form;
}

ll::form::SimpleForm ScoreListForm::buildLoadingForm() {
    using namespace ll::form;
    return SimpleForm("§l§7加载中...", "§7正在获取分数数据，请稍候...");
}

ll::form::SimpleForm ScoreListForm::buildErrorForm(const std::string& error) {
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

void ScoreListForm::showPage(class Player& player, int page) {
    mCurrentPage = page;
    buildPageForm().sendTo(player);
}

void ScoreListForm::show(class Player& player) {
    if (!mHasData) {
        buildEmptyForm().sendTo(player);
        return;
    }
    buildPageForm().sendTo(player);
}

int ScoreListForm::getCurrentPage() const {
    return mCurrentPage;
}

int ScoreListForm::getTotalPages() const {
    return mPaginator.getTotalPages();
}

int ScoreListForm::getTotalSongs() const {
    return static_cast<int>(mResponse.records.size());
}

int ScoreListForm::getPageSize() const {
    return mPageSize;
}

std::string ScoreListForm::formatSongEntry(const SongScore& song, int index) const {
    std::ostringstream oss;
    oss << "§f" << index << ". " << song.songName << "\n";
    oss << "  §7" << Paginator::formatDifficulty(song.difficulty, song.level);
    oss << "  §b" << Paginator::formatScore(song.score);
    oss << "  " << Paginator::formatGrade(song.grade, song.fullCombo, song.allPerfect);
    return oss.str();
}

} // namespace FishScore
