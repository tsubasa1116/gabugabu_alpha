#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;
#include "direct3d.h"
#include "shader.h"
#include "sprite.h"
#include "keyboard.h"
#include "audio.h"
#include <mmsystem.h>

static IXAudio2* g_Xaudio{};
static IXAudio2MasteringVoice* g_MasteringVoice{};

void InitAudio()
{
	// XAudio生成
	HRESULT hr = XAudio2Create(&g_Xaudio, 0);
	if (FAILED(hr))
	{
		g_Xaudio = nullptr;
		return;
	}

	// マスタリングボイス生成
	hr = g_Xaudio->CreateMasteringVoice(&g_MasteringVoice);
	if (FAILED(hr))
	{
		g_MasteringVoice = nullptr;
		// leave g_Xaudio valid; caller may still call UninitAudio
	}
}

void UninitAudio()
{
	if (g_MasteringVoice)
	{
		g_MasteringVoice->DestroyVoice();
		g_MasteringVoice = nullptr;
	}
	if (g_Xaudio)
	{
		g_Xaudio->Release();
		g_Xaudio = nullptr;
	}
}

struct AUDIO
{
	IXAudio2SourceVoice*	SourceVoice{};
	BYTE*					SoundData{};

	int						Length{};
	int						PlayLength{};
};

#define AUDIO_MAX (100)
static AUDIO g_Audio[AUDIO_MAX]{};

int LoadAudio(const char *FileName)
{
	int index = -1;

	for (int i = 0; i < AUDIO_MAX; i++)
	{
		if (g_Audio[i].SourceVoice == nullptr)
		{
			index = i;
			break;
		}
	}

	if (index == -1)
		return -1;

	// サウンドデータ読込
	WAVEFORMATEX wfx = { 0 };

	HMMIO hmmio = NULL;
	MMIOINFO mmioinfo = { 0 };
	MMCKINFO riffchunkinfo = { 0 };
	MMCKINFO datachunkinfo = { 0 };
	MMCKINFO mmckinfo = { 0 };
	UINT32 buflen = 0;
	LONG readlen = 0;

	hmmio = mmioOpenA((LPSTR)FileName, &mmioinfo, MMIO_READ);
	if (!hmmio)
	{
		return -1;
	}

	// RIFF/WAVE チャンク探索
	riffchunkinfo.fccType = mmioFOURCC('W', 'A', 'V', 'E');
	if (mmioDescend(hmmio, &riffchunkinfo, NULL, MMIO_FINDRIFF) != MMSYSERR_NOERROR)
	{
		mmioClose(hmmio, 0);
		return -1;
	}

	// fmt チャンク
	mmckinfo.ckid = mmioFOURCC('f', 'm', 't', ' ');
	if (mmioDescend(hmmio, &mmckinfo, &riffchunkinfo, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		mmioClose(hmmio, 0);
		return -1;
	}

	if (mmckinfo.cksize >= sizeof(WAVEFORMATEX))
	{
		if (mmioRead(hmmio, (HPSTR)&wfx, sizeof(wfx)) != sizeof(wfx))
		{
			mmioClose(hmmio, 0);
			return -1;
		}
	}
	else
	{
		PCMWAVEFORMAT pcmwf = { 0 };
		if (mmioRead(hmmio, (HPSTR)&pcmwf, sizeof(pcmwf)) != sizeof(pcmwf))
		{
			mmioClose(hmmio, 0);
			return -1;
		}
		memset(&wfx, 0x00, sizeof(wfx));
		memcpy(&wfx, &pcmwf, sizeof(pcmwf));
		wfx.cbSize = 0;
	}
	mmioAscend(hmmio, &mmckinfo, 0);

	// data チャンク
	datachunkinfo.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if (mmioDescend(hmmio, &datachunkinfo, &riffchunkinfo, MMIO_FINDCHUNK) != MMSYSERR_NOERROR)
	{
		mmioClose(hmmio, 0);
		return -1;
	}

	buflen = datachunkinfo.cksize;
	if (buflen == 0)
	{
		mmioClose(hmmio, 0);
		return -1;
	}

	g_Audio[index].SoundData = new (std::nothrow) unsigned char[buflen];
	if (!g_Audio[index].SoundData)
	{
		mmioClose(hmmio, 0);
		return -1;
	}

	readlen = mmioRead(hmmio, (HPSTR)g_Audio[index].SoundData, buflen);
	if (readlen <= 0)
	{
		delete[] g_Audio[index].SoundData;
		g_Audio[index].SoundData = nullptr;
		mmioClose(hmmio, 0);
		return -1;
	}

	g_Audio[index].Length = static_cast<int>(readlen);
	if (wfx.nBlockAlign != 0)
		g_Audio[index].PlayLength = static_cast<int>(readlen / wfx.nBlockAlign);
	else
		g_Audio[index].PlayLength = 0;

	mmioClose(hmmio, 0);

	// サウンドソース生成
	if (!g_Xaudio)
	{
		// XAudio が初期化されていない
		delete[] g_Audio[index].SoundData;
		g_Audio[index].SoundData = nullptr;
		return -1;
	}

	HRESULT hr = g_Xaudio->CreateSourceVoice(&g_Audio[index].SourceVoice, &wfx);
	if (FAILED(hr) || g_Audio[index].SourceVoice == nullptr)
	{
		// 失敗時は確実にクリーンアップして終了
		delete[] g_Audio[index].SoundData;
		g_Audio[index].SoundData = nullptr;
		g_Audio[index].SourceVoice = nullptr;
		return -1;
	}

	return index;
}

void UnloadAudio(int Index)
{
	if (Index < 0 || Index >= AUDIO_MAX) return;
	if (g_Audio[Index].SourceVoice == nullptr) return;

	g_Audio[Index].SourceVoice->Stop();
	g_Audio[Index].SourceVoice->DestroyVoice();
	g_Audio[Index].SourceVoice = nullptr;

	delete[] g_Audio[Index].SoundData;
	g_Audio[Index].SoundData = nullptr;

	g_Audio[Index].Length = 0;
	g_Audio[Index].PlayLength = 0;
}

void PlayAudio(int Index, bool Loop)
{
	if (Index < 0 || Index >= AUDIO_MAX) return;
	if (g_Audio[Index].SourceVoice == nullptr) return;

	g_Audio[Index].SourceVoice->Stop();
	g_Audio[Index].SourceVoice->FlushSourceBuffers();

	// バッファ設定
	XAUDIO2_BUFFER bufinfo;

	memset(&bufinfo, 0x00, sizeof(bufinfo));
	bufinfo.AudioBytes = g_Audio[Index].Length;
	bufinfo.pAudioData = g_Audio[Index].SoundData;
	bufinfo.PlayBegin = 0;
	bufinfo.PlayLength = g_Audio[Index].PlayLength;

	// ループ設定
	if (Loop)
	{
		bufinfo.LoopBegin = 0;
		bufinfo.LoopLength = g_Audio[Index].PlayLength;
		bufinfo.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	g_Audio[Index].SourceVoice->SubmitSourceBuffer(&bufinfo, NULL);

	// 再生
	g_Audio[Index].SourceVoice->Start();
}



