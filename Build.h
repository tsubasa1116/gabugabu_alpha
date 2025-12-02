#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
#include "sprite.h"
#include "shader.h"
#include "collider.h"
#include "Building.h"

using namespace DirectX;

//=========================================
// MAP構成ブロックの種類
//=========================================

enum FIELD
{
    FIELD_BOX = 0,        // 箱
    FIELD_BUILDING,       // 建物（Building）
    FIELD_MAX
};

//=========================================
// MAPデータ構造体（1マス分）
//=========================================

class Glass
{
public:
    XMFLOAT3 pos;         // 座標
    AABB boundingBox;     // 当たり判定
    FIELD GL;

    bool isActive = true;
    // --- ここを追加！ ---
    BuildingType type = BuildingType::Glass;

};

class Concrete
{
public:
    XMFLOAT3 pos;         // 座標
    AABB boundingBox;     // 当たり判定
    FIELD Co;

    bool isActive = true;
    // --- ここを追加！ ---
    BuildingType type = BuildingType::Concrete;

};

class Plant
{
public:
    XMFLOAT3 pos;         // 座標
    AABB boundingBox;     // 当たり判定
    FIELD Pl;

    bool isActive = true;
    // --- ここを追加！ ---
    BuildingType type = BuildingType::Plant;

};

class ELECTRIC
{
public:
    XMFLOAT3 pos;         // 座標
    AABB boundingBox;     // 当たり判定
    FIELD El;
    bool isActive = true;
    // --- ここを追加！ ---
    BuildingType type = BuildingType::Electric;

};



