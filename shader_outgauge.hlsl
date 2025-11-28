cbuffer cbGaugeOuter : register(b4)
{
    float gaugeValue;
    float3 pad;
    float4 gaugeColor;
};

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
    float2 p = uv - center;
    float dist = length(p);

    // “à‘¤‚ÌŒŠ or ŠO‘¤‚ÌŠO‚Í•`‚©‚È‚¢
    if (dist < innerRadius || dist > outerRadius)
        discard;

    // Šp“x
    float angle = atan2(p.y, p.x);
    angle += 3.14 * 0.5;
    // angle = -angle;
    angle = frac(angle / (2 * 3.14));

    if (dist > outerRadius - borderThickness)
    {
        return float4(0, 0, 0, 1); // •˜g
    }
    
    // i’»”»’è
    if (angle <= gaugeValue)
        return gaugeColor;

    return float4(0, 0, 0, 0); // –¢’B•”•ª‚Í“§–¾
}
