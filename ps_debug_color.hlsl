cbuffer cbColor : register(b1)
{
    float4 setColor; // C++側で Shader_SetColor() で設定した色
};

struct PS_INPUT
{
    float4 posH : SV_POSITION;
    float4 color : COLOR0; // 頂点バッファに含まれる色情報
};

float4 main(PS_INPUT ps_in) : SV_TARGET
{
    // 頂点の色と定数バッファの色を乗算してそのまま返す（テクスチャなし）
    float4 finalColor = ps_in.color * setColor;
    return finalColor;
}