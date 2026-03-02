#include "loadThread.h"
#include <thread>
#include <objbase.h>
#include <iostream>
#include <string>

namespace Loader {
    static std::vector<std::function<void()>> g_tasks;
    static std::atomic<bool> g_isFinished(false);
    static bool g_isStarted = false;

    void AddTask(std::function<void()> task) 
    {
        if (g_isStarted) return;
        g_tasks.push_back(task);
        // デバッグ: タスク追加を確認
        OutputDebugStringA(("タスク追加: 現在のタスク数 = " + std::to_string(g_tasks.size()) + "\n").c_str());
    }

    void StartTaskLoad() 
    {
        g_isStarted = true;
        
        // 開始時のタスク数を出力
        OutputDebugStringA(("ロード開始: タスク数 = " + std::to_string(g_tasks.size()) + "\n").c_str());
        
        if (g_tasks.empty()) 
        {
            OutputDebugStringA("警告: タスクが空です。即座に完了します。\n");
            g_isFinished = true;
            return;
        }
        
        std::thread([]() 
        {
            OutputDebugStringA("ロードスレッド開始\n");
            CoInitializeEx(NULL, COINIT_MULTITHREADED);

            int taskIndex = 0;
            for (auto& task : g_tasks) 
            {
                try 
                {
                    OutputDebugStringA(("タスク実行中: " + std::to_string(taskIndex) + "\n").c_str());
                    task();
                    OutputDebugStringA(("タスク完了: " + std::to_string(taskIndex) + "\n").c_str());
                    taskIndex++;
                }
                catch (const std::exception& e) 
                {
                    OutputDebugStringA(("タスクエラー: " + std::string(e.what()) + "\n").c_str());
                }
                catch (...) 
                {
                    OutputDebugStringA("タスクで不明なエラーが発生\n");
                }
            }

            CoUninitialize();
            g_isFinished = true;
            OutputDebugStringA("全タスク完了\n");
        }).detach();
    }

    bool IsFinished() { return g_isFinished; }
    void Reset() 
{
        if (!g_isFinished)
        {
            // ロードが終わっていないならリセットしない
            OutputDebugStringA("Loader::Reset(): ロード未完了;;リセットをスキップします\n");
            return;
        }

        g_tasks.clear();
        g_isFinished = false;
        g_isStarted = false;
        OutputDebugStringA("Loader::Reset(): リセット完了\n");
    }
}