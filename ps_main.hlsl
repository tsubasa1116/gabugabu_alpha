/*==============================================================================

   2D描画用ピクセルシェーダー [shader_pixel_2d.hlsl]
--------------------------------------------------------------------------------
==============================================================================*/

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

cbuffer cbColor : register(b1)
{
    float4 setColor; // 乗算用カラー
    float4 lerpColor; // 線形補間用カラー
    float lerpFactor; // 補間係数 (0.0 = 元の色, 1.0 = lerpColor)
    float3 pad; 
};

struct PS_INPUT
{
    float4 posH : SV_POSITION; //ピクセルの座標
    float4 color : COLOR0; //ピクセルの色
    float2 texcoord : TEXCOORD0;
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    float4 col;
    col = g_Texture.Sample(g_SamplerState, ps_in.texcoord);
	
	// 乗算カラー
    col *= ps_in.color * setColor;
	
	// 線形補間カラー
    col.rgb = lerp(col.rgb, lerpColor.rgb, lerpFactor);
	
    return col;
}
