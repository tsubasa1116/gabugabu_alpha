//======================================================
//	LoadingScreen.h
//  ロード中に動画を再生するためのクラス
//======================================================
#pragma once

#include "VideoTexture.h"
#include "Manager.h"
#include <thread>
#include <atomic>
#include <functional>

class LoadingScreen
{
public:
    LoadingScreen();
    ~LoadingScreen();

    void StartLoading(SCENE nextScene, std::function<void()> loadTask, bool waitForVideo = false);

    bool Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wchar_t* videoPath);
    void Finalize();
    bool Update();
    void Draw();

    bool IsLoadingComplete() const; // ロード完了判定
    bool IsFadingOut() const { return m_IsFadingOut; }  // フェードアウト中か

    SCENE GetNextScene() const { return m_NextScene; }

private:
    VideoTexture m_Video;                       // ロード中動画
    std::thread m_LoadThread;                   // ロード用スレッド
    std::atomic<bool> m_LoadComplete{ false };  // ロード処理完了フラグ（スレッドセーフ）
    bool m_VideoFinished = false;               // 動画再生完了フラグ
    bool m_WaitForVideo = false;                // 動画終了を待つかどうか
    bool m_IsLoading = false;                   // ロード中フラグ
    bool m_IsFadingOut = false;                 // フェードアウト中フラグ
    bool m_FadeOutStarted = false;              // フェード開始済みフラグ（追加）
    int m_FadeOutCounter = 0;                   // フェードアウトカウンター
    int m_FadeOutDuration = 60;                 // フェードアウト所要フレーム数
    SCENE m_NextScene = SCENE_NONE;             // 遷移先シーン

    ID3D11DeviceContext* m_pContext = nullptr;  // 描画用コンテキスト
    ID3D11Device* m_pDevice = nullptr;          // デバイス
    ID3D11ShaderResourceView* m_pFadeTexture = nullptr;  // フェード用白テクスチャ
    bool m_HasVideo;

    float GetLoopSkipTime(const wchar_t* videoPath);
};

// グローバル関数
void LoadingScreen_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
void LoadingScreen_Finalize();
bool LoadingScreen_Update();  // 戻り値: true=ロード中 / false=完了
void LoadingScreen_Draw();

// ロード画面付きでシーン遷移を開始
void SetSceneWithLoading(SCENE nextScene, const wchar_t* videoPath = L"asset\\movie\\gameLoad.mp4");
void SetSceneSimple(SCENE nextScene);

// ロード中かどうか
bool IsLoading();

// ロード完了後の遷移先シーンを取得
SCENE GetLoadingNextScene();
