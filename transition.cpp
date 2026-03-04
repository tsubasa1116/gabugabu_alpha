//======================================================
//	transition.cpp[]
//
//	制作者：田中佑奈			日付：2026//
//======================================================

#include	"transition.h"
#include	"shader.h"

static TransitionObject g_Transition;
static ID3D11ShaderResourceView* g_Texture = nullptr;
static ID3D11ShaderResourceView* g_Texture2 = nullptr;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

// スプライトシート設定
static const int SHEET_COLS = 4;              // 列数
static const int SHEET_ROWS = 4;              // 行数
static const int FRAMES_PER_SHEET = SHEET_COLS * SHEET_ROWS; // 1シートあたり16コマ
static const int TOTAL_FRAMES = FRAMES_PER_SHEET * 2;        // 2シート合計32コマ

static int   g_AnimFrame = 0;        // 現在のコマ（0～31）
static float g_AnimTimer = 0.0f;     // コマ送りタイマー

void Transition_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_Transition.sceneChanged = false;
    g_pDevice = pDevice;
    g_pContext = pContext;

    {
        TexMetadata meta;
        ScratchImage img;
        LoadFromWICFile(L"asset\\texture\\uiTransition_v2.png", WIC_FLAGS_NONE, &meta, img);
        CreateShaderResourceView(pDevice, img.GetImages(), img.GetImageCount(), meta, &g_Texture);
    }

    {
        TexMetadata meta;
        ScratchImage img;
        LoadFromWICFile(L"asset\\texture\\uiTransition02_v1.png", WIC_FLAGS_NONE, &meta, img);
        CreateShaderResourceView(pDevice, img.GetImages(), img.GetImageCount(), meta, &g_Texture2);
    }

    g_Transition.progress = 0.0f;
    g_Transition.state = TRANSITION_NONE;
    g_AnimFrame = 0;
    g_AnimTimer = 0.0f;
}

void Transition_Finalize()
{
    if (g_Texture != NULL)
    {
        g_Texture->Release();
        g_Texture = NULL;
    }
    if (g_Texture2 != NULL)
    {
        g_Texture2->Release();
        g_Texture2 = NULL;
    }
}

void Transition_Update()
{
    if (g_Transition.state == TRANSITION_NONE) return;

    // コマ送り（1コマあたりの描画フレーム数 = 全体フレーム数 / 全コマ数）
    float framesPerAnim = g_Transition.frame / (float)TOTAL_FRAMES;
    g_AnimTimer += 1.0f;

    if (g_AnimTimer >= framesPerAnim)
    {
        g_AnimTimer = 0.0f;
        g_AnimFrame++;
    }

    // 1枚目の最終コマでシーン切替フラグを立てる
    if (g_Transition.state == TRANSITION_OUT && g_AnimFrame >= FRAMES_PER_SHEET)
    {
        g_Transition.sceneChanged = true;
        g_Transition.state = TRANSITION_IN;
    }

    // 2枚目の最終コマで終了
    if (g_AnimFrame >= TOTAL_FRAMES)
    {
        g_AnimFrame = TOTAL_FRAMES - 1;
        g_Transition.state = TRANSITION_NONE;
    }

    // sceneChanged フラグが立っていたらシーン切替を実行
    if (g_Transition.sceneChanged)
    {
        g_Transition.sceneChanged = false;
        SetScene(g_Transition.scene);
    }
}

void Transition_Draw()
{
    if (g_Transition.state == TRANSITION_NONE) return;

    Shader_Begin();

    float w = (float)Direct3D_GetBackBufferWidth();
    float h = (float)Direct3D_GetBackBufferHeight();

    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0, w, h, 0, 0, 1));
    SetBlendState(BLENDSTATE_ALPHA);

    // どちらのシートか判定
    ID3D11ShaderResourceView* currentTex;
    int localFrame;

    if (g_AnimFrame < FRAMES_PER_SHEET)
    {
        currentTex = g_Texture;
        localFrame = g_AnimFrame;
    }
    else
    {
        currentTex = g_Texture2;
        localFrame = g_AnimFrame - FRAMES_PER_SHEET;
    }

    // コマ位置からUV座標を計算
    int col = localFrame % SHEET_COLS;
    int row = localFrame / SHEET_COLS;

    float u0 = (float)col / (float)SHEET_COLS;
    float v0 = (float)row / (float)SHEET_ROWS;
    float u1 = (float)(col + 1) / (float)SHEET_COLS;
    float v1 = (float)(row + 1) / (float)SHEET_ROWS;

    // テクスチャをセットして全画面描画（白で描画して絵をそのまま表示）
    g_pContext->PSSetShaderResources(0, 1, &currentTex);
    XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
    DrawSpriteUV({ w / 2, h / 2 }, { w, h }, white,
        XMFLOAT2(u0, v0), XMFLOAT2(u1, v1));
}

void SetTransition(float frame, XMFLOAT4 color, TRANSITION_STATE state, SCENE scene)
{
    g_Transition.frame = frame;
    g_Transition.color = color;
    g_Transition.scene = scene;

    g_Transition.progress = 0.0f;
    g_Transition.sceneChanged = false;
    g_Transition.state = TRANSITION_OUT;

    // アニメーションリセット
    g_AnimFrame = 0;
    g_AnimTimer = 0.0f;
}

TRANSITION_STATE GetTransitionState()
{
    return g_Transition.state;
}