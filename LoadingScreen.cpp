//======================================================
//	LoadingScreen.cpp
//  ロード中に動画を再生するためのクラス
//======================================================
#include "LoadingScreen.h"
#include "direct3d.h"
#include "shader.h"
#include "sprite.h"
#include "fade.h"
#include "color.h"

//======================================================
// グローバル変数
//======================================================
static LoadingScreen* g_pLoadingScreen = nullptr;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

//======================================================
// LoadingScreenクラス
//======================================================

LoadingScreen::LoadingScreen() {}

LoadingScreen::~LoadingScreen()
{
    Finalize();
}

bool LoadingScreen::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wchar_t* videoPath)
{
    m_pContext = pContext;
    m_LoadComplete = false;
    m_VideoFinished = false;
    m_IsLoading = false;

    // 動画の初期化
    return m_Video.Initialize(pDevice, pContext, videoPath);
}

void LoadingScreen::Finalize()
{
    // ロードスレッドが動いていたら終了を待つ
    if (m_LoadThread.joinable())
    {
        m_LoadThread.join();
    }

    m_Video.Finalize();
    m_IsLoading = false;
}

void LoadingScreen::StartLoading(SCENE nextScene, std::function<void()> loadTask, bool waitForVideo)
{
    m_NextScene = nextScene;
    m_WaitForVideo = waitForVideo;
    m_LoadComplete = false;
    m_VideoFinished = false;
    m_IsLoading = true;

    // 動画を最初から再生
    m_Video.Reset();

    // 別スレッドでロード処理を実行
    m_LoadThread = std::thread([this, loadTask]()
    {
        // ロード処理を実（重い処理はここで行う）
        if (loadTask)
        {
            loadTask();
        }

        // ロード完了フラグを立てる
        m_LoadComplete = true;
    });
}

bool LoadingScreen::Update()
{
    if (!m_IsLoading) return false;

    // 動画フレームを更新
    bool stillPlaying = m_Video.Update();

    if (!m_WaitForVideo && m_LoadComplete)
    {
        m_IsLoading = false;
        return false;  // ロード完了
    }
    // 動画が終了したかチェック
    if (!stillPlaying || m_Video.IsFinished())
    {
        if (m_LoadComplete)
        {
            // ロード完了・動画終了で遷移
            m_VideoFinished = true;
        }
        else
        {
            // ロードがまだ終わっていない → 動画をループ再生
            m_Video.Reset();
        }
    }

    // 完了判定
    if (IsLoadingComplete())
    {
        m_IsLoading = false;
        return false;  // ロード完了
    }

    return true;  // まだロード中
}

void LoadingScreen::Draw()
{
    if (!m_IsLoading) return;

    // シェーダー設定
    Shader_Begin();
    Shader_SetColor(color::white);

    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    // UI用の直交投影行列を設定
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
        0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));

    // 動画テクスチャを描画
    ID3D11ShaderResourceView* pSRV = m_Video.GetShaderResourceView();
    if (pSRV && m_pContext)
    {
        m_pContext->PSSetShaderResources(0, 1, &pSRV);
        SetBlendState(BLENDSTATE_NONE);

        // 画面全体に動画を描画
        XMFLOAT2 pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
        XMFLOAT4 col = color::white;
        DrawSprite(pos, size, col);
    }
}

bool LoadingScreen::IsLoadingComplete() const
{
    if (m_WaitForVideo)
    {
        // 動画終了を待つ場合、ロード完了・動画終了
        return m_LoadComplete && m_VideoFinished;
    }
    else
    {
        // 動画終了を待たない場合、ロード完了のみでOK
        return m_LoadComplete.load();
    }
}

//======================================================
// グローバル関数実装
//======================================================

void LoadingScreen_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;
}

void LoadingScreen_Finalize()
{
    if (g_pLoadingScreen)
    {
        g_pLoadingScreen->Finalize();
        delete g_pLoadingScreen;
        g_pLoadingScreen = nullptr;
    }
}

bool LoadingScreen_Update()
{
    if (!g_pLoadingScreen) return false;

    bool stillLoading = g_pLoadingScreen->Update();

    if (!stillLoading)
    {
        // ロード完了で次のシーンへ遷移
        SCENE nextScene = g_pLoadingScreen->GetNextScene();

        // LoadingScreenを解放
        LoadingScreen_Finalize();

        // 次のシーンを設定
        SetScene(nextScene);

        // フェードイン
        SetFade(100, color::white, FADE_IN, nextScene);
    }

    return stillLoading;
}

void LoadingScreen_Draw()
{
    if (g_pLoadingScreen)
    {
        g_pLoadingScreen->Draw();
    }
}

bool IsLoading()
{
    return (g_pLoadingScreen != nullptr);
}

SCENE GetLoadingNextScene()
{
    if (g_pLoadingScreen)
    {
        return g_pLoadingScreen->GetNextScene();
    }
    return SCENE_NONE;
}

void SetSceneWithLoading(SCENE nextScene, const wchar_t* videoPath)
{
    // 既存のロード画面があれば解放
    LoadingScreen_Finalize();

    // 新しいロード画面を作成
    g_pLoadingScreen = new LoadingScreen();

    if (!g_pLoadingScreen->Initialize(g_pDevice, g_pContext, videoPath))
    {
        // 動画読み込み失敗の場合、通常のシーン遷移にフォールバック
        delete g_pLoadingScreen;
        g_pLoadingScreen = nullptr;
        SetScene(nextScene);
        return;
    }

    // ロード処理を定義（シーンごとに必要なリソースをロード）
    std::function<void()> loadTask = [nextScene]()
    {
        // ここにシーンごとのリソース読み込み処理を追加
        // DirectXリソースの作成はメインスレッドで行う必要がある場合あり
        // ファイル読み込みや計算処理をここで行う

        switch (nextScene)
        {
        case SCENE_GAME:
            // ゲームシーン用のリソース読み込み
            // LoadGameResources();
            // 重い処理をシミュレート（実際は削除）
            std::this_thread::sleep_for(std::chrono::seconds(5));
            break;

        case SCENE_TITLE:
            // タイトル用リソース読み込み
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            break;

        default:
            // その他のシーン
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            break;
        }
    };

    // ロード開始（動画が終わるまで待つ: true / ロード完了次第遷移: false）
    g_pLoadingScreen->StartLoading(nextScene, loadTask, true);
}