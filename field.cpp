//======================================================
//	field.cpp[]
//======================================================
#include "field.h"
#include "Camera.h"
#include "keyboard.h"
#include "collider.h"
#include "debug_render.h"
#include "model.h"
#include "Building.h"
#include "player.h"
#include "special.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include "color.h"

//======================================================
//	ãƒã‚¯ãƒ­å®šç¾©
//======================================================
#define BOX_NUM_VERTEX	(24)
#define FIELD_TEX_MAX	(2)

//======================================================
//	ã‚°ãƒ­ãƒ¼ãƒãƒ«å¤‰æ•°
//======================================================
MODEL* Test = NULL;//ãƒEƒãƒE‚°

////ã‚°ãƒ­ãƒ¼ãƒãƒ«å¤‰æ•°
static	ID3D11Device* g_pDevice = NULL;
static	ID3D11DeviceContext* g_pContext = NULL;
////é ‚ç‚¹ãƒãƒƒãƒ•ã‚¡
//static	ID3D11Buffer* g_VertexBuffer = NULL;
////ã‚¤ãƒ³ãƒEƒƒã‚¯ã‚¹ãƒãƒƒãƒ•ã‚¡
//static	ID3D11Buffer* g_IndexBuffer = NULL;
//ãƒE‚¯ã‚¹ãƒãƒ£å¤‰æ•°
//static ID3D11ShaderResourceView* g_Texture;

// FIELD enum (FIELD_BUILDING, FIELD_BOX) ã®æ•°ã ã‘ãƒ†ã‚¯ã‚¹ãƒãƒ£ã‚’ç®¡çE
static ID3D11ShaderResourceView* g_Texture[FIELD_TEX_MAX];
#define FIELD_TEX_MAX (4)
// FIELD::no ã®å€¤ã«å¯¾å¿œã™ã‚‹ãƒ†ã‚¯ã‚¹ãƒãƒ£ãƒ•ã‚¡ã‚¤ãƒ«åE
static const wchar_t* g_TexturePaths[FIELD_TEX_MAX] =
{
	L"Asset\\Texture\\texturefieldConcrete02_v1.png",  // 0
	L"Asset\\Texture\\texturefieldTree02_v1.png",  // 1
	L"Asset\\Texture\\texturefieldTree01_v1.png",  
	L"Asset\\Texture\\texturefieldConcrete03_v1.png",
	//L"Asset\\Texture\\texturefieldConcrete01_v1.png",// 1
};

static const char* g_ModelName[] = {
	"field",
	"field_v2",
	"field_v3",
	"propsConcreteMain_v2",		// 3ãƒã‚¹å¤§å»ºç‰©
	"propsConcreteSub_v2",		// ãƒãƒ³ã‚·ãƒ§ãƒ³
	"propsElectricitySub_v2",	// è»Šã¨ä¿¡å·
	"propsGlassSub_v2",			// ãƒ“ãƒ«
	"propsTreeSub_v2",			// åºE‘‰æ¨¹
	"build_glass_new"			// å¤‰ãªå»ºç‰©
	"propsTowerMain_v3"			//æ±äº¬ã‚¿ãƒ¯-
};
static const char* g_ModelName1[] = {
	"raibu",
	"kitosaku"
};
 
