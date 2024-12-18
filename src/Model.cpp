#include "Model.h"
#include "Structures.h"

Model::Model(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, const XMMATRIX& transformation) :
    vertices(vertices), indices(indices), transformation(transformation)
{

}

Model* Model::ImportMeshFromFileOfIndex(
    const std::string& path, 
    size_t index,
    Material& material,
    const XMFLOAT4& color)
{
    Assimp::Importer importer;
    const auto* scene = importer.ReadFile(path, aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices);
    const auto* rootNode = scene->mRootNode;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    if (rootNode->mNumChildren < index)
    {
        return nullptr;
    }

    const auto* childNode = rootNode->mChildren[index];

    if (childNode->mNumMeshes <= 0)
    {
        return nullptr;
    }

    auto modelTransformation = ConvertMatrix(childNode->mTransformation);

    const auto* mesh = scene->mMeshes[childNode->mMeshes[0]];

    if (scene->HasMaterials())
    {
        auto matIndex = mesh->mMaterialIndex;
        auto* curMat = scene->mMaterials[matIndex];
        aiString textureName;
        curMat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureName);
        material.texturePath = std::string("textures\\") + std::string(textureName.C_Str());
    }
    else
    {
        material.texturePath = std::string("textures\\statue.jpg");
    }

    vertices.reserve(mesh->mNumVertices);
    indices.reserve(mesh->mNumFaces * 3);

    for (size_t i = 0; i < mesh->mNumVertices; i++)
    {
        const auto& curVert = mesh->mVertices[i];
        const auto& curNormal = mesh->mNormals[i];
        const auto& curTexCoord = mesh->mTextureCoords[0][i];
        vertices.emplace_back(Vertex
            {
                {curVert.x, curVert.y, curVert.z},
                {curTexCoord.x, 1.f - curTexCoord.y},
                {curNormal.x, curNormal.y, curNormal.z},
            });
    }

    for (size_t i = 0; i < mesh->mNumFaces; i++)
    {
        const auto& curFace = mesh->mFaces[i];
        for (size_t j = 0; j < 3; j++)
        {
            indices.emplace_back(curFace.mIndices[j]);
        }
    }

    return new Model(vertices, indices, modelTransformation);
}