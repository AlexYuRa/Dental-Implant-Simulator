/**
 * @file Animation.h
 * @brief Sistema de animación esquelética
 * 
 * Carga y gestiona animaciones desde archivos ASSIMP (GLB/FBX).
 * Contiene la jerarquía de nodos del esqueleto y los keyframes
 * de cada hueso para reproducir animaciones fluidas.
 * 
 * Usado por: Animator.h para reproducir animaciones en tiempo real
 * Depende de: Bone.h, Model.h, ASSIMP
 */

#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "Bone.h"
#include "Model.h"

#include <iostream>

// Estructura para representar la jerarquía de nodos del esqueleto
struct AssimpNodeData
{
    glm::mat4 transformation;
    std::string name;
    int childrenCount;
    std::vector<AssimpNodeData> children;
};

// Clase Animation: carga y gestiona animaciones desde ASSIMP
class Animation
{
public:
    // Constructor: carga animación desde archivo
    Animation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        assert(scene && scene->mRootNode);

        // Verificar que el archivo tiene animaciones
        if (!scene->HasAnimations())
        {
            std::cout << "ERROR::ANIMATION: El archivo no contiene animaciones" << std::endl;
            return;
        }

        // Obtener la primera animación
        auto animation = scene->mAnimations[0];
        m_Duration = animation->mDuration;
        m_TicksPerSecond = animation->mTicksPerSecond;

        // Leer jerarquía de nodos desde el nodo raíz
        ReadHeirarchyData(m_RootNode, scene->mRootNode);

        // Leer bones faltantes y crear objetos Bone
        ReadMissingBones(animation, *model);
    }

    ~Animation() {}

    // Busca un bone por nombre
    Bone* FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& Bone)
            {
                return Bone.GetBoneName() == name;
            }
        );

        if (iter == m_Bones.end())
            return nullptr;
        else
            return &(*iter);
    }

    // Getters
    inline float GetTicksPerSecond() const { return m_TicksPerSecond; }
    inline float GetDuration() const { return m_Duration; }
    inline const AssimpNodeData& GetRootNode() const { return m_RootNode; }
    inline const std::map<std::string, BoneInfo>& GetBoneIDMap() const { return m_BoneInfoMap; }

private:
    float m_Duration;                               // Duración de la animación
    int m_TicksPerSecond;                          // Ticks por segundo
    std::vector<Bone> m_Bones;                     // Vector de todos los bones
    AssimpNodeData m_RootNode;                     // Nodo raíz de la jerarquía
    std::map<std::string, BoneInfo> m_BoneInfoMap; // Mapeo de nombres de bones a BoneInfo

    // Lee bones faltantes desde la animación y los sincroniza con el modelo
    void ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        int size = animation->mNumChannels;

        // Obtener el mapa de bones del modelo
        auto& boneInfoMap = model.GetBoneInfoMap();
        int& boneCount = model.GetBoneCount();

        // Iterar sobre todos los canales de la animación
        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            // Si el bone no existe en el mapa, agregarlo
            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }

            // Crear objeto Bone con los keyframes del canal
            m_Bones.push_back(Bone(boneName, boneInfoMap[boneName].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    // Lee recursivamente la jerarquía de nodos desde ASSIMP
    void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        assert(src);

        dest.name = src->mName.data;
        dest.transformation = ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        // Leer recursivamente todos los nodos hijos
        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }
};

#endif
