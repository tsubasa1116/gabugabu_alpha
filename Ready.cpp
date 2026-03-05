//======================================================
//	ready.cpp[]
// 
//	蛻ｶ菴懆・ｼ夂伐荳ｭ菴大･・		譌･莉假ｼ・024//
//======================================================

//Ready.cpp
#include	"Manager.h"
#include	"sprite.h"
#include	"keyboard.h"

#include	"Ready.h"

#include "fade.h"
#include "swipe.h"
#include "shader.h"
#include "input.h"
#include "color.h"

#include <chrono>
#include <cmath>
#include "LoadingScreen.h"
#include "loadThread.h"

static	ID3D11ShaderResourceView* g_Texture = NULL;	//繝・け繧ｹ繝√Ε・第椢繧定｡ｨ縺吶が繝悶ず繧ｧ繧ｯ繝・
static	ID3D11ShaderResourceView* g_Texture2 = NULL;
static	ID3D11ShaderResourceView* g_Texture3 = NULL;
static	ID3D11ShaderResourceView* g_Texture4 = NULL;
static	ID3D11ShaderResourceView* g_Texture5 = NULL;
static	ID3D11ShaderResourceView* g_Texture6 = NULL;
static	ID3D11ShaderResourceView* g_Texture7 = NULL;
static	ID3D11ShaderResourceView* g_Texture8 = NULL;
static	ID3D11ShaderResourceView* g_Texture9 = NULL;
static	ID3D11ShaderResourceView* g_Texture10 = NULL;
static	ID3D11ShaderResourceView* g_Texture11 = NULL;
static	ID3D11ShaderResourceView* g_Texture12 = NULL;
static	ID3D11ShaderResourceView* g_Texture13 = NULL;
static	ID3D11ShaderResourceView* g_Texture14 = NULL;
static	ID3D11ShaderResourceView* g_Texture15 = NULL;
static	ID3D11ShaderResourceView* g_Texture16 = NULL;
static	ID3D11ShaderResourceView* g_Texture17 = NULL;
static	ID3D11ShaderResourceView* g_Texture18 = NULL;
static	ID3D11ShaderResourceView* g_Texture19 = NULL;

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static TexMetadata g_TexMeta2{};
static TexMetadata g_TexMeta3{};
static TexMetadata g_TexMeta4{};
static TexMetadata g_TexMeta5{};
static TexMetadata g_TexMeta6{};
static TexMetadata g_TexMeta7{};
static TexMetadata g_TexMeta8{};
static TexMetadata g_TexMeta9{};
static TexMetadata g_TexMeta10{};
static TexMetadata g_TexMeta11{};
static TexMetadata g_TexMeta12{};
static TexMetadata g_TexMeta13{};
static TexMetadata g_TexMeta14{};
static TexMetadata g_TexMeta15{};
static TexMetadata g_TexMeta16{};
static TexMetadata g_TexMeta17{};
static TexMetadata g_TexMeta18{};
static TexMetadata g_TexMeta19{};

// 繝励Ξ繧､繝､繝ｼ蜿ょ刈繝輔Λ繧ｰ・井ｸ蠎ｦ謚ｼ縺励◆繧液rue・・
static bool g_PlayerJoined[4] = { false, false, false, false };

// OK繝昴ャ繝励う繝ｳ繧｢繝九Γ繝ｼ繧ｷ繝ｧ繝ｳ逕ｨ
static constexpr float OK_POP_DURATION = 0.4f; // 繝昴ャ繝励う繝ｳ謇隕∵凾髢難ｼ育ｧ抵ｼ・
static float g_OKPopElapsed[4] = { -1.0f, -1.0f, -1.0f, -1.0f }; // -1 = 髱櫁｡ｨ遉ｺ

// 貅門ｙ螳御ｺ・せ繝ｩ繧､繝峨う繝ｳ逕ｨ
static constexpr float READY_SLIDE_DURATION = 0.3f; // 繧ｹ繝ｩ繧､繝画園隕∵凾髢難ｼ育ｧ抵ｼ・
static float g_ReadySlideElapsed = -1.0f; // -1 = 髱櫁｡ｨ遉ｺ
static bool g_AllJoinedTriggered = false;  // 蜈ｨ蜩｡蜿ょ刈讀懃衍貂医∩繝輔Λ繧ｰ

