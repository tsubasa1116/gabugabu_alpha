cbuffer cbChecker : register(b0)
{
    float4 colorA; // 黒マスの色など
    float4 colorB; // 白マスの色など
    float2 tileCount; // 画面全体でのマス数（X,Y）
    float2 pad;
};

struct PS_INPUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_INPUT i) : SV_TARGET
{
	// uvをマス数でスケールしてどのマスか求める
    float2 scaled = i.texcoord * tileCount;

    // マスのインデックス（整数）にする
    int2 cell = (int2) floor(scaled);

    // x+yの偶奇で色を切り替える
    int checker = (cell.x + cell.y) & 1;

    return (checker == 0) ? colorA : colorB;
}