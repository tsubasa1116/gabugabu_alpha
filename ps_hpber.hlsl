Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

cbuffer cbChecker : register(b5)
{
    float4 colorA;    // 背景色1
    float4 colorB;    // 背景色2
    float2 tileCount; // マス数
    float intensity;  // 重ねる強さ 0.乗せない, 1.完全に上書き
    float3 pad;
};

struct PS_INPUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT i) : SV_TARGET
{
    // 元のテクスチャ色
    float4 base = g_Texture.Sample(g_Sampler, i.uv) * i.color;
    
    // 市松模様の計算
    float2 scaled = i.uv * tileCount;
    int2 cell = (int2) floor(scaled);
    int checker = (cell.x + cell.y) & 1;
    
    float4 checkColor = (checker == 0) ? colorA : colorB;
    
    // base とcheckerをブレンドして薄く重ねる
    float4 result = lerp(base, base * checkColor, intensity);
    
    return result;
}
