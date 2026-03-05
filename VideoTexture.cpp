#include "VideoTexture.h"
#include <cassert>

VideoTexture::VideoTexture() {}

VideoTexture::~VideoTexture()
{
	Finalize();
}

bool VideoTexture::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const wchar_t* filePath)
{
	m_pContext = pContext;

	// MediaFoundationの初期化
	// 動画デコードに必要なライブラリを起動する
	HRESULT hr = MFStartup(MF_VERSION);
	if (FAILED(hr)) return false;

	// ソースリーダーの作成
	// ファイルから動画フレームを読み取るためのオブジェクト
	IMFAttributes* pAttributes = nullptr;
	MFCreateAttributes(&pAttributes, 1);

	// ビデオ処理を有効化
	pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);

	hr = MFCreateSourceReaderFromURL(filePath, pAttributes, &m_pReader);
	pAttributes->Release();
	if (FAILED(hr)) return false;

	// DirectXのテクスチャの形式に変換させる
	IMFMediaType* pMediaType = nullptr;
	MFCreateMediaType(&pMediaType);
	pMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	pMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
	hr = m_pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, nullptr, pMediaType);
	pMediaType->Release();
	if (FAILED(hr)) return false;

	// 動画のサイズを取得
	IMFMediaType* pCurrentType = nullptr;
	m_pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pCurrentType);
	UINT64 frameSize = 0;
	pCurrentType->GetUINT64(MF_MT_FRAME_SIZE, &frameSize);
	m_Width = (UINT)(frameSize >> 32);        // 上位32ビット = 幅
	m_Height = (UINT)(frameSize & 0xFFFFFFFF); // 下位32ビット = 高さ
	pCurrentType->Release();

	// DirectXテクスチャの作成
	// 動画フレームを格納するためのテクスチャ
	D3D11_TEXTURE2D_DESC texDesc = {};
	texDesc.Width = m_Width;
	texDesc.Height = m_Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.Usage = D3D11_USAGE_DYNAMIC;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = pDevice->CreateTexture2D(&texDesc, nullptr, &m_pTexture);
	if (FAILED(hr)) return false;

	// シェーダーリソースビュー（SRV）の作成
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	hr = pDevice->CreateShaderResourceView(m_pTexture.Get(), &srvDesc, &m_pSRV);
	if (FAILED(hr)) return false;

	// 再生制御用の初期化
	m_StartTime = std::chrono::steady_clock::now();   // 再生開始時刻を記録
	m_LastTimestamp = -1;                              // 最後に表示したフレームのタイムスタンプ
	m_PlaybackSpeed = 1.0f;
	m_LoopSkipTime = 0.0;

	m_Initialized = true;
	m_Finished = false;
	return true;
}

void VideoTexture::Finalize()
{
	m_pSRV.Reset();
	m_pTexture.Reset();
	m_pReader.Reset();

	if (m_Initialized)
	{
		MFShutdown();  // MediaFoundation終了処理
		m_Initialized = false;
	}
}

