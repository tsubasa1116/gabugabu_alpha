Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
    float4 posH : SV_POSITION; //ピクセルの座標
    float4 color : COLOR0; //ピクセルの色
    float2 uv : TEXCOORD0;
};

static const float blueOffset = 0.0005f;

float4 main(PS_INPUT input):SV_Target
{
    float4 col = 0;
    
    for (float x = -1; x <= 1; x++)
    {
        for (float y = -1; y <= 1; y++)
        {
            float2 offset = float2(x, y) * blueOffset;
            col += g_Texture.Sample(g_Sampler, input.uv + offset);
        }
    }
    
    col /= 9.0f;
    
    return col * input.color;
}