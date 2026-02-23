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

using namespace DirectX;

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
    m_pDevice = pDevice;
    m_pContext = pContext;
    m_LoadComplete = false;
    m_VideoFinished = false;
    m_IsLoading = false;
    m_IsFadingOut = false;
    m_FadeOutStarted = false;
    m_FadeOutCounter = 0;
    m_FadeOutDuration = 30;

    // フェード用テクスチャ読み込み
    TexMetadata metadata;
    ScratchImage image;
    LoadFromWICFile(L"asset\\texture\\fade.bmp", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &m_pFadeTexture);
    assert(&m_pFadeTexture);

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
    
    if (m_pFadeTexture)
    {
        m_pFadeTexture->Release();
        m_pFadeTexture = nullptr;
    }
    
    m_IsLoading = false;
    m_IsFadingOut = false;
}

void LoadingScreen::StartLoading(SCENE nextScene, std::function<void()> loadTask, bool waitForVideo)
{
    m_NextScene = nextScene;
    m_WaitForVideo = waitForVideo;
    m_LoadComplete = false;
    m_VideoFinished = false;
    m_IsLoading = true;
    m_IsFadingOut = false;
    m_FadeOutStarted = false;
    m_FadeOutCounter = 0;

    // 動画を最初から再生
    m_Video.Reset();

    // 別スレッドでロード処理を実行
    m_LoadThread = std::thread([this, loadTask]()
    {
        // ロード処理を実行
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

    // 動画フレームを更新（フェードアウト中もループするようにする）
    bool stillPlaying = m_Video.Update();

    // 動画が終了したらループ再生（ロード中・フェードアウト中どちらも）
    if (!stillPlaying || m_Video.IsFinished())
    {
        m_Video.Reset();
    }

    // フェードアウト中の場合
    if (m_IsFadingOut)
    {
        m_FadeOutCounter++;
        
        // フェードアウト完了
        if (m_FadeOutCounter > m_FadeOutDuration)
        {
            m_IsLoading = false;
            return false;  // ロード画面終了
        }
        return true;  // フェードアウト中は継続
    }

    // ロード完了チェック（フェードは一度だけ）
    if (m_LoadComplete && !m_FadeOutStarted)
    {
        // フェードアウト（fade.cppではなく独自にしました）
        m_IsFadingOut = true;
        m_FadeOutStarted = true;
        m_FadeOutCounter = 0;
    }

    return true;  // まだロード中
}

void LoadingScreen::Draw()
{
    if (!m_IsLoading) return;

    // シェーダー設定
    Shader_Begin();
    Shader_SetColor(color::white);

    // UI用の直交投影行列を設定
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
        0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));

    // 動画テクスチャを描画
    ID3D11ShaderResourceView* pSRV = m_Video.GetShaderResourceView();
    if (pSRV && m_pContext)
    {
        m_pContext->PSSetShaderResources(0, 1, &pSRV);
        SetBlendState(BLENDSTATE_NONE);

        XMFLOAT2 pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
        XMFLOAT4 col = color::white;
        DrawSprite(pos, size, col);
    }

    // フェードアウト中は白いオーバーレイを描画
    if (m_IsFadingOut && m_pFadeTexture)
    {
        // アルファ値を調整してしっかり白くフェードアウトさせる
        float alpha = (float)m_FadeOutCounter / (float)m_FadeOutDuration;
        if (alpha > 1.0f) alpha = 1.0f;
        
        XMFLOAT4 fadeCol = { 1.0f, 1.0f, 1.0f, alpha };
        
        // フェード用テクスチャを設定
        m_pContext->PSSetShaderResources(0, 1, &m_pFadeTexture);
        SetBlendState(BLENDSTATE_ALPHA);
        
        XMFLOAT2 pos = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
        DrawSprite(pos, size, fadeCol);
    }
}

bool LoadingScreen::IsLoadingComplete() const
{
    if (m_WaitForVideo)
    {
        return m_LoadComplete && m_VideoFinished;
    }
    else
    {
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
        // フェードアウト完了で次のシーンへ遷移
        SCENE nextScene = g_pLoadingScreen->GetNextScene();

        // LoadingScreenを解放
        LoadingScreen_Finalize();

        // 次のシーンを設定
        SetScene(nextScene);

        // フェードイン開始
        SetFade(60, color::white, FADE_IN, nextScene);
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

    // ロード処理を定義
    std::function<void()> loadTask = [nextScene]()
    {
        switch (nextScene)
        {
        case SCENE_GAME:
            std::this_thread::sleep_for(std::chrono::seconds(5));
            break;

        case SCENE_TITLE:
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            break;

        default:
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            break;
        }
    };

    g_pLoadingScreen->StartLoading(nextScene, loadTask, true);
}