bool VideoTexture::Update()
{
	if (!m_pReader || m_Finished) return false;

	// 現在の再生位置を計算
	// 再生開始からの経過時間 × 再生速度 = 動画内の再生位置
	auto now = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = now - m_StartTime;

	// 再生位置を100ナノ秒単位に変換する（MediaFoundationのタイムスタンプ単位）
	LONGLONG targetTime = static_cast<LONGLONG>(elapsed.count() * m_PlaybackSpeed * 10000000.0);

	// 目標時刻に達するまでフレームを読み飛ばす
	// 既に表示済みの時刻より前なら何もしない
	if (targetTime <= m_LastTimestamp)
	{
		// まだ次のフレームの時刻になっていない
		return true;
	}

	// フレームを読み込む（目標時刻を超えるまでループ）
	IMFSample* pSampleToShow = nullptr;  // 最終的に表示するフレーム
	LONGLONG timestampToShow = 0;

	while (true)
	{
		DWORD streamIndex, flags;
		LONGLONG timestamp;
		IMFSample* pSample = nullptr;

		// 次のフレームを読み込む
		HRESULT hr = m_pReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0,              // フラグ
			&streamIndex,   // 読み込んだストリームのインデックス
			&flags,         // 状態フラグ
			&timestamp,     // フレームのタイムスタンプ
			&pSample);      // フレームデータ

		// 動画の終端に達した
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			m_Finished = true;
			if (pSample) pSample->Release();
			if (pSampleToShow) pSampleToShow->Release();
			return false;
		}

		if (pSample)
		{
			// 前回保持していたサンプルを解放
			if (pSampleToShow)
			{
				pSampleToShow->Release();
			}

			// 今回のサンプルを保持
			pSampleToShow = pSample;
			timestampToShow = timestamp;
			// 目標時刻を超えたらループを抜ける
			if (timestamp >= targetTime)
			{
				break;
			}
			// まだ目標時刻に達していない場合は次のフレームを読む
		}
		else
		{
			// フレームがない場合（デコード待ちなど）
			break;
		}
	}

	// フレームをテクスチャに書き込む
	if (pSampleToShow)
	{
		WriteFrameToTexture(pSampleToShow);
		m_LastTimestamp = timestampToShow;  // 表示したフレームのタイムスタンプを記録
		pSampleToShow->Release();
	}

	return true;
}

void VideoTexture::WriteFrameToTexture(IMFSample* pSample)
{
	// サンプルからバッファを取得
	IMFMediaBuffer* pBuffer = nullptr;
	pSample->ConvertToContiguousBuffer(&pBuffer);

	BYTE* pData = nullptr;
	DWORD maxLen, curLen;
	pBuffer->Lock(&pData, &maxLen, &curLen);

	// テクスチャにピクセルデータをコピー
	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = m_pContext->Map(m_pTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (SUCCEEDED(hr))
	{
		UINT srcPitch = m_Width * 4;  // 1行のバイト数（RGBA = 4バイト）
		BYTE* pDst = (BYTE*)mapped.pData;
		BYTE* pSrc = pData;

		// 1行ずつコピー
		for (UINT y = 0; y < m_Height; y++)
		{
			memcpy(pDst, pSrc, srcPitch);
			pDst += mapped.RowPitch;  // テクスチャ側の行ピッチ
			pSrc += srcPitch;         // ソース側の行ピッチ
		}
		m_pContext->Unmap(m_pTexture.Get(), 0);
	}

	pBuffer->Unlock();
	pBuffer->Release();
}

void VideoTexture::Reset()
{
	if (m_pReader)
	{
		// 動画の再生位置を先頭に戻す
		PROPVARIANT var;
		PropVariantInit(&var);
		var.vt = VT_I8;
		var.hVal.QuadPart = 0;  // 0 = 先頭
		m_pReader->SetCurrentPosition(GUID_NULL, var);
		PropVariantClear(&var);

		// 再生制御をリセット
		m_StartTime = std::chrono::steady_clock::now();
		m_LastTimestamp = -1;
		m_Finished = false;
	}
}

void VideoTexture::ResetForLoop()
{
	if (m_pReader)
	{
		// スキップ時間を100ナノ秒単位に変換
		LONGLONG skipTime = static_cast<LONGLONG>(m_LoopSkipTime * 10000000.0);

		// 動画の再生位置をスキップ時間の位置に設定
		PROPVARIANT var;
		PropVariantInit(&var);
		var.vt = VT_I8;
		var.hVal.QuadPart = skipTime;
		m_pReader->SetCurrentPosition(GUID_NULL, var);
		PropVariantClear(&var);

		// 再生制御をリセット（スキップ分を考慮）
		m_StartTime = std::chrono::steady_clock::now();
		m_LastTimestamp = skipTime;
		m_Finished = false;
	}
}