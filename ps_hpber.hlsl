Texture2D g_Texture : register(t0);
SamplerState g_Sampler : register(s0);

cbuffer cbChecker : register(b5)
{
    float4 palam;
	float4 colorA;
	float4 colorB;
};

struct PS_INPUT
{
	float4 posH : SV_POSITION; //ピクセルの座標
	float4 color : COLOR0; //ピクセルの色
	float2 uv : TEXCOORD0;
};

float4 main(PS_INPUT i) : SV_TARGET
{
    float time = palam.x;
    float alpha = palam.y;
    float speed = palam.z;
	
    float4 colA = colorA;
    float4 colB = colorB;
	
	//=======================
	// 市松模様（っぽい）の描画
	//=======================
	
	// テクスチャの色を取得
    float4 base = g_Texture.Sample(g_Sampler, i.uv) * i.color;
	
	// uv内での位置を20倍に拡大する
	float2 p = i.uv * 20.0;
	// 時間経過で動かす（速度を掛けて調整）
	p += time.x * speed;

	// sin関数で波を作る
	// 3.14（π）を掛けて、-1から1の間で波が1周期するように調整する（20uv/2π）
	// x軸方向 = 縦縞、y軸方向 = 横縞 により、かけ合わせて市松模様を再現する
    float v = sin(p.x * 3.14) * sin(p.y * 3.14);
	// yの値がx以上なら2、y未満なら0を返す(step関数)
	float checker = step(0.05, v);

	// checkerの値によってcolAかcolBを選ぶ
	// lerp(a,b,t)=a(1-t)+bt -> t=0ならa、t=1ならb
	float4 check = lerp(colA, colB, checker);

	 // checkの色をoverlayに保存する（α値はテクスチャの値を保持）
    float4 overlay = { check.rgb, base.a };

	// baseとoverlayをalphaで調節
	return lerp(base, overlay, alpha);
}