// 貅門ｙ繝・く繧ｹ繝医・繝・・繧｢繧ｦ繝育畑
static constexpr float TEXT_POPOUT_DURATION = 0.3f; // 繝昴ャ繝励い繧ｦ繝域園隕∵凾髢難ｼ育ｧ抵ｼ・
static float g_TextPopOutElapsed = -1.0f; // -1 = 譛ｪ髢句ｧ・

// 蜈ｨ蜩｡蜿ょ刈蠕後・閾ｪ蜍暮・遘ｻ繧ｿ繧､繝槭・
static constexpr float AUTO_TRANSITION_DELAY = 2.0f; // 蜈ｨ蜩｡OK蠕後・蠕・ｩ滓凾髢難ｼ育ｧ抵ｼ・
static float g_AutoTransitionTimer = -1.0f;           // -1 = 譛ｪ髢句ｧ・

// 譎る俣邂｡逅・
static std::chrono::steady_clock::time_point g_ReadyLastTime;

static bool g_ReadyInitialized = false;
static bool g_IsWarmedUp = false;

// 繧､繝ｼ繧ｸ繝ｳ繧ｰ・医し繧､繝ｳ縺ｮ繧､繝ｼ繧ｺ繧｢繧ｦ繝茨ｼ・
static inline float EaseOutSine(float t) {
	if (t <= 0.0f) return 0.0f;
	if (t >= 1.0f) return 1.0f;
	return sinf(t * 3.14159265f * 0.5f);
}

void Ready_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	if (g_ReadyInitialized) return;

	g_pDevice = pDevice;
	g_pContext = pContext;

	// 蜿ょ刈繝輔Λ繧ｰ蛻晄悄蛹・
	for (int i = 0; i < 4; i++) g_PlayerJoined[i] = false;

	// OK繝昴ャ繝励う繝ｳ蛻晄悄蛹・
	for (int i = 0; i < 4; i++) g_OKPopElapsed[i] = -1.0f;

	// 貅門ｙ螳御ｺ・せ繝ｩ繧､繝牙・譛溷喧
	g_ReadySlideElapsed = -1.0f;
	g_AllJoinedTriggered = false;
	g_TextPopOutElapsed = -1.0f;
	g_IsWarmedUp = false;

	// 閾ｪ蜍暮・遘ｻ繧ｿ繧､繝槭・蛻晄悄蛹・
	g_AutoTransitionTimer = -1.0f;

	// 譎る俣蛻晄悄蛹・
	g_ReadyLastTime = std::chrono::steady_clock::now();

	Loader::AddTask([pDevice]()
		{
			//逋ｽ繝・け繧ｹ繝√Ε隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata		metadata;
				ScratchImage	image;
				LoadFromWICFile(L"asset\\texture\\white.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
				assert(g_Texture);//隱ｭ縺ｿ霎ｼ縺ｿ螟ｱ謨玲凾縺ｫ繝繧､繧｢繝ｭ繧ｰ繧定｡ｨ遉ｺ
			}

			// 閭梧勹隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\ready_color2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture2);
				g_TexMeta2 = metadata;
				assert(g_Texture2);
			}

			// 鮟呈棧隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\blackLine2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture3);
				g_TexMeta3 = metadata;
				assert(g_Texture3);
			}

			// 1・ｰ繧ｷ繝ｫ繧ｨ繝・ヨ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character1OFF2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture4);
				g_TexMeta4 = metadata;
				assert(g_Texture4);
			}

			// ・抵ｼｰ繧ｷ繝ｫ繧ｨ繝・ヨ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character2OFF2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture5);
				g_TexMeta5 = metadata;
				assert(g_Texture5);
			}

			// ・難ｼｰ繧ｷ繝ｫ繧ｨ繝・ヨ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character3OFF2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture6);
				g_TexMeta6 = metadata;
				assert(g_Texture6);
			}

			// ・費ｼｰ繧ｷ繝ｫ繧ｨ繝・ヨ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character4OFF2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture7);
				g_TexMeta7 = metadata;
				assert(g_Texture7);
			}

			// 繝励Ξ繧､繝､繝ｼ繝翫Φ繝舌・隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\playerNumber2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture8);
				g_TexMeta8 = metadata;
				assert(g_Texture8);
			}

			// 蜷ｹ縺榊・縺苓ｪｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\hukidashi2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture9);
				g_TexMeta9 = metadata;
				assert(g_Texture9);
			}

			// 蜷ｹ縺榊・縺苓ｪｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\ready2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture10);
				g_TexMeta10 = metadata;
				assert(g_Texture10);
			}

			// ・托ｼｰ繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character1ON2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture11);
				g_TexMeta11 = metadata;
				assert(g_Texture11);
			}

			// ・抵ｼｰ繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character2ON2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture12);
				g_TexMeta12 = metadata;
				assert(g_Texture12);
			}

			// ・難ｼｰ繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character3ON2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture13);
				g_TexMeta13 = metadata;
				assert(g_Texture13);
			}

			// ・費ｼｰ繧ｭ繝｣繝ｩ繧ｯ繧ｿ繝ｼ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\character4ON2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture14);
				g_TexMeta14 = metadata;
				assert(g_Texture14);
			}

			// ・托ｼｰ・ｯ・ｫ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture15);
				g_TexMeta15 = metadata;
				assert(g_Texture15);
			}

			// ・抵ｼｰ・ｯ・ｫ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture16);
				g_TexMeta16 = metadata;
				assert(g_Texture16);
			}

			// ・難ｼｰ・ｯ・ｫ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture17);
				g_TexMeta17 = metadata;
				assert(g_Texture17);
			}

			// ・費ｼｰ・ｯ・ｫ隱ｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\OK2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture18);
				g_TexMeta18 = metadata;
				assert(g_Texture18);
			}

			// 貅門ｙ螳御ｺ・ｪｭ縺ｿ霎ｼ縺ｿ
			{
				TexMetadata metadata;
				ScratchImage image;
				LoadFromWICFile(L"asset\\texture\\everyoneOK2.png", WIC_FLAGS_NONE, &metadata, image);
				CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture19);
				g_TexMeta19 = metadata;
				assert(g_Texture19);
			}
		});

		Loader::StartTaskLoad();
		g_ReadyInitialized = true;

}


