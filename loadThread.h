#pragma once
#include <functional>
#include <vector>
#include <atomic>

namespace Loader {
    // ロードを予約する
    void AddTask(std::function<void()> task);

    // 溜まった予約を別スレッドで一気に実行する
    void StartTaskLoad();

    // 全部のロードが終わったか確認する
    bool IsFinished();

    void Reset();
}
