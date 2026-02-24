//======================================================
//	ResourceCache.cpp
//======================================================
#include "ResourceCache.h"

// 静的メンバーの定義
std::unordered_map<std::wstring, ID3D11ShaderResourceView*> ResourceCache::s_TextureCache;
std::unordered_map<std::wstring, MODEL*> ResourceCache::s_ModelCache;

void ResourceCache::Initialize()
{
    s_TextureCache.clear();
    s_ModelCache.clear();
}

void ResourceCache::Finalize()
{
    ClearAll();
}

void ResourceCache::AddTexture(const std::wstring& path, ID3D11ShaderResourceView* pSRV)
{
    if (pSRV == nullptr) return;

    // 既に存在する場合は古いものを解放
    auto it = s_TextureCache.find(path);
    if (it != s_TextureCache.end() && it->second != nullptr)
    {
        it->second->Release();
    }

    s_TextureCache[path] = pSRV;
}

ID3D11ShaderResourceView* ResourceCache::GetTexture(const std::wstring& path)
{
    auto it = s_TextureCache.find(path);
    if (it != s_TextureCache.end())
    {
        return it->second;
    }
    return nullptr;
}

bool ResourceCache::HasTexture(const std::wstring& path)
{
    return s_TextureCache.find(path) != s_TextureCache.end();
}

void ResourceCache::AddModel(const std::wstring& path, MODEL* pModel)
{
    if (pModel == nullptr) return;

    // 既に存在する場合は古いものを解放
    auto it = s_ModelCache.find(path);
    if (it != s_ModelCache.end() && it->second != nullptr)
    {
        ModelRelease(it->second);
    }

    s_ModelCache[path] = pModel;
}

MODEL* ResourceCache::GetModel(const std::wstring& path)
{
    auto it = s_ModelCache.find(path);
    if (it != s_ModelCache.end())
    {
        return it->second;
    }
    return nullptr;
}

bool ResourceCache::HasModel(const std::wstring& path)
{
    return s_ModelCache.find(path) != s_ModelCache.end();
}

void ResourceCache::ClearSceneResources(const std::wstring& scenePrefix)
{
    // テクスチャの解放
    for (auto it = s_TextureCache.begin(); it != s_TextureCache.end(); )
    {
        if (it->first.find(scenePrefix) == 0)  // パスが指定のプレフィックスで始まる
        {
            if (it->second) it->second->Release();
            it = s_TextureCache.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // モデルの解放
    for (auto it = s_ModelCache.begin(); it != s_ModelCache.end(); )
    {
        if (it->first.find(scenePrefix) == 0)
        {
            if (it->second) ModelRelease(it->second);
            it = s_ModelCache.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void ResourceCache::ClearAll()
{
    // 全テクスチャを解放
    for (auto& pair : s_TextureCache)
    {
        if (pair.second) pair.second->Release();
    }
    s_TextureCache.clear();

    // 全モデルを解放
    for (auto& pair : s_ModelCache)
    {
        if (pair.second) ModelRelease(pair.second);
    }
    s_ModelCache.clear();
}