void Ready_Warmup()
{
	if (!g_pContext) return;

	// 蜈ｨ繝・け繧ｹ繝√Ε繧帝・蛻励↓縺ｾ縺ｨ繧√ｋ
	ID3D11ShaderResourceView* textures[] = {
		g_Texture,  g_Texture2,  g_Texture3,  g_Texture4,  g_Texture5,
		g_Texture6,  g_Texture7,  g_Texture8,  g_Texture9,  g_Texture10,
		g_Texture11, g_Texture12, g_Texture13, g_Texture14, g_Texture15,
		g_Texture16, g_Texture17, g_Texture18, g_Texture19
	};

	for (auto tex : textures)
	{
		if (tex)
		{
			// 繧ｹ繝ｭ繝・ヨ0縺ｫ繧ｻ繝・ヨ縺励※縲・繝昴Μ繧ｴ繝ｳ謠冗判・医ヰ繧､繝ｳ繝峨ｒ蠑ｷ蛻ｶ縺吶ｋ・・
			g_pContext->PSSetShaderResources(0, 1, &tex);
			g_pContext->Draw(0, 0);
		}
	}


	// 邨ゅｏ縺｣縺溘ｉ繧ｹ繝ｭ繝・ヨ繧堤ｩｺ縺ｫ縺励※縺翫￥
	ID3D11ShaderResourceView* nullSRV = nullptr;
	g_pContext->PSSetShaderResources(0, 1, &nullSRV);
}

void Ready_Finalize()
{
	//繝・け繧ｹ繝√Ε縺ｮ隗｣謾ｾ縺ｪ縺ｩ
	SAFE_RELEASE(g_Texture);
	SAFE_RELEASE(g_Texture2);
	SAFE_RELEASE(g_Texture3);
	SAFE_RELEASE(g_Texture4);
	SAFE_RELEASE(g_Texture5);
	SAFE_RELEASE(g_Texture6);
	SAFE_RELEASE(g_Texture7);
	SAFE_RELEASE(g_Texture8);
	SAFE_RELEASE(g_Texture9);
	SAFE_RELEASE(g_Texture10);
	SAFE_RELEASE(g_Texture11);
	SAFE_RELEASE(g_Texture12);
	SAFE_RELEASE(g_Texture13);
	SAFE_RELEASE(g_Texture14);
	SAFE_RELEASE(g_Texture15);
	SAFE_RELEASE(g_Texture16);
	SAFE_RELEASE(g_Texture17);
	SAFE_RELEASE(g_Texture18);
	SAFE_RELEASE(g_Texture19);

	g_ReadyInitialized = false;
}

