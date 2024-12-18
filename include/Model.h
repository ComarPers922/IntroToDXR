#pragma once
#include <array>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <DirectXMath.h>
#include <iostream>
#include <vector>

struct Material;
using namespace DirectX;

const std::string UNDEFINED_TEXTURE = "UNDEFINED_TEXTURE";

struct Vertex;

class Model
{
public:
    Model(const std::vector<Vertex>& vertices = std::vector<Vertex>(),
        const std::vector<uint32_t>& indices = std::vector<uint32_t>(),
        const XMMATRIX& transformation = XMMatrixIdentity());

    static Model* ImportMeshFromFileOfIndex(
        const std::string& path,
        size_t index,
        Material& material,
        const XMFLOAT4& color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));

    static XMMATRIX ConvertMatrix(aiMatrix4x4 inMat)
	{
        return XMLoadFloat4x4(&XMFLOAT4X4(
            inMat.a1, inMat.b1, inMat.c1, inMat.d1,
            inMat.a2, inMat.b2, inMat.c2, inMat.d2,
            inMat.a3, inMat.b3, inMat.c3, inMat.d3,
            inMat.a4, inMat.b4, inMat.c4, inMat.d4));
    }

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    // std::vector<uint32_t> normals;
    // std::vector<XMFLOAT4> colors;

    XMMATRIX transformation;
};