//ãƒãƒƒãƒ—ãƒ‡ãƒ¼ã‚¿é…åE
MAPDATA Map[] =
{
	// ===== åœ°é¢ãƒ»ç‰¹æ®E=====			 
	{ {},{}, FIELD::FIELD_Electricity,1}, // 1kaku
	{ {},{}, FIELD::FIELD_Electricity,0}, // 2kaku
	{ {},{}, FIELD::FIELD_Plant,2},           // 3kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 4kaku
	{ {},{}, FIELD::FIELD_Plant,2}, // 5kaku
	{ {},{}, FIELD::FIELD_Electricity,0},           // 6kaku
	{ {},{}, FIELD::FIELD_Plant,2},           // 7kaku
	{ {},{}, FIELD::FIELD_Concrete,2},           // 8kaku
	{ {},{}, FIELD::FIELD_Concrete,1},           // 9
	{ {},{}, FIELD::FIELD_Glass},           // 10
						  
	// ===== BOX 10 ===== 
	{ {},{}, FIELD::FIELD_Glass,}, // 11
	{ {},{}, FIELD::FIELD_Plant,2}, // 12kaku
	{ {},{}, FIELD::FIELD_Plant,2}, // 13kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 14kaku
	{ {},{}, FIELD::FIELD_Glass}, // 15kaku
	{ {},{}, FIELD::FIELD_Electricity,4}, // 16
	{ {},{}, FIELD::FIELD_Electricity,0}, // 17kaku
	{ {},{}, FIELD::FIELD_Concrete,2}, // 18
	{ {},{}, FIELD::FIELD_Electricity,0}, // 19kaku
	{ {},{}, FIELD::FIELD_Concrete,1}, // 20kaku
						  
	// ===== BOX 20 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 21
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 22
	{ {},{}, FIELD::FIELD_Plant,1 }, // 23
	{ {},{}, FIELD::FIELD_Plant,2}, // 24
	{ {},{}, FIELD::FIELD_Plant,2 }, // 25
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 26
	{ {},{}, FIELD::FIELD_Concrete,2}, // 27
	{ {},{}, FIELD::FIELD_Plant,2 }, // 28
	{ {},{}, FIELD::FIELD_Plant,3 }, // 29
	{ {},{}, FIELD::FIELD_Glass }, // 30
						 
	// ===== BOX 30 =====
	{ {},{}, FIELD::FIELD_Electricity,0 }, // 31
	{ {},{}, FIELD::FIELD_Plant,2 }, // 32
	{ {},{}, FIELD::FIELD_Plant,2 }, // 33
	{ {},{}, FIELD::FIELD_Electricity,4}, // 34
	{ {},{}, FIELD::FIELD_Glass }, // 35
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 36
	{ {},{}, FIELD::FIELD_Plant,2 }, // 37
	{ {},{}, FIELD::FIELD_Glass,3 }, // 38
	{ {},{}, FIELD::FIELD_BOX }, // 39
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 40
						 
	// ===== BOX 40 =====
	{ {},{}, FIELD::FIELD_Plant,2 }, // 41
	{ {},{}, FIELD::FIELD_Plant,2 }, // 42
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 43
	{ {},{}, FIELD::FIELD_Glass }, // 44
	{ {},{}, FIELD::FIELD_Plant,2 }, // 45
	{ {},{}, FIELD::FIELD_Glass, }, // 46
	{ {},{}, FIELD::FIELD_Electricity,4}, // 47
	{ {},{}, FIELD::FIELD_Glass,1 }, // 48  ¶ãƒfƒJ‚¢Œš•¨
	{ {},{}, FIELD::FIELD_Electricity,2 }, // 49   ‰EãƒfƒJ‚¢Œš•¨
	{ {},{}, FIELD::FIELD_Electricity,4}, // 50
						 
	// ===== BOX 50 =====
	{ {},{}, FIELD::FIELD_Electricity }, // 51
	{ {},{}, FIELD::FIELD_Concrete }, // 52 ‰E‰ºƒfƒJ‚¢Œš•¨
	{ {},{}, FIELD::FIELD_Plant,5}, // 53¶‰ºƒfƒJ‚¢
	{ {},{}, FIELD::FIELD_Electricity}, // 54
	{ {},{}, FIELD::FIELD_Plant,2}, // 55
	{ {},{}, FIELD::FIELD_Glass,3}, // 56
	{ {},{}, FIELD::FIELD_Electricity}, // 57
	{ {},{}, FIELD::FIELD_Plant,2}, // 58
	{ {},{}, FIELD::FIELD_Concrete,1}, // 59
	{ {},{}, FIELD::FIELD_Electricity,}, // 60
						  
	// ===== BOX 60 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 61
	{ {},{}, FIELD::FIELD_Glass }, // 62
	{ {},{}, FIELD::FIELD_Electricity }, // 63
	{ {},{}, FIELD::FIELD_Electricity }, // 64
	{ {},{}, FIELD::FIELD_Electricity,3 }, // 65
	{ {},{}, FIELD::FIELD_Electricity }, // 66
	{ {},{}, FIELD::FIELD_Glass }, // 67
	{ {},{}, FIELD::FIELD_BOX }, // 68
	{ {},{}, FIELD::FIELD_Electricity }, // 69
	{ {},{}, FIELD::FIELD_Electricity }, // 70
						 
	// ===== BOX 70 =====
	{ {},{}, FIELD::FIELD_Plant,2 }, // 71
	{ {},{}, FIELD::FIELD_Plant,2 }, // 72
	{ {},{}, FIELD::FIELD_Electricity }, // 73
	{ {},{}, FIELD::FIELD_Electricity,3 },// 74
	{ {},{}, FIELD::FIELD_Plant,1 }, // 75
	{ {},{}, FIELD::FIELD_Plant,3 }, // 76
	{ {},{}, FIELD::FIELD_Plant,2 }, // 77
	{ {},{}, FIELD::FIELD_Plant,1 }, // 78
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 79
	{ {},{}, FIELD::FIELD_Plant,2 }, // 80
						  
	// ===== BOX 80 ===== 
	{ {},{}, FIELD::FIELD_Plant,1 }, // 81
	{ {},{}, FIELD::FIELD_Plant,2 }, // 82
	{ {},{}, FIELD::FIELD_Plant,3 }, // 83
	{ {},{}, FIELD::FIELD_Electricity ,4}, // 84
	{ {},{}, FIELD::FIELD_Plant,2 }, // 85
	{ {},{}, FIELD::FIELD_Plant,2 }, // 86
	{ {},{}, FIELD::FIELD_Plant,3 }, // 87
	{ {},{}, FIELD::FIELD_Glass },   // 88
	{ {},{}, FIELD::FIELD_Plant,2 }, // 89
	{ {},{}, FIELD::FIELD_Plant,2 }, // 90
	 					  
	// ===== BOX 90 ===== 
	{ {},{}, FIELD::FIELD_Plant,2 }, // 91
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 92
	{ {},{}, FIELD::FIELD_Plant,2 }, // 93
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 94
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 95
	{ {},{}, FIELD::FIELD_Concrete ,2}, // 96
	{ {},{}, FIELD::FIELD_Plant,3 }, // 97
	{ {},{}, FIELD::FIELD_Plant,2 }, // 98
	{ {},{}, FIELD::FIELD_Plant,2 }, // 99
	{ {},{}, FIELD::FIELD_Concrete,1 }, // 100
						  
	// ===== BOX 100 =====
	{ {},{}, FIELD::FIELD_Plant,2}, // 101
	{ {},{}, FIELD::FIELD_Concrete,2 }, // 102  ãE¾ã®ã¾ã¾ã ã¨ã“ã“ã¾ã§ã—ã‹ãƒ¢ãƒEƒ«ãŒç½®ã‘ãªãE
	{ {},{}, FIELD::FIELD_BOX }, // 103
	{ {},{}, FIELD::FIELD_BOX }, // 104
	{ {},{}, FIELD::FIELD_BOX }, // 105
	{ {},{}, FIELD::FIELD_BOX }, // 106
	{ {},{}, FIELD::FIELD_BOX }, // 107
	{ {},{}, FIELD::FIELD_BOX }, // 108
	{ {},{}, FIELD::FIELD_BOX }, // 109
	{ {},{}, FIELD::FIELD_BOX }, // 110
						  
	// ===== BOX 110 ==== BOX
	{ {},{}, FIELD::FIELD_BOX }, // 111
	{ {},{}, FIELD::FIELD_BOX }, // 112
	{ {},{}, FIELD::FIELD_BOX }, // 113
	{ {},{}, FIELD::FIELD_BOX }, // 114
	{ {},{}, FIELD::FIELD_BOX }, // 115
	{ {},{}, FIELD::FIELD_BOX }, // 116
	{ {},{}, FIELD::FIELD_BOX }, // 117
	{ {},{}, FIELD::FIELD_BOX }, // 118
	{ {},{}, FIELD::FIELD_BOX }, // 119
	{ {},{}, FIELD::FIELD_BOX }, // 120

	// ===== çµ‚äºEEãƒ¼ã‚«ãƒ¼Eˆã‚«ã‚¦ãƒ³ãƒˆã—ãªãE¼E====
	{ XMFLOAT3(2.0f,-1.0f,5.0f), {}, FIELD::FIELD_MAX }
};