void Ready_Update()
{
	// 譎る俣蟾ｮ蛻・峩譁ｰ
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<float> elapsed = now - g_ReadyLastTime;
	float dt = elapsed.count();
	g_ReadyLastTime = now;

	// 繝励Ξ繧､繝､繝ｼ蜿ょ刈蛻､螳夲ｼ域款縺吶◆縺ｳ縺ｫ繝昴ャ繝励う繝ｳ繝ｪ繧ｹ繧ｿ繝ｼ繝茨ｼ・
	if (Keyboard_IsKeyDownTrigger(KK_D1) || (g_Input[0].A)) { g_PlayerJoined[0] = true; g_OKPopElapsed[0] = 0.0f; }
	if (Keyboard_IsKeyDownTrigger(KK_D2) || (g_Input[1].A)) { g_PlayerJoined[1] = true; g_OKPopElapsed[1] = 0.0f; }
	if (Keyboard_IsKeyDownTrigger(KK_D3) || (g_Input[2].A)) { g_PlayerJoined[2] = true; g_OKPopElapsed[2] = 0.0f; }
	if (Keyboard_IsKeyDownTrigger(KK_D4) || (g_Input[3].A)) { g_PlayerJoined[3] = true; g_OKPopElapsed[3] = 0.0f; }

	// OK繝昴ャ繝励う繝ｳ繧ｿ繧､繝槭・騾ｲ陦・
	for (int i = 0; i < 4; i++)
	{
		if (g_OKPopElapsed[i] >= 0.0f)
			g_OKPopElapsed[i] += dt;
	}

	// 蜈ｨ蜩｡蜿ょ刈縺励◆繧画ｺ門ｙ螳御ｺ・せ繝ｩ繧､繝蛾幕蟋・& 繝・く繧ｹ繝医・繝・・繧｢繧ｦ繝磯幕蟋・& 閾ｪ蜍暮・遘ｻ繧ｿ繧､繝槭・髢句ｧ・
	if (!g_AllJoinedTriggered &&
		g_PlayerJoined[0] && g_PlayerJoined[1] &&
		g_PlayerJoined[2] && g_PlayerJoined[3])
	{
		g_AllJoinedTriggered = true;
		g_ReadySlideElapsed = 0.0f;
		g_TextPopOutElapsed = 0.0f;
		g_AutoTransitionTimer = 0.0f;
	}

	// 貅門ｙ螳御ｺ・せ繝ｩ繧､繝峨ち繧､繝槭・騾ｲ陦・
	if (g_ReadySlideElapsed >= 0.0f)
		g_ReadySlideElapsed += dt;

	// 繝・く繧ｹ繝医・繝・・繧｢繧ｦ繝医ち繧､繝槭・騾ｲ陦・
	if (g_TextPopOutElapsed >= 0.0f)
		g_TextPopOutElapsed += dt;

	//キー入力チェック
	//スタートボタンが押されたらシーンを切り替え
	//フェード処理中はキーを受け付けない
	if ((Keyboard_IsKeyDownTrigger(KK_ENTER) || (g_Input->X) )&& (GetFadeState() == FADE_NONE) && !IsLoading())
	{
		if(g_AllJoinedTriggered)
		{
			XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
			SetFadeWithLoading(40, color, FADE_OUT, SCENE_GAME, L"asset\\movie\\gameLoad.mp4");
		}
	}
	
}

