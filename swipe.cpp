//======================================================
//	swipe.cpp[]
//
//	制作者：田中佑奈			日付：2026//
//======================================================

#include	"swipe.h"
#include	"shader.h"

static SwipeObject g_Swipe;
static ID3D11ShaderResourceView* g_Texture = nullptr;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

void Swipe_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_Swipe.sceneChanged = false;
    g_pDevice = pDevice;
    g_pContext = pContext;

    TexMetadata meta;
    ScratchImage img;
    LoadFromWICFile(L"asset\\texture\\fade.bmp", WIC_FLAGS_NONE, &meta, img);
    CreateShaderResourceView(pDevice, img.GetImages(), img.GetImageCount(), meta, &g_Texture);

    g_Swipe.progress = 0.0f;
    g_Swipe.state = SWIPE_NONE;
}

void Swipe_Finalize()
{
    if (g_Texture != NULL)
    {
        g_Texture->Release();
        g_Texture = NULL;
    }
}

void Swipe_Update()
{
    // sceneChanged フラグが立っていたらここで実際にシーン切替を行う
    // （Draw 中ではなく Update 中に行うことで描画直中の重い処理を避ける）
    if (g_Swipe.sceneChanged)
    {
        g_Swipe.sceneChanged = false;
        SetScene(g_Swipe.scene);
    }
}

void Swipe_Draw()
{
    if (g_Swipe.state == SWIPE_NONE) return;

    Shader_Begin();

    float w = (float)Direct3D_GetBackBufferWidth();
    float h = (float)Direct3D_GetBackBufferHeight();

    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0, w, h, 0, 0, 1));
    g_pContext->PSSetShaderResources(0, 1, &g_Texture);
    SetBlendState(BLENDSTATE_ALPHA);

    float t = g_Swipe.progress;

    float x;
    if (t <= 1.0f)
    {
        // ① 右 → 左 に覆う
        x = w * (1.0f - t) + w / 2;
    }
    else
    {
        // ② そのまま左へ抜ける
        x = w * (-(t - 1.0f)) + w / 2;
    }

    DrawSprite({ x, h / 2 }, { w, h }, g_Swipe.color);

    float speed = 1.0f / g_Swipe.frame;

    if (g_Swipe.state == SWIPE_OUT)
    {
        g_Swipe.progress += speed;
        if (g_Swipe.progress >= 1.0f)
        {
            g_Swipe.progress = 1.0f;
            // Draw 中ではシーン切替を実行せず、フラグだけ立てる
            g_Swipe.sceneChanged = true;
            g_Swipe.state = SWIPE_IN;
        }
    }
    else if (g_Swipe.state == SWIPE_IN)
    {
        g_Swipe.progress += speed;   // ★ 減らさない
        if (g_Swipe.progress >= 2.0f)
        {
            g_Swipe.state = SWIPE_NONE;
        }
    }
}

void SetSwipe(int frame, XMFLOAT4 color, SWIPE_STATE state, SCENE scene)
{
    g_Swipe.frame = frame;
    g_Swipe.color = color;
    g_Swipe.scene = scene;

    g_Swipe.progress = 0.0f;
    g_Swipe.sceneChanged = false;
    g_Swipe.state = SWIPE_OUT;
}

SWIPE_STATE	GetSwipeState()
{
    return	g_Swipe.state;	//現在の状態
}