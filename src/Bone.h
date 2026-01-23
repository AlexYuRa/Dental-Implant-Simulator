/**
 * @file Bone.h
 * @brief Representación de huesos para animación esquelética
 * 
 * Define la estructura de un hueso individual del esqueleto:
 * - Keyframes de posición, rotación y escala
 * - Interpolación entre keyframes
 * - Cálculo de la matriz de transformación local
 * 
 * Un modelo animado puede tener hasta 200 huesos, cada uno
 * con múltiples keyframes que definen su movimiento.
 * 
 * Usado por: Animation.h, Animator.h
 */

#ifndef BONE_H
#define BONE_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/scene.h>
#include <vector>
#include <string>
#include <iostream>

// Estructura para keyframes de posición
struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

// Estructura para keyframes de rotación
struct KeyRotation {
    glm::quat orientation;
    float timeStamp;
};

// Estructura para keyframes de escala
struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};

// Clase Bone: representa un hueso individual con interpolación de keyframes
class Bone {
public:
    // Constructor: extrae keyframes desde aiNodeAnim
    Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        m_NumPositions = channel->mNumPositionKeys;
        m_NumRotations = channel->mNumRotationKeys;
        m_NumScalings = channel->mNumScalingKeys;

        // Extraer keyframes de posición
        for (unsigned int i = 0; i < m_NumPositions; i++) {
            KeyPosition data;
            data.position = glm::vec3(
                channel->mPositionKeys[i].mValue.x,
                channel->mPositionKeys[i].mValue.y,
                channel->mPositionKeys[i].mValue.z
            );
            data.timeStamp = channel->mPositionKeys[i].mTime;
            m_Positions.push_back(data);
        }

        // Extraer keyframes de rotación (quaternions)
        for (unsigned int i = 0; i < m_NumRotations; i++) {
            KeyRotation data;
            data.orientation = glm::quat(
                channel->mRotationKeys[i].mValue.w,  // w primero en glm::quat
                channel->mRotationKeys[i].mValue.x,
                channel->mRotationKeys[i].mValue.y,
                channel->mRotationKeys[i].mValue.z
            );
            data.timeStamp = channel->mRotationKeys[i].mTime;
            m_Rotations.push_back(data);
        }

        // Extraer keyframes de escala
        for (unsigned int i = 0; i < m_NumScalings; i++) {
            KeyScale data;
            data.scale = glm::vec3(
                channel->mScalingKeys[i].mValue.x,
                channel->mScalingKeys[i].mValue.y,
                channel->mScalingKeys[i].mValue.z
            );
            data.timeStamp = channel->mScalingKeys[i].mTime;
            m_Scales.push_back(data);
        }
    }

    // Actualiza la transformación local del hueso interpolando keyframes
    void Update(float animationTime) {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    // Getters
    glm::mat4 GetLocalTransform() const { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() const { return m_ID; }

private:
    std::vector<KeyPosition> m_Positions;
    std::vector<KeyRotation> m_Rotations;
    std::vector<KeyScale> m_Scales;
    int m_NumPositions;
    int m_NumRotations;
    int m_NumScalings;

    glm::mat4 m_LocalTransform;
    std::string m_Name;
    int m_ID;

    // Encuentra el índice del keyframe de posición anterior al tiempo dado
    int GetPositionIndex(float animationTime)
    {
        for (int index = 0; index < m_NumPositions - 1; ++index)
        {
            if (animationTime < m_Positions[index + 1].timeStamp)
                return index;
        }
        assert(0);  // No debería llegar aquí
        return 0;
    }

    // Encuentra el índice del keyframe de rotación anterior al tiempo dado
    int GetRotationIndex(float animationTime)
    {
        for (int index = 0; index < m_NumRotations - 1; ++index)
        {
            if (animationTime < m_Rotations[index + 1].timeStamp)
                return index;
        }
        assert(0);
        return 0;
    }

    // Encuentra el índice del keyframe de escala anterior al tiempo dado
    int GetScaleIndex(float animationTime)
    {
        for (int index = 0; index < m_NumScalings - 1; ++index)
        {
            if (animationTime < m_Scales[index + 1].timeStamp)
                return index;
        }
        assert(0);
        return 0;
    }

    // Calcula el factor de interpolación lineal entre dos keyframes
    float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        return midWayLength / framesDiff;
    }

    // Interpola la posición entre keyframes usando interpolación lineal
    glm::mat4 InterpolatePosition(float animationTime)
    {
        // Si solo hay un keyframe, retornar esa posición
        if (m_NumPositions == 1)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        // Encontrar keyframes anterior y siguiente
        int p0Index = GetPositionIndex(animationTime);
        int p1Index = p0Index + 1;

        // Calcular factor de interpolación
        float scaleFactor = GetScaleFactor(
            m_Positions[p0Index].timeStamp,
            m_Positions[p1Index].timeStamp,
            animationTime
        );

        // Interpolar linealmente entre las dos posiciones
        glm::vec3 finalPosition = glm::mix(
            m_Positions[p0Index].position,
            m_Positions[p1Index].position,
            scaleFactor
        );

        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    // Interpola la rotación entre keyframes usando interpolación esférica (slerp)
    glm::mat4 InterpolateRotation(float animationTime)
    {
        // Si solo hay un keyframe, retornar esa rotación
        if (m_NumRotations == 1)
        {
            auto rotation = glm::normalize(m_Rotations[0].orientation);
            return glm::mat4_cast(rotation);
        }

        // Encontrar keyframes anterior y siguiente
        int p0Index = GetRotationIndex(animationTime);
        int p1Index = p0Index + 1;

        // Calcular factor de interpolación
        float scaleFactor = GetScaleFactor(
            m_Rotations[p0Index].timeStamp,
            m_Rotations[p1Index].timeStamp,
            animationTime
        );

        // Interpolar esféricamente entre las dos rotaciones (quaternions)
        glm::quat finalRotation = glm::slerp(
            m_Rotations[p0Index].orientation,
            m_Rotations[p1Index].orientation,
            scaleFactor
        );

        finalRotation = glm::normalize(finalRotation);
        return glm::mat4_cast(finalRotation);
    }

    // Interpola la escala entre keyframes usando interpolación lineal
    glm::mat4 InterpolateScaling(float animationTime)
    {
        // Si solo hay un keyframe, retornar esa escala
        if (m_NumScalings == 1)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        // Encontrar keyframes anterior y siguiente
        int p0Index = GetScaleIndex(animationTime);
        int p1Index = p0Index + 1;

        // Calcular factor de interpolación
        float scaleFactor = GetScaleFactor(
            m_Scales[p0Index].timeStamp,
            m_Scales[p1Index].timeStamp,
            animationTime
        );

        // Interpolar linealmente entre las dos escalas
        glm::vec3 finalScale = glm::mix(
            m_Scales[p0Index].scale,
            m_Scales[p1Index].scale,
            scaleFactor
        );

        return glm::scale(glm::mat4(1.0f), finalScale);
    }
};

#endif
