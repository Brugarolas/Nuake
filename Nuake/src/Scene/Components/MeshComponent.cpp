#include "MeshComponent.h"
#include "src/Rendering/Textures/Material.h"
#include "src/Rendering/Renderer.h"
#include "src/Rendering/Textures/TextureManager.h"
#include <src/Rendering/Textures/MaterialManager.h>

namespace Nuake {
    void MeshComponent::LoadModel()
    {
        this->meshes.clear();
        Assimp::Importer import;
        import.SetPropertyFloat("PP_GSN_MAX_SMOOTHING_ANGLE", 90);
        const aiScene* scene = import.ReadFile(FileSystem::Root + ModelPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FixInfacingNormals | aiProcess_CalcTangentSpace);
        

        directory = "";
        std::vector<std::string> path = String::Split(ModelPath, '/');
        for (int i = 0; i < path.size(); i++)
            if (i < path.size() - 1)
                directory += path[i] + '/';
            
        if (scene->HasTextures())
        {
            int textureCount = scene->mNumTextures;
            for (unsigned int i = 0; i < textureCount; i++)
            {
                scene->mTextures[i];
            }
        }
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            Logger::Log("ASSIMP! Failed to load model" + std::string(import.GetErrorString()), CRITICAL);
            return;
        }

        ProcessNode(scene->mRootNode, scene);
    }

    void MeshComponent::ProcessNode(aiNode* node, const aiScene* scene)
    {
        // process all the node's meshes (if any)
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(ProcessMesh(mesh, scene));
        }
        // then do the same for each of its children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene);
        }
    }

    Ref<Mesh> MeshComponent::ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vertex.texture = 1.0f;

            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;

            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.normal = vector;

            if (mesh->mTangents)
            {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
            }

            if (mesh->mBitangents)
            {
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            }


            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.uv = vec;
            }
            else
                vertex.uv = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // process indices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // process material
        if (mesh->mMaterialIndex < 0)
            return nullptr;

        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        std::string materialPath = directory + std::to_string(mesh->mMaterialIndex) + material->GetName().C_Str();
        Ref<Material> newMaterial = MaterialManager::Get()->GetMaterial(materialPath);

        aiString str;
            
        unsigned int textureCount = material->GetTextureCount(aiTextureType_DIFFUSE);
        material->GetTexture(aiTextureType_DIFFUSE, 0, &str);

        // Embedded texture
        if (String::BeginsWith(str.C_Str(), "*"))
        {
            int textureIndex = std::atoi(String::Split(str.C_Str(), '*')[1].c_str());
            const aiTexture* texture = scene->GetEmbeddedTexture(str.C_Str());

            Ref<Texture> newTexture = CreateRef<Texture>(Vector2(texture->mWidth, texture->mHeight), (unsigned char*)texture->pcData, texture->mWidth);
            newMaterial->SetAlbedo(newTexture);
        }
        else
        {
            std::string texturePath = FileSystem::Root + directory + str.C_Str();
            if (!FileSystem::FileExists(texturePath))
            {
                Logger::Log("Texture file couldn't be found: " + texturePath, Nuake::LOG_TYPE::CRITICAL);
                texturePath = "resources/Textures/default/Default.png";
            }
            newMaterial->SetAlbedo(TextureManager::Get()->GetTexture(texturePath));
        }

        material->GetTexture(aiTextureType_NORMALS, 0, &str);
        if (String::BeginsWith(str.C_Str(), "*"))
        {
            int textureIndex = std::atoi(String::Split(str.C_Str(), '*')[1].c_str());
            const aiTexture* texture = scene->GetEmbeddedTexture(str.C_Str());

            Ref<Texture> newTexture = CreateRef<Texture>(Vector2(texture->mWidth, texture->mHeight), (unsigned char*)texture->pcData, texture->mWidth);
            newMaterial->SetNormal(newTexture);
        }
        else
        {
            newMaterial->SetNormal(TextureManager::Get()->GetTexture(FileSystem::Root + directory + str.C_Str()));
        }
        //newMaterial->SetNormal(TextureManager::Get()->GetTexture(directory + str.C_Str()));

        material->GetTexture(aiTextureType_METALNESS, 0, &str);
        if (String::BeginsWith(str.C_Str(), "*"))
        {
            int textureIndex = std::atoi(String::Split(str.C_Str(), '*')[1].c_str());
            const aiTexture* texture = scene->GetEmbeddedTexture(str.C_Str());

            Ref<Texture> newTexture = CreateRef<Texture>(Vector2(texture->mWidth, texture->mHeight), (unsigned char*)texture->pcData, texture->mWidth);
            newMaterial->SetMetalness(newTexture);
        }
        else
        {
            newMaterial->SetMetalness(TextureManager::Get()->GetTexture(FileSystem::Root + directory + str.C_Str()));
        }
        //newMaterial->SetMetalness(TextureManager::Get()->GetTexture(directory + str.C_Str()));

        material->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &str);
        if (String::BeginsWith(str.C_Str(), "*"))
        {
            int textureIndex = std::atoi(String::Split(str.C_Str(), '*')[1].c_str());
            const aiTexture* texture = scene->GetEmbeddedTexture(str.C_Str());

            Ref<Texture> newTexture = CreateRef<Texture>(Vector2(texture->mWidth, texture->mHeight), (unsigned char*)texture->pcData, texture->mWidth);
            newMaterial->SetRoughness(newTexture);
        }
        else
        {
            newMaterial->SetRoughness(TextureManager::Get()->GetTexture(FileSystem::Root + directory + str.C_Str()));
        }
        //newMaterial->SetRoughness(TextureManager::Get()->GetTexture(directory + str.C_Str()));

        //material->GetTexture(aiTextureType_DISPLACEMENT, 0, &str);
        //newMaterial->SetDisplacement(TextureManager::Get()->GetTexture(directory + str.C_Str()));

        material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &str);
        if (String::BeginsWith(str.C_Str(), "*"))
        {
            int textureIndex = std::atoi(String::Split(str.C_Str(), '*')[1].c_str());
            const aiTexture* texture = scene->GetEmbeddedTexture(str.C_Str());

            Ref<Texture> newTexture = CreateRef<Texture>(Vector2(texture->mWidth, texture->mHeight), (unsigned char*)texture->pcData, texture->mWidth);
            newMaterial->SetAO(newTexture);
        }
        else
        {
            newMaterial->SetAO(TextureManager::Get()->GetTexture(FileSystem::Root + directory + str.C_Str()));
        }
        //newMaterial->SetAO(TextureManager::Get()->GetTexture(directory + str.C_Str()));

        Ref<Mesh> nuakeMesh = CreateRef<Mesh>();
        nuakeMesh->AddSurface(vertices, indices);
        nuakeMesh->SetMaterial(newMaterial);

        return nuakeMesh;
    }

    std::vector<Texture*> MeshComponent::LoadMaterialTextures(aiMaterial* mat, aiTextureType type)
    {
        std::vector<Texture*> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            std::string fixedStr = std::string(str.C_Str());

            Texture* texture = new Texture(directory + fixedStr);
            textures.push_back(texture);
        }
        return textures;
    }
}