//======================================================
//	åˆæœŸåŒ–é–¢æ•°
//======================================================
void Field_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	char modelPath[256];
	snprintf(modelPath, sizeof(modelPath), "asset\\model\\%s.fbx", g_ModelName[1]);

	Test = ModelLoad(modelPath);//ãƒEƒãƒE‚°

	// é…åEè¦ç´ æ•°Eˆçµ‚äºEEãƒ¼ã‚«ãƒ¼ FIELD_MAX ã‚’å«ã¾ãªãE¼E
	int count = GetFieldObjectCount();
	if (count <= 1)
	{
		g_pDevice = pDevice;
		g_pContext = pContext;
		Building_Initialize(pDevice, pContext);
		return;
	}

	// ====== å…­è§’æ ¼å­å€™è£œã‚’å¤šæ•°ç”ŸæEã—ã€ä¸­å¿E«è¿‘ã„ã‚‚ãEã‹ã‚‰ N å€‹é¸ã‚“ã§ã€Œã‚ˆã‚ŠåEå½¢ã€ã«é…ç½® ======
	// MAPDATA::radius ã‚Ehex sizeEEenter->cornerE‰ã¨è¦‹ãªã™ï¼Elat-topEE
	const float size = Map->radius;
	const float sqrt3 = sqrtf(3.0f);

	// æ¨ªæ–¹å‘ã‚¹ã‚±ãƒ¼ãƒ«Eˆå¿E¦ãªã‚‰èª¿æ•´EE
	const float horizontalScale = 1.0f;

	// å€™è£œã‚’ç”ŸæEã™ã‚‹ãŸã‚ã®ãƒªãƒ³ã‚°æ•°Eˆä½™è£•ã‚’æŒãŸã›ã‚‹EE
	// count å€‹ã‚’ä¸¸ãé¸ã¶ãŸã‚ã€å€™è£œãEå¤šå°‘å¤šã‚ã«ç”ŸæEã™ã‚‹EEarginFactorEE
	const float marginFactor =5.0f; // 1.0 = æœ€ä½é™, 1.25 = ä½™è£E25%
	int rings = 1;
	while (1 + 3 * rings * (rings + 1) < static_cast<int>(count * marginFactor))
		++rings;

	// è»¸åº§æ¨Eq,r)ã‚’åŒå¿Eƒªãƒ³ã‚°ã§ç”ŸæEEEotalCandidates >= countEE
	int totalCandidates = 1 + 3 * rings * (rings + 1);

	// ãƒ˜ãƒ«ãƒ‘ãEæ§‹é€ ä½“ï¼ˆãƒ­ãƒ¼ã‚«ãƒ«EE
	struct Candidate { int q; int r; float wx; float wz; float dist; };

	// å‹•çš„ç¢ºä¿ï¼ˆãƒ­ãƒ¼ã‚«ãƒ«ã« vector ã‚’ä½¿ã‚ãªãE½¢ã«ã—ã¦ã‚¤ãƒ³ã‚¯ãƒ«ãƒ¼ãƒ‰ä¸è¦ã«EE
	Candidate* candidates = new Candidate[totalCandidates];

	// ä¸­å¿E
	int idx = 0;
	candidates[idx].q = 0;
	candidates[idx].r = 0;
	candidates[idx].wx = 0.0f;
	candidates[idx].wz = 0.0f;
	candidates[idx].dist = 0.0f;
	++idx;

	// 6æ–¹å‘ãEã‚¯ãƒˆãƒ«EExial coordsEE
	const int dirQ[6] = { 1, 1, 0, -1, -1, 0 };
	const int dirR[6] = { 0, -1, -1, 0, 1, 1 };

	for (int k = 1; idx < totalCandidates; ++k)
	{
		int q = -k;
		int r = k;
		for (int side = 0; side < 6 && idx < totalCandidates; ++side)
		{
			for (int step = 0; step < k && idx < totalCandidates; ++step)
			{
				float wx = size * 1.5f * static_cast<float>(q) * horizontalScale;
				float wz = size * sqrt3 * (static_cast<float>(r) + static_cast<float>(q) * 0.5f);
				float d = sqrtf(wx * wx + wz * wz);

				candidates[idx].q = q;
				candidates[idx].r = r;
				candidates[idx].wx = wx;
				candidates[idx].wz = wz;
				candidates[idx].dist = d;
				++idx;

				q += dirQ[side];
				r += dirR[side];
			}
		}
	}

	// ä¸­å¿E«è¿‘ã„é E« count å€‹ã‚’é¸ã¶Eˆç°¡æ˜“é¸æŠã‚½ãƒ¼ãƒˆãƒ©ã‚¤ã‚¯EE
	// é¸æŠæ•° N = countEEap é…åEã®è¦ç´ æ•°EE
	int N = count;
	// å®‰åEç­E N ãŒå€™è£œæ•°ã‚’è¶EˆãªãE‚ˆãE«
	if (N > totalCandidates) N = totalCandidates;

	// éƒ¨åˆE¸æŠï¼šåEé ­ N ä»¶ã‚’åEæœŸé¸æŠã—EŒæ®‹ã‚Šã‚’èµ°æŸ»ã—ã¦ã‚ˆã‚Šè¿‘ã‘ã‚ŒãEå…¥ã‚Œæ›¿ãˆã‚‹EE(M*N)ã ãŒå€™è£œãEãã“ã¾ã§å¤§ãããªãE¼E
	// ã¾ãšåEé ­ N ã‚Eselected ã¨ã™ã‚‹EˆéEåˆ—åEæ“ä½œï¼E
	Candidate* selected = new Candidate[N];
	for (int i = 0; i < N; ++i) selected[i] = candidates[i];

	// ç¾åœ¨ã®æœ€é ã‚¤ãƒ³ãƒEƒƒã‚¯ã‚¹ã‚’æ±‚ã‚ã‚‹é–¢æ•°
	auto findWorstIndex = [&](int limit) -> int {
		int worst = 0;
		float maxd = selected[0].dist;
		for (int j = 1; j < limit; ++j)
		{
			if (selected[j].dist > maxd)
			{

				maxd = selected[j].dist;
				worst = j;
			}
		}
		return worst;
		};

	int worstIdx = findWorstIndex(N);

	// æ®‹ã‚Šå€™è£œã‚’æ¤œæŸ»
	for (int i = N; i < totalCandidates; ++i)
	{
		if (candidates[i].dist < selected[worstIdx].dist)
		{
			// ç½®æE
			selected[worstIdx] = candidates[i];
			// worstIndex ã‚’åEè¨ˆç®E
			worstIdx = findWorstIndex(N);
		}
	}

	// ã“ã“ã§ selected[] ã¯ä¸­å¿E«è¿‘ã„ N å€‹ãEå€™è£œï¼ˆãŸã ã—é EºãEä»»æ„ï¼‰ãªã®ã§ã€ä¸­å¿E«è¿‘ã„é E«ä¸¦ã¹æ›¿ãˆã‚‹ã“ã¨ã§è¦‹ãŸç›®ãŒã‚ˆã‚ŠèEç„¶ã«
	// ç°¡æ˜“çš„ã«ãƒãƒ–ãƒ«ã‚½ãƒ¼ãƒˆï¼E ãŒå°ã•ãEEã§ååEEE
	for (int a = 0; a < N - 1; ++a)
	{
		for (int b = 0; b < N - 1 - a; ++b)
		{
			if (selected[b].dist > selected[b + 1].dist)
			{
				Candidate tmp = selected[b];
				selected[b] = selected[b + 1];
				selected[b + 1] = tmp;
			}
		}
	}

	// Map é…åEã¸å‰²ã‚Šå½“ã¦Ešä¸­å¿E«è¿‘ã„é E«é…ç½®ã—ã¦ãE
	int assign = 0;
	for (int i = 0; i < count; ++i)
	{
		if (Map[i].no == FIELD::FIELD_MAX) break;

		if (assign < N)
		{
			Map[i].pos.x = selected[assign].wx;
			Map[i].pos.z = selected[assign].wz;
			Map[i].pos.y = -1.0f;
			Map[i].isActive = true;
			++assign;
		}
		else
		{
			// å‰²ã‚Šå½“ã¦ã‚‰ã‚Œãªã‹ã£ãŸæ®‹ã‚Šã¯éè¡¨ç¤º
			Map[i].isActive = false;
		}
	}

	// è§£æ”¾
	delete[] candidates;
	delete[] selected;

	// ====================================================

	g_pDevice = pDevice;
	g_pContext = pContext;

	// --------------------------------------------------------------------
	// è¤E•°ã®ãƒE‚¯ã‚¹ãƒãƒ£ã‚’èª­ã¿è¾¼ã¿
	// --------------------------------------------------------------------
	for (int i = 0; i < FIELD_TEX_MAX; ++i) // å®šç¾©ã—ãŸãƒE‚¯ã‚¹ãƒãƒ£ã®æ•°ã ã‘ãƒ«ãƒ¼ãƒE
	{
		TexMetadata metadata;
		ScratchImage image;
		// é…åEã«å®šç¾©ã—ãŸãƒ‘ã‚¹ã‹ã‚‰ãƒE‚¯ã‚¹ãƒãƒ£ã‚’èª­ã¿è¾¼ã‚€
		LoadFromWICFile(g_TexturePaths[i], WIC_FLAGS_NONE, &metadata, image);
		CreateShaderResourceView(g_pDevice, image.GetImages(),
			image.GetImageCount(), metadata, &g_Texture[i]);
		assert(g_Texture[i]);
	}
	// --------------------------------------------------------------------

	Building_Initialize(pDevice, pContext);
}

