//======================================================
//	ResourceCache.h
//  読み込んだリソースを管理するキャッシュシステム
//======================================================
#pragma once

#include <d3d11.h>
#include <unordered_map>
#include <string>
#include "model.h"

class ResourceCache
{
public:
    static void Initialize();
    static void Finalize();

    // テクスチャの追加・取得
    static void AddTexture(const std::wstring& path, ID3D11ShaderResourceView* pSRV);
    static ID3D11ShaderResourceView* GetTexture(const std::wstring& path);
    static bool HasTexture(const std::wstring& path);

    // モデルの追加・取得
    static void AddModel(const std::wstring& path, MODEL* pModel);
    static MODEL* GetModel(const std::wstring& path);
    static bool HasModel(const std::wstring& path);

    // 特定シーンのリソースを解放
    static void ClearSceneResources(const std::wstring& scenePrefix);

    // 全リソースを解放
    static void ClearAll();

private:
    static std::unordered_map<std::wstring, ID3D11ShaderResourceView*> s_TextureCache;
    static std::unordered_map<std::wstring, MODEL*> s_ModelCache;
};