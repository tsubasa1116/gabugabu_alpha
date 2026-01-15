cbuffer cbGaugeOuter : register(b4)
{
    float gaugeValue;
    float3 pad;
    float4 gaugeColor;
};

Texture2D g_OutTexture : register(t0);
SamplerState g_OutSampler : register(s0);

static const float2 center = float2(0.5, 0.5);
static const float outerRadius = 0.48;
static const float innerRadius = 0.38;
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

    // テクスチャでマスク（画像のアルファが透明部を決める）
    float4 tex = g_OutTexture.Sample(g_OutSampler, uv);
    if (tex.a < 0.01f)
        discard;

    // 中心基準で角度計算（0..1 に正規化）
    float2 center = float2(0.5, 0.5);
    float2 p = uv - center;
    float angle = atan2(p.y, p.x);
    angle += 3.14159265 * 0.5;
    angle = frac(angle / (2.0 * 3.14159265));

    // 進捗内ならゲージ色、外なら透明（テクスチャのアルファを掛ける）
    if (angle <= gaugeValue)
    {
        // テクスチャの色を乗算して柔らかく（テクスチャが白ならそのまま）
        float3 rgb = gaugeColor.rgb * tex.rgb;
        float a = gaugeColor.a * tex.a;
        return float4(rgb, a);
    }

    // 未塗り部分は透明にする（リング画像自体は透明部分を持つのでここでも透明）
    return float4(0, 0, 0, 0);
}