void Ready_Draw()
{
	if (!Loader::IsFinished) return;

	if (!g_IsWarmedUp)
	{
		Ready_Warmup();
		g_IsWarmedUp = true;
	}

	// 繧ｷ繧ｧ繝ｼ繝繝ｼ繧呈緒逕ｻ繝代う繝励Λ繧､繝ｳ縺ｫ險ｭ螳・
	Shader_BeginUI();

	// 鬆らせ繧ｷ繧ｧ繝ｼ繝繝ｼ縺ｫ螟画鋤陦悟・繧定ｨｭ螳・
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));

	// 逋ｽ謠冗判
	if (g_Texture)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture);//g_Texture繧剃ｽｿ縺・ｈ縺・↓險ｭ螳壹☆繧・
		SetBlendState(BLENDSTATE_NONE);//繝悶Ξ繝ｳ繝臥┌縺・
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };	//繧ｹ繝励Λ繧､繝医・濶ｲ
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
		DrawSprite(pos, size, col);//1譫夂ｵｵ繧定｡ｨ遉ｺ
	}

	// 閭梧勹謠冗判
	if (g_Texture2)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture2);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta2.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta2.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// 繝励Ξ繧､繝､繝ｼ繝翫Φ繝舌・謠冗判
	if (g_Texture8)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture8);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 110 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta8.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta8.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// ・托ｼｰ繧ｷ繝ｫ繧ｨ繝・ヨ謠冗判
	if (g_Texture4)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture4);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT / 2 + 30 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta4.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta4.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// ・抵ｼｰ繧ｷ繝ｫ繧ｨ繝・ヨ謠冗判
	if (g_Texture5)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture5);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2 , SCREEN_HEIGHT / 2 + 33 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta5.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta5.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// ・難ｼｰ繧ｷ繝ｫ繧ｨ繝・ヨ謠冗判
	if (g_Texture6)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture6);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 33 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta6.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta6.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// ・費ｼｰ繧ｷ繝ｫ繧ｨ繝・ヨ謠冗判
	if (g_Texture7)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture7);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 33 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta7.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta7.height * 0.8f * SCREEN_ADJUST_Y};
		DrawSprite(pos, size, col);
	}

	// ・托ｼｰ謠冗判・亥盾蜉貂医∩縺ｪ繧牙ｸｸ縺ｫ陦ｨ遉ｺ・・
	if (g_PlayerJoined[0] && g_Texture11)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture11);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 30 }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta11.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta11.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// ・抵ｼｰ謠冗判・亥盾蜉貂医∩縺ｪ繧牙ｸｸ縺ｫ陦ｨ遉ｺ・・
	if (g_PlayerJoined[1] && g_Texture12)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture12);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2 , SCREEN_HEIGHT / 2 + 33 }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta12.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta12.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// ・難ｼｰ謠冗判・亥盾蜉貂医∩縺ｪ繧牙ｸｸ縺ｫ陦ｨ遉ｺ・・
	if (g_PlayerJoined[2] && g_Texture13)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture13);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 25 }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta13.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta13.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// ・費ｼｰ謠冗判・亥盾蜉貂医∩縺ｪ繧牙ｸｸ縺ｫ陦ｨ遉ｺ・・
	if (g_PlayerJoined[3] && g_Texture14)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture14);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 28  }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta14.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta14.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// 鮟呈棧謠冗判
	if (g_Texture3)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture3);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta3.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta3.height * 0.8f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// 貅門ｙ縺後〒縺阪◆繧峨・繧ｿ繝ｳ繧呈款縺励※縺ｭ謠冗判・亥・蜩｡蜿ょ刈縺ｧ蜿ｳ縺ｸ繝昴ャ繝励い繧ｦ繝茨ｼ・
	if (g_Texture10)
	{
		if (g_TextPopOutElapsed >= 0.0f)
		{
			float t = g_TextPopOutElapsed / TEXT_POPOUT_DURATION;
			if (t > 1.0f) t = 1.0f;
			float e = EaseOutSine(t);

			// 荳ｭ螟ｮ縺九ｉ蜿ｳ逕ｻ髱｢螟悶∈
			float startX = SCREEN_WIDTH / 2;
			float endX = SCREEN_WIDTH * 1.5f;
			float posX = startX + (endX - startX) * e;
			float alpha = 1.0f - e;

			// 螳悟・縺ｫ豸医∴縺溘ｉ謠冗判縺励↑縺・
			if (t < 1.0f)
			{
				g_pContext->PSSetShaderResources(0, 1, &g_Texture10);
				SetBlendState(BLENDSTATE_ALPHA);
				XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, alpha };
				XMFLOAT2 pos = { posX * SCREEN_ADJUST_X, SCREEN_HEIGHT / 2 + 4 * SCREEN_ADJUST_Y };
				XMFLOAT2 size = { (float)g_TexMeta9.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta9.height * 0.12f * SCREEN_ADJUST_Y };
				DrawSprite(pos, size, col);
			}
		}
		else
		{
			// 騾壼ｸｸ陦ｨ遉ｺ
			g_pContext->PSSetShaderResources(0, 1, &g_Texture10);
			SetBlendState(BLENDSTATE_ALPHA);
			XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
			XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 + 4 * SCREEN_ADJUST_Y };
			XMFLOAT2 size = { (float)g_TexMeta9.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta9.height * 0.12f * SCREEN_ADJUST_Y };
			DrawSprite(pos, size, col);
		}
	}

	// 貅門ｙ螳御ｺ・緒逕ｻ・亥・蜩｡蜿ょ刈縺ｧ蟾ｦ縺九ｉ繧ｹ繝ｩ繧､繝峨う繝ｳ・・
	if (g_ReadySlideElapsed >= 0.0f && g_Texture19)
	{
		float t = g_ReadySlideElapsed / READY_SLIDE_DURATION;
		if (t > 1.0f) t = 1.0f;
		float e = EaseOutSine(t);

		// 蟾ｦ逕ｻ髱｢螟悶°繧峨せ繝ｩ繧､繝峨う繝ｳ
		float startX = -SCREEN_WIDTH * 0.3f;
		float endX = SCREEN_WIDTH / 2;
		float posX = startX + (endX - startX) * e;

		g_pContext->PSSetShaderResources(0, 1, &g_Texture19);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, e };
		XMFLOAT2 pos = { posX , SCREEN_HEIGHT / 2 + 4 * SCREEN_ADJUST_Y };
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.8f * SCREEN_ADJUST_X, (float)g_TexMeta9.height * 0.12f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// 蜷ｹ縺榊・縺玲緒逕ｻ
	if (g_Texture9)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture9);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.78f * SCREEN_ADJUST_X, (float)g_TexMeta7.height * 0.78f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// 謠冗判
	if (g_Texture9)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture9);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.78f * SCREEN_ADJUST_X, (float)g_TexMeta7.height * 0.78f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// 蜷ｹ縺榊・縺玲緒逕ｻ
	if (g_Texture9)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_Texture9);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 60 * SCREEN_ADJUST_Y }; // 菴咲ｽｮ縺ｯ縺雁･ｽ縺ｿ縺ｧ
		XMFLOAT2 size = { (float)g_TexMeta9.width * 0.78f * SCREEN_ADJUST_X, (float)g_TexMeta7.height * 0.78f * SCREEN_ADJUST_Y };
		DrawSprite(pos, size, col);
	}

	// OK繝・け繧ｹ繝√Ε繝ｻ菴咲ｽｮ縺ｮ螳夂ｾｩ・・P?4P・・
	ID3D11ShaderResourceView* okTextures[4] = { g_Texture15, g_Texture16, g_Texture17, g_Texture18 };
	TexMetadata* okMetas[4] = { &g_TexMeta15, &g_TexMeta16, &g_TexMeta17, &g_TexMeta18 };
	XMFLOAT2 okPositions[4] = {
		{ SCREEN_WIDTH / 2 - 505 * SCREEN_ADJUST_X, SCREEN_HEIGHT / 2 - 250 * SCREEN_ADJUST_Y },
		{ SCREEN_WIDTH / 2 + 505 * SCREEN_ADJUST_X, SCREEN_HEIGHT / 2 - 250 * SCREEN_ADJUST_Y },
		{ SCREEN_WIDTH / 2 - 515 * SCREEN_ADJUST_X, SCREEN_HEIGHT / 2 + 130 * SCREEN_ADJUST_Y },
		{ SCREEN_WIDTH / 2 + 510 * SCREEN_ADJUST_X, SCREEN_HEIGHT / 2 + 130 * SCREEN_ADJUST_Y }
	};


	// ・托ｼｰ・橸ｼ費ｼｰ・ｯ・ｫ謠冗判・医・繝・・繧､繝ｳ莉倥″・・
	for (int i = 0; i < 4; i++)
	{
		if (g_OKPopElapsed[i] < 0.0f || !okTextures[i]) continue;

		float t = g_OKPopElapsed[i] / OK_POP_DURATION;
		if (t > 1.0f) t = 1.0f;
		float e = EaseOutSine(t);

		float okScale = 0.5f + 0.5f * e;  // 0.5 竊・1.0
		float okAlpha = e;                 // 0 竊・1

		float baseScaleX = (i == 3) ? 0.78f : 0.8f; // 4P縺縺大・縺ｮ繧ｹ繧ｱ繝ｼ繝ｫ縺碁＆縺・
		float baseScaleY = (i == 3) ? 0.78f : 0.8f;

		XMFLOAT2 baseSize = {
			(float)okMetas[i]->width * baseScaleX,
			(float)okMetas[i]->height * baseScaleY
		};
		XMFLOAT2 size = { baseSize.x * okScale, baseSize.y * okScale };

		g_pContext->PSSetShaderResources(0, 1, &okTextures[i]);
		SetBlendState(BLENDSTATE_ALPHA);
		XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, okAlpha };
		DrawSprite(okPositions[i], size, col);
	}
}