//======================================================
//	çµ‚äºEEçE–¢æ•°
//======================================================
void Field_Finalize(void)
{
	ModelRelease(Test);

	//SAFE_RELEASE(g_VertexBuffer);
	//SAFE_RELEASE(g_IndexBuffer);
	for (int i = 0; i < FIELD_TEX_MAX; ++i) SAFE_RELEASE(g_Texture[i]);

	Building_Finalize();
}


//======================================================
//	æç”»é–¢æ•°
//======================================================
void Field_Draw(bool s_IsKonamiCodeEntered)
{
	static bool input2 = false;

	// ãƒEƒãƒE‚°ã‚­ãƒ¼
	if (s_IsKonamiCodeEntered)
	{
		if (Keyboard_IsKeyDownTrigger(KK_D2))
		{
			input2 = !input2;
		}
	}

	// ã‚·ã‚§ãƒ¼ãƒ€ãƒ¼é–‹å§E
	Shader_Begin();
	Shader_SetColor(color::white);

	// s—ñæ“¾
	XMMATRIX projection = GetProjectionMatrix();
	XMMATRIX view = GetViewMatrix();
	XMMATRIX VP = view * projection;

	int i = 0;

	// ======================================================
	// ƒtƒB[ƒ‹ƒh•`‰æ
	// ======================================================
	while (Map[i].no != FIELD_MAX)
	{
		if (!Map[i].isActive)
		{
			i++;
			continue;
		}

		// ------------------------------
		// ƒ[ƒ‹ƒhs—ñì¬
		// ------------------------------
		XMMATRIX ScalingMatrix = XMMatrixScaling(1.0f, 1.0f, 1.0f);

		XMMATRIX RotationMatrix = XMMatrixRotationRollPitchYaw(
			XMConvertToRadians(-90.0f),
			0.0f,
			0.0f
		);

		XMMATRIX TranslationMatrix = XMMatrixTranslation(
			Map[i].pos.x,
			Map[i].pos.y,
			Map[i].pos.z
		);

		XMMATRIX World = ScalingMatrix * RotationMatrix * TranslationMatrix;
		XMMATRIX WVP = World * VP;

		Shader_SetWorldMatrix(World);
		Shader_SetMatrix(World * VP);

		// ------------------------------
		// í—Ş‚²‚Æ‚ÉƒeƒNƒXƒ`ƒƒØ‚è‘Ö‚¦
		// ------------------------------
		int texIndex = 0; // ƒfƒtƒHƒ‹ƒg

		switch (Map[i].no)
		{
		case FIELD::FIELD_Plant:
			texIndex = 2;
			break;

		case FIELD::FIELD_Concrete:
			texIndex = 0;
			break;

		case FIELD::FIELD_Glass:
			texIndex = 0;
			break;

		case FIELD::FIELD_Electricity:
			texIndex = 0;
			break;

		case FIELD::FIELD_BOX:
			texIndex = 3;
			break;

		default:
			texIndex = 0;
			break;
		}

		//// ƒeƒNƒXƒ`ƒƒ‚ğƒpƒCƒvƒ‰ƒCƒ“‚©‚ç‰ğœ
		ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
		g_pContext->PSSetShaderResources(0, 1, &g_Texture[1]);
		//------------------------------------------------
		i++;
	}

	///////////////////////////////////////
	// TODO:boundingBox‚ğQÆ‚µ‚½‚¢
	
	// --- 3. ƒfƒoƒbƒO•`‰æ‚Í‘S•”‚Ìƒ}ƒbƒv‚ğ•`‚«I‚í‚Á‚½Œã‚Éu1‰ñ‚¾‚¯v‚â‚é ---
	if (s_IsKonamiCodeEntered)
	{
		SetBlendState(BLENDSTATE_NONE);
		SetDepthTest(false); // d‚È‚è‚ğ–³‹‚µ‚ÄŒ©‚¦‚é‚æ‚¤‚É
		Shader_SetMatrix(VP); // ƒ[ƒ‹ƒhs—ñ‚ÍIdentity‚É‚·‚é‚Ì‚ÅVP‚¾‚¯‚ÅOK

		// ƒtƒB[ƒ‹ƒhƒIƒuƒWƒFƒNƒg‚Ì˜ZŠp’Œ
		int fieldCount = GetFieldObjectCount();
		MAPDATA* fieldObjects = GetFieldObjects();
		for (int j = 0; j < fieldCount; ++j)
		{
			if (!fieldObjects[j].isActive) continue;
			Debug_DrawHex(Map[j].boundingBox, XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
		}

		// ƒvƒŒƒCƒ„[‚ÌƒXƒyƒVƒƒƒ‹”ÍˆÍi‰~j
		for (int p = 0; p < PLAYER_MAX; ++p)
		{
			PLAYEROBJECT* player = GetPlayer(p);
			if (!player->active || !player->useSpecial) continue;

			// ‚±‚±‚ÉŠeƒ^ƒCƒv‚ÌDebug_DrawCircleˆ—‚ğ‚Ü‚Æ‚ß‚é
			// (‚³‚Á‚«‚ÌƒR[ƒh‚Ì player.type ‚²‚Æ‚Ì”»’è‚ğ“ü‚ê‚é)

			// A•¨EƒRƒ“ƒNƒŠ[ƒg‚ÌƒXƒyƒVƒƒƒ‹‚ªg—p‚³‚ê‚Ä‚¢‚éê‡A‰~‚ÌƒtƒŒ[ƒ€‚ğÔF‚Å•\¦

			for (int p = 0; p < PLAYER_MAX; ++p)
			{
				PLAYEROBJECT* playerObject = GetPlayer(p);
				PLAYEROBJECT& player = *playerObject;
				if (!player.useSpecial) continue;
				// A•¨EƒRƒ“ƒNƒŠ[ƒg‚ÌƒXƒyƒVƒƒƒ‹

				if (player.type == PlayerType::Plant || player.type == PlayerType::Concrete)
				{
					// ‰~‚Ì’†S‚Æ”¼Œa‚ğİ’è
					XMFLOAT3 center = playerObject->position;
					float radius = 5.0f;

					// ÔF‚Å‰~‚ğ•`‰æ
					Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
				}

				// “d‹C‚ÌƒXƒyƒVƒƒƒ‹
				if (player.type == PlayerType::Electricity)
				{
					for (int i = 0; i < SPECIAL_ELECTRICITY_QUANTITY; ++i)
					{
						// “d‹C‚Ì‰~‚Ì’†S‚Æ”¼Œa‚ğæ“¾
						XMFLOAT3 center = player.electricityCircles[i].center;
						float radius = player.electricityCircles[i].radius;

						// ÔF‚Å‰~‚ğ•`‰æ
						Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
					}
				}

				// ƒKƒ‰ƒX‚ÌƒXƒyƒVƒƒƒ‹
				if (player.type == PlayerType::Glass)
				{
					for (const auto& box : player.glassBoxes)
					{
						// ƒKƒ‰ƒX‚Ì‰~‚Ì’†S‚Æ”¼Œa‚ğİ’è
						XMFLOAT3 center = box.position;
						float radius = 0.3f; // ”¼Œa0.3‚Ì‰~

						// ÔF‚Å‰~‚ğ•`‰æ
						Debug_DrawCircle(center, radius, XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
					}
				}
			}
		}
	}
}

//======================================================
//	æ›´æ–°å‡¦çE
//======================================================
void Field_Update(void)
{
	int i = 0;
	while (Map[i].no != FIELD_MAX)
	{
		// ã‚‚ã—ã‚¢ã‚¯ãƒE‚£ãƒ–ã˜ã‚Eªã‹ã£ãŸã‚‰ã€æç”»ã—ãªãE§æ¬¡ã¸
		if (!Map[i].isActive)
		{
			i++; // i ã‚’é€²ã‚ã‚‹ã®ã‚’å¿˜ã‚ŒãªãE§EE
			continue; // ã“ãEå…ˆãEæç”»å‡¦çE‚’ã‚¹ã‚­ãƒEE
		}

		Map[i].boundingBox.center = Map[i].pos;			// -1
		Map[i].boundingBox.radius = Map[i].radius;		// 1
		Map[i].boundingBox.height = Map[i].height;		// 3.0



		i++;
	}
}

// ======================================================
//	ã‚²ãƒE‚¿ãƒ¼
// ------------------------------------------------------
//	ãƒ•ã‚£ãƒ¼ãƒ«ãƒ‰ãEé…åEã®å…ˆé ­ãƒã‚¤ãƒ³ã‚¿ã‚’è¿”ã™
// ======================================================
MAPDATA* GetFieldObjects()
{
	return Map;
}

// ãƒ•ã‚£ãƒ¼ãƒ«ãƒ‰ã‚ªãƒ–ã‚¸ã‚§ã‚¯ãƒˆãEç·æ•°ã‚’è¿”ã™
int GetFieldObjectCount()
{
	int count = 0;
	// mapé…åEã¯FIELD_MAXã‚’çµ‚äºEEãƒ¼ã‚«ãƒ¼ã¨ã—ã¦ãE‚‹
	while (Map[count].no != FIELD_MAX)
	{
		count++;
	}
	return count;
}


