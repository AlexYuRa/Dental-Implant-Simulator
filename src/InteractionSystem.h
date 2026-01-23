/**
 * @file InteractionSystem.h
 * @brief Sistema de interacción 3D (selección, ray casting, arrastre)
 * 
 * Permite seleccionar y manipular objetos 3D con el mouse:
 * - AABB: Cajas de colisión alineadas a los ejes
 * - RayCaster: Conversión de click 2D a rayo 3D
 * - DragHandler: Lógica de arrastrar objetos
 * 
 * Proceso de selección:
 * 1. Click del usuario (coordenadas 2D de pantalla)
 * 2. RayCaster genera un rayo en espacio 3D
 * 3. Se prueba intersección con AABBs de objetos
 * 4. Se selecciona el objeto más cercano impactado
 * 
 * Usado por: Application.h para selección de instrumental
 */

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Forward declarations
class Model;
class Camera;

// ============================================================================
// AABB (Axis-Aligned Bounding Box)
// ============================================================================
struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB() : min(FLT_MAX), max(-FLT_MAX) {}

    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    glm::vec3 center() const {
        return (min + max) * 0.5f;
    }
};

// ============================================================================
// RayCaster - Clase para operaciones de ray casting
// ============================================================================
class RayCaster {
public:
    // Convierte coordenadas de pantalla (2D) a rayo 3D en el mundo
    static glm::vec3 screenToWorldRay(
        double mouseX, double mouseY,
        int screenWidth, int screenHeight,
        const Camera& camera
    );

    // Test de intersección entre un rayo y un AABB
    static bool rayAABBIntersection(
        const glm::vec3& rayOrigin,
        const glm::vec3& rayDir,
        const AABB& aabb,
        float& distance
    );

    // Calcula el AABB de un modelo después de aplicar transformaciones
    static AABB calculateTransformedAABB(
        Model* model,
        const glm::mat4& transform
    );

    // Proyecta un punto 3D al plano perpendicular a la cámara
    static glm::vec3 projectToViewPlane(
        const glm::vec3& cameraPos,
        const glm::vec3& rayDir,
        float distance
    );
};

// ============================================================================
// DragController - Clase para gestionar drag & drop de objetos
// ============================================================================
class DragController {
public:
    enum class DraggableObject {
        NONE,
        QUADRUPED,
        SCENE,
        CONSULTORIO
    };

    DragController(int screenWidth, int screenHeight);

    // Manejo de eventos de mouse
    void handleMouseClick(
        double mouseX, double mouseY,
        const Camera& camera,
        Model* quadrupedModel,
        Model* sceneModel,
        Model* consultorioModel,
        const glm::vec3& quadrupedCurrentPos,
        const glm::vec3& sceneCurrentPos,
        const glm::vec3& consultorioCurrentPos
    );

    void handleMouseDrag(
        double mouseX, double mouseY,
        const Camera& camera
    );

    void handleMouseHover(
        double mouseX, double mouseY,
        const Camera& camera,
        Model* quadrupedModel,
        Model* sceneModel,
        Model* consultorioModel,
        const glm::vec3& quadrupedCurrentPos,
        const glm::vec3& sceneCurrentPos,
        const glm::vec3& consultorioCurrentPos
    );

    void handleMouseRelease();

    // Getters
    bool isDragging() const { return m_isDragging; }
    DraggableObject getSelectedObject() const { return m_selectedObject; }
    const glm::vec3& getQuadrupedPosition() const { return m_quadrupedPosition; }
    const glm::vec3& getScenePosition() const { return m_scenePosition; }
    const glm::vec3& getConsultorioPosition() const { return m_consultorioPosition; }

private:
    // Dimensiones de pantalla
    int m_screenWidth;
    int m_screenHeight;

    // Estado de drag & drop
    DraggableObject m_selectedObject;
    bool m_isDragging;
    glm::vec3 m_quadrupedPosition;
    glm::vec3 m_scenePosition;
    glm::vec3 m_consultorioPosition;
    glm::vec3 m_dragOffset;
    float m_dragPlaneDistance;
};
