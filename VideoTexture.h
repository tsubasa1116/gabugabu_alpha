#pragma once
#include <d3d11.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <chrono>

#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

class VideoTexture
{
public:
	VideoTexture();
	~VideoTexture();

	bool Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wchar_t* filePath);
	void Finalize();
	bool Update();

	ID3D11ShaderResourceView* GetShaderResourceView() const { return m_pSRV.Get(); }
	UINT GetWidth() const { return m_Width; }
	UINT GetHeight() const { return m_Height; }
	bool IsFinished() const { return m_Finished; }
	void Reset();

	void SetPlaybackSpeed(float speed) { m_PlaybackSpeed = speed; }
	float GetPlaybackSpeed() const { return m_PlaybackSpeed; }

private:
	// テクスチャにフレームデータを書き込む
	void WriteFrameToTexture(IMFSample* pSample);

	Microsoft::WRL::ComPtr<IMFSourceReader> m_pReader;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_pTexture;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_pSRV;
	ID3D11DeviceContext* m_pContext = nullptr;

	UINT m_Width = 0;
	UINT m_Height = 0;
	bool m_Finished = false;
	bool m_Initialized = false;

	std::chrono::steady_clock::time_point m_StartTime;  // 再生開始時刻
	float m_PlaybackSpeed = 1.0f;                       // 再生速度
	LONGLONG m_LastTimestamp = 0;                       // 最後に表示したフレームのタイムスタンプ
};