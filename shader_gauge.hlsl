cbuffer cbGauge : register(b3)
{
    float fire;
    float water;
    float wind;
    float earth;

    float4 fireColor;
    float4 waterColor;
    float4 windColor;
    float4 earthColor;
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
    float total = fire + water + wind + earth;
    if (total <= 0)
        return float4(0, 0, 0, 1); // 全部0なら黒丸

    // 終端角度
    float endFire = fire / total;
    float endWater = endFire + water / total;
    float endWind = endWater + wind / total;
    float endEarth = 1.0;

    // 区間判定
    if (angle < endFire)
        return fireColor;
    if (angle < endWater)
        return waterColor;
    if (angle < endWind)
        return windColor;
    return earthColor;
}
