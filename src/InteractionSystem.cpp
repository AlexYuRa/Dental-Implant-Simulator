/**
 * @file InteractionSystem.cpp
 * @brief Implementación del sistema de interacción 3D
 * 
 * Contiene la lógica de:
 * - RayCaster: Conversión de coordenadas 2D → rayo 3D
 * - AABBCalculator: Cálculo de bounding boxes
 * - DragHandler: Arrastre de objetos en un plano
 * 
 * Ver InteractionSystem.h para documentación de las clases.
 */

#include "InteractionSystem.h"
#include "Model.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

// ============================================================================
// RayCaster Implementation
// ============================================================================

glm::vec3 RayCaster::screenToWorldRay(
    double mouseX, double mouseY,
    int screenWidth, int screenHeight,
    const Camera& camera
) {
    // 1. Normalizar coordenadas de pantalla a rango [-1, 1]
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;  // Invertir Y

    // 2. Crear vector en clip space
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // 3. Convertir a eye space (deshacer proyección)
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (float)screenWidth / (float)screenHeight,
        0.1f, 10000.0f
    );
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // 4. Convertir a world space (deshacer vista)
    glm::vec3 rayWorld = glm::vec3(glm::inverse(camera.getViewMatrix()) * rayEye);
    return glm::normalize(rayWorld);
}

bool RayCaster::rayAABBIntersection(
    const glm::vec3& rayOrigin,
    const glm::vec3& rayDir,
    const AABB& aabb,
    float& distance
) {
    // Calcular inversa de la dirección del rayo
    glm::vec3 invDir = 1.0f / rayDir;

    // Calcular distancias a los planos de la caja
    glm::vec3 t0s = (aabb.min - rayOrigin) * invDir;
    glm::vec3 t1s = (aabb.max - rayOrigin) * invDir;

    // Asegurar que t0 < t1
    glm::vec3 tsmaller = glm::min(t0s, t1s);
    glm::vec3 tbigger = glm::max(t0s, t1s);

    // Encontrar el punto de entrada y salida
    float tmin = glm::max(glm::max(tsmaller.x, tsmaller.y), tsmaller.z);
    float tmax = glm::min(glm::min(tbigger.x, tbigger.y), tbigger.z);

    // Verificar intersección
    if (tmin > tmax || tmax < 0.0f) return false;

    // Calcular distancia
    distance = tmin > 0.0f ? tmin : tmax;
    return true;
}

AABB RayCaster::calculateTransformedAABB(
    Model* model,
    const glm::mat4& transform
) {
    AABB aabb;

    // Obtener AABB original del modelo
    for (const auto& mesh : model->getMeshes()) {
        for (const auto& vertex : mesh.getVertices()) {
            glm::vec4 transformedPos = transform * glm::vec4(vertex.Position, 1.0f);
            aabb.expand(glm::vec3(transformedPos));
        }
    }

    return aabb;
}

glm::vec3 RayCaster::projectToViewPlane(
    const glm::vec3& cameraPos,
    const glm::vec3& rayDir,
    float distance
) {
    return cameraPos + rayDir * distance;
}

// ============================================================================
// DragController Implementation
// ============================================================================

