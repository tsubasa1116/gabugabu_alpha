Texture2D glassTex : register(t0);
Texture2D concreteTex : register(t1);
Texture2D plantTex : register(t2);
Texture2D electricTex : register(t3);

SamplerState g_Sampler : register(s0);

cbuffer cbGauge : register(b3)
{
    float glass;
    float concrete;
    float plant;
    float electric;
};

static const float2 center = float2(0.5, 0.5);
static const float radius = 0.45;
static const float borderThickness = 0.03;

struct PS_INPUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0;
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_Target
{
    float2 uv = input.texcoord;
    float2 p = uv - center;

    float dist = length(p);
    if (dist > radius)
        discard;

    // 距離 → 枠線
    if (dist > radius - borderThickness)
        return float4(0, 0, 0, 1);

    // 角度を 0~1 に
    float angle = atan2(p.y, p.x);
    angle += 3.14 * 0.5;
    angle = -angle;
    angle = frac(angle / (2 * 3.14));

    // 合計
    float total = glass + concrete + plant + electric;
    if (total <= 0)
        return float4(0, 0, 0, 1); // 全部0なら黒丸

    // 終端角度
    float endGlass = glass / total;
    float endConcrete = endGlass + concrete / total;
    float endPlant = endConcrete + plant / total;
    float endElectric = 1.0;

    // 区間判定
    if (angle < endGlass)
        return glassTex.Sample(g_Sampler, uv);
    if (angle < endConcrete)
        return concreteTex.Sample(g_Sampler, uv);
    if (angle < endPlant)
        return plantTex.Sample(g_Sampler, uv);
    return electricTex.Sample(g_Sampler, uv);
}