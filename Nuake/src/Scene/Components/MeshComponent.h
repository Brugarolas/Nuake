#pragma once
#include <vector>

#include "src/Resource/Serializable.h"
#include "src/Resource/Model.h"

#include <string>

namespace Nuake
{
    struct MeshComponent
    {
        Ref<Model> ModelResource;
        std::string ModelPath;

        MeshComponent();

        void LoadModel();
       
        std::string directory;

        json Serialize() 
        {
            BEGIN_SERIALIZE();
            SERIALIZE_VAL(ModelPath);
            SERIALIZE_OBJECT(ModelResource);
            END_SERIALIZE();
        }

        bool Deserialize(const std::string str) 
        {
            BEGIN_DESERIALIZE();
            ModelPath = j["ModelPath"].dump();
            ModelResource = CreateRef<Model>();

            if (j.contains("ModelResource"))
            {
                auto& res = j["ModelResource"];
                ModelResource->Deserialize(res.dump());
            }

            return true;
        }
    };
}