DragController::DragController(int screenWidth, int screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_selectedObject(DraggableObject::NONE)
    , m_isDragging(false)
    , m_quadrupedPosition(0.0f, 0.0f, 0.0f)
    , m_scenePosition(0.0f, 0.0f, 0.0f)
    , m_consultorioPosition(0.0f, 0.0f, 0.0f)
    , m_dragOffset(0.0f, 0.0f, 0.0f)
    , m_dragPlaneDistance(0.0f)
{
}

void DragController::handleMouseClick(
    double mouseX, double mouseY,
    const Camera& camera,
    Model* quadrupedModel,
    Model* sceneModel,
    Model* consultorioModel,
    const glm::vec3& quadrupedCurrentPos,
    const glm::vec3& sceneCurrentPos,
    const glm::vec3& consultorioCurrentPos
) {
    glm::vec3 rayOrigin = camera.getPosition();
    glm::vec3 rayDir = RayCaster::screenToWorldRay(
        mouseX, mouseY,
        m_screenWidth, m_screenHeight,
        camera
    );

    float quadrupedDist = FLT_MAX;
    float consultorioDist = FLT_MAX;

    // Test contra cuadrúpedo
    glm::mat4 quadrupedTransform = glm::translate(glm::mat4(1.0f), quadrupedCurrentPos);
    AABB quadrupedAABB = RayCaster::calculateTransformedAABB(quadrupedModel, quadrupedTransform);
    bool hitQuadruped = RayCaster::rayAABBIntersection(rayOrigin, rayDir, quadrupedAABB, quadrupedDist);

    // Test contra consultorio
    glm::mat4 consultorioTransform = glm::translate(glm::mat4(1.0f), consultorioCurrentPos);
    AABB consultorioAABB = RayCaster::calculateTransformedAABB(consultorioModel, consultorioTransform);
    bool hitConsultorio = RayCaster::rayAABBIntersection(rayOrigin, rayDir, consultorioAABB, consultorioDist);

    // DEBUG
    std::cout << "\n=== CLICK DEBUG ===" << std::endl;
    std::cout << "Cuadrupedo: " << (hitQuadruped ? "HIT" : "NO HIT");
    if (hitQuadruped) std::cout << " (dist: " << quadrupedDist << ")";
    std::cout << std::endl;
    std::cout << "Consultorio: " << (hitConsultorio ? "HIT" : "NO HIT");
    if (hitConsultorio) std::cout << " (dist: " << consultorioDist << ")";
    std::cout << std::endl;

    // Seleccionar el más cercano
    if (hitQuadruped && quadrupedDist < consultorioDist) {
        m_selectedObject = DraggableObject::QUADRUPED;
        m_dragPlaneDistance = quadrupedDist;
        glm::vec3 hitPoint = RayCaster::projectToViewPlane(rayOrigin, rayDir, quadrupedDist);
        m_dragOffset = m_quadrupedPosition - hitPoint;
        std::cout << ">>> CUADRUPEDO SELECCIONADO <<<" << std::endl;
        m_isDragging = true;
    } else if (hitConsultorio) {
        m_selectedObject = DraggableObject::CONSULTORIO;
        m_dragPlaneDistance = consultorioDist;
        glm::vec3 hitPoint = RayCaster::projectToViewPlane(rayOrigin, rayDir, consultorioDist);
        m_dragOffset = m_consultorioPosition - hitPoint;
        std::cout << ">>> CONSULTORIO SELECCIONADO <<<" << std::endl;
        m_isDragging = true;
    } else {
        std::cout << ">>> NADA SELECCIONADO <<<" << std::endl;
    }
    std::cout << "===================\n" << std::endl;
}

void DragController::handleMouseDrag(
    double mouseX, double mouseY,
    const Camera& camera
) {
    if (!m_isDragging || m_selectedObject == DraggableObject::NONE) return;

    glm::vec3 rayDir = RayCaster::screenToWorldRay(
        mouseX, mouseY,
        m_screenWidth, m_screenHeight,
        camera
    );
    glm::vec3 newPosition = RayCaster::projectToViewPlane(
        camera.getPosition(),
        rayDir,
        m_dragPlaneDistance
    ) + m_dragOffset;

    if (m_selectedObject == DraggableObject::QUADRUPED) {
        m_quadrupedPosition = newPosition;
    } else if (m_selectedObject == DraggableObject::SCENE) {
        m_scenePosition = newPosition;
    } else if (m_selectedObject == DraggableObject::CONSULTORIO) {
        m_consultorioPosition = newPosition;
    }
}

void DragController::handleMouseHover(
    double mouseX, double mouseY,
    const Camera& camera,
    Model* quadrupedModel,
    Model* sceneModel,
    Model* consultorioModel,
    const glm::vec3& quadrupedCurrentPos,
    const glm::vec3& sceneCurrentPos,
    const glm::vec3& consultorioCurrentPos
) {
    glm::vec3 rayOrigin = camera.getPosition();
    glm::vec3 rayDir = RayCaster::screenToWorldRay(
        mouseX, mouseY,
        m_screenWidth, m_screenHeight,
        camera
    );

    float quadrupedDist = FLT_MAX;
    float consultorioDist = FLT_MAX;

    // Test contra cuadrúpedo
    glm::mat4 quadrupedTransform = glm::translate(glm::mat4(1.0f), quadrupedCurrentPos);
    AABB quadrupedAABB = RayCaster::calculateTransformedAABB(quadrupedModel, quadrupedTransform);
    bool hitQuadruped = RayCaster::rayAABBIntersection(rayOrigin, rayDir, quadrupedAABB, quadrupedDist);

    // Test contra consultorio
    glm::mat4 consultorioTransform = glm::translate(glm::mat4(1.0f), consultorioCurrentPos);
    AABB consultorioAABB = RayCaster::calculateTransformedAABB(consultorioModel, consultorioTransform);
    bool hitConsultorio = RayCaster::rayAABBIntersection(rayOrigin, rayDir, consultorioAABB, consultorioDist);

    // Mostrar información de hover
    static double lastHoverTime = 0.0;
    double currentTime = glfwGetTime();

    // Actualizar cada 0.5 segundos para no saturar la consola
    if (currentTime - lastHoverTime > 0.5) {
        if (hitQuadruped || hitConsultorio) {
            std::cout << "\n=== HOVER INFO ===" << std::endl;
            if (hitQuadruped) std::cout << "  Cuadrupedo: HIT (distancia: " << quadrupedDist << ")" << std::endl;
            if (hitConsultorio) std::cout << "  Consultorio: HIT (distancia: " << consultorioDist << ")" << std::endl;

            if (hitQuadruped && quadrupedDist < consultorioDist) {
                std::cout << "  >> SE SELECCIONARIA: CUADRUPEDO <<" << std::endl;
            } else if (hitConsultorio) {
                std::cout << "  >> SE SELECCIONARIA: CONSULTORIO <<" << std::endl;
            }
            std::cout << "==================\n" << std::endl;
        }
        lastHoverTime = currentTime;
    }
}

void DragController::handleMouseRelease() {
    if (m_isDragging) {
        std::cout << "Objeto liberado en posicion: ("
                    << (m_selectedObject == DraggableObject::QUADRUPED ? m_quadrupedPosition.x : m_scenePosition.x) << ", "
                    << (m_selectedObject == DraggableObject::QUADRUPED ? m_quadrupedPosition.y : m_scenePosition.y) << ", "
                    << (m_selectedObject == DraggableObject::QUADRUPED ? m_quadrupedPosition.z : m_scenePosition.z) << ")"
                    << std::endl;
    }
    m_isDragging = false;
    m_selectedObject = DraggableObject::NONE;
}
