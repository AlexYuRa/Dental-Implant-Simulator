/**
 * @file Animator.h
 * @brief Controlador de animaciones esqueléticas en tiempo real
 * 
 * Actualiza el estado de una animación frame a frame,
 * calculando las matrices de transformación finales de cada hueso.
 * Estas matrices se envían al shader para deformar el modelo.
 * 
 * Responsabilidades:
 * - Avanzar el tiempo de la animación
 * - Interpolar entre keyframes
 * - Calcular matrices finales de huesos (hasta 200)
 * - Controlar loop, pausa, velocidad
 * 
 * Depende de: Animation.h, Bone.h
 */

#ifndef ANIMATOR_H
#define ANIMATOR_H

#include <vector>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Animation.h"
#include "Bone.h"

#include <iostream>

// Clase Animator: controla la animación en tiempo real
class Animator
{
public:
    static constexpr int MAX_BONES = 200;
    // Constructor: inicializa el animator con una animación
    Animator(Animation* animation)
    {
        m_CurrentTime = 0.0f;
        m_CurrentAnimation = animation;
        m_Loop = true;  // Por defecto en loop
        m_Finished = false;

        // Reservar espacio para matrices de bones
        m_FinalBoneMatrices.reserve(MAX_BONES);
        for (int i = 0; i < MAX_BONES; i++)
            m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
    }

    // Actualiza la animación cada frame
    void UpdateAnimation(float dt)
    {
        m_DeltaTime = dt;
        if (m_CurrentAnimation && !m_Finished)
        {
            // Avanzar el tiempo de animación
            m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;

            // Verificar si llegó al final
            float duration = m_CurrentAnimation->GetDuration();
            if (m_CurrentTime >= duration) {
                if (m_Loop) {
                    // Loop infinito: reiniciar cuando llegue al final
                    m_CurrentTime = fmod(m_CurrentTime, duration);
                } else {
                    // Sin loop: quedarse en el último frame
                    m_CurrentTime = duration - 0.001f;  // Ligeramente antes del final
                    m_Finished = true;
                }
            }

            // Calcular todas las transformaciones de bones
            CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
        }
    }

    // Cambia la animación actual
    void PlayAnimation(Animation* pAnimation)
    {
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
        m_Finished = false;
    }

    // Configurar si la animación debe repetirse en loop
    void SetLoop(bool loop) { m_Loop = loop; }
    bool IsLooping() const { return m_Loop; }
    
    // Verificar si la animación terminó (solo si no está en loop)
    bool IsFinished() const { return m_Finished; }
    
    // Reiniciar la animación
    void Reset() {
        m_CurrentTime = 0.0f;
        m_Finished = false;
    }

    // Retorna las matrices finales de bones para pasar al shader
    const std::vector<glm::mat4>& GetFinalBoneMatrices() const
    {
        return m_FinalBoneMatrices;
    }

    // Obtener el tiempo actual de la animación
    float GetCurrentTime() const { return m_CurrentTime; }

private:
    std::vector<glm::mat4> m_FinalBoneMatrices;  // Matrices finales
    Animation* m_CurrentAnimation;               // Animación actual
    float m_CurrentTime;                         // Tiempo actual de animación
    float m_DeltaTime;                           // Delta time
    bool m_Loop;                                 // Si la animación se repite
    bool m_Finished;                             // Si la animación terminó (solo sin loop)

    // Calcula recursivamente las transformaciones de bones
    void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform)
    {
        std::string nodeName = node->name;
        glm::mat4 nodeTransform = node->transformation;

        // Buscar si este nodo corresponde a un bone
        Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone)
        {
            // Si es un bone, actualizar su transformación con interpolación
            Bone->Update(m_CurrentTime);
            nodeTransform = Bone->GetLocalTransform();
        }

        // Calcular transformación global (acumulativa desde la raíz)
        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        // Si este nodo es un bone registrado, calcular matriz final
        auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
        if (boneInfoMap.find(nodeName) != boneInfoMap.end())
        {
            int index = boneInfoMap[nodeName].id;
            glm::mat4 offset = boneInfoMap[nodeName].offset;

            // Matriz final = transformación global * offset matrix
            m_FinalBoneMatrices[index] = globalTransformation * offset;
        }

        // Procesar recursivamente todos los nodos hijos
        for (int i = 0; i < node->childrenCount; i++)
            CalculateBoneTransform(&node->children[i], globalTransformation);
    }
};

#endif
