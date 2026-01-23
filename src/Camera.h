/**
 * @file Camera.h
 * @brief Sistema de cámara orbital 3D
 * 
 * Implementa una cámara que orbita alrededor de un punto objetivo,
 * ideal para visualizar modelos 3D desde cualquier ángulo.
 * 
 * Características:
 * - Rotación orbital (yaw/pitch) con clic derecho
 * - Zoom con rueda del mouse
 * - Pan (desplazamiento) con clic medio
 * - Movimiento WASD del punto objetivo
 * - Vistas predefinidas (frontal, lateral, superior, etc.)
 * - Sistema de estados para undo/redo
 * 
 * Controles:
 * - Clic derecho + arrastrar: rotar
 * - Scroll: zoom
 * - Clic medio + arrastrar: pan
 * - WASD: mover objetivo
 */

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>

class Camera {
public:
    enum class ViewPreset {
        FREE,                  // Vista libre (por defecto)
        FRONTAL,              // Vista frontal de la arcada
        LATERAL_DERECHA,      // Vista lateral derecha
        LATERAL_IZQUIERDA,    // Vista lateral izquierda
        SUPERIOR,             // Vista superior (oclusal superior)
        INFERIOR,             // Vista inferior (oclusal inferior)
        OCLUSAL,              // Vista oclusal directa
        PANORAMICA            // Vista panorámica de toda la boca
    };

    struct CameraState {
        glm::vec3 target;
        float distance;
        float yaw;
        float pitch;
        ViewPreset preset;
    };

private:
    // Estado de la cámara
    glm::vec3 m_target;
    glm::vec3 m_targetSmooth;      // Para interpolación suave
    float m_distance;
    float m_distanceSmooth;        // Para zoom suave
    float m_yaw;
    float m_yawSmooth;             // Para rotación suave
    float m_pitch;
    float m_pitchSmooth;           // Para rotación suave
    
    // Estado de entrada
    float m_lastX;
    float m_lastY;
    bool m_firstMouse;
    bool m_rightMousePressed;
    bool m_middleMousePressed;
    
    // Smooth/Interpolación
    float m_smoothFactor;           // Factor de suavizado (0-1)
    bool m_enableSmoothing;
    
    // Presets
    ViewPreset m_currentPreset;
    bool m_isTransitioning;
    float m_transitionProgress;
    float m_transitionDuration;
    CameraState m_transitionStart;
    CameraState m_transitionEnd;
    
    // Restricciones espaciales
    bool m_enableBounds;
    glm::vec3 m_minBounds;
    glm::vec3 m_maxBounds;
    
    // Historia de estados (Undo/Redo)
    std::stack<CameraState> m_undoStack;
    std::stack<CameraState> m_redoStack;
    static constexpr size_t MAX_HISTORY = 50;
    
    // Constantes ajustadas para precisión médica
    static constexpr float DEFAULT_DISTANCE = 8.0f;
    static constexpr float DEFAULT_YAW = -90.0f;
    static constexpr float DEFAULT_PITCH = 25.0f;
    static constexpr float ROTATE_SENSITIVITY = 0.15f;
    static constexpr float PAN_SENSITIVITY = 0.008f;
    static constexpr float ZOOM_SENSITIVITY = 0.5f;
    static constexpr float KEYBOARD_SPEED = 5.0f;      // Ahora con deltaTime
    static constexpr float MIN_DISTANCE = 0.1f;        // Más cercano para detalles
    static constexpr float MAX_DISTANCE = 50.0f;
    static constexpr float MAX_PITCH = 89.0f;
    static constexpr float MIN_PITCH = -89.0f;
    static constexpr float DEFAULT_SMOOTH_FACTOR = 0.15f;
    static constexpr float TRANSITION_DURATION = 0.8f;  // Segundos

public:
    Camera(glm::vec3 target = glm::vec3(0.0f, 1.0f, 0.0f),
           float screenWidth = 1280.0f, float screenHeight = 720.0f)
        : m_target(target)
        , m_targetSmooth(target)
        , m_distance(DEFAULT_DISTANCE)
        , m_distanceSmooth(DEFAULT_DISTANCE)
        , m_yaw(DEFAULT_YAW)
        , m_yawSmooth(DEFAULT_YAW)
        , m_pitch(DEFAULT_PITCH)
        , m_pitchSmooth(DEFAULT_PITCH)
        , m_lastX(screenWidth / 2.0f)
        , m_lastY(screenHeight / 2.0f)
        , m_firstMouse(true)
        , m_rightMousePressed(false)
        , m_middleMousePressed(false)
        , m_smoothFactor(DEFAULT_SMOOTH_FACTOR)
        , m_enableSmoothing(true)
        , m_currentPreset(ViewPreset::FREE)
        , m_isTransitioning(false)
        , m_transitionProgress(0.0f)
        , m_transitionDuration(TRANSITION_DURATION)
        , m_enableBounds(false)
        , m_minBounds(-10.0f, -5.0f, -10.0f)
        , m_maxBounds(10.0f, 10.0f, 10.0f)
    {}

    // ========== ACTUALIZACIÓN PRINCIPAL ==========
    void update(float deltaTime) {
        if (m_isTransitioning) {
            updateTransition(deltaTime);
        }
        
        if (m_enableSmoothing) {
            // Interpolación suave de todos los parámetros
            float factor = std::min(1.0f, m_smoothFactor * deltaTime * 60.0f);
            m_targetSmooth = glm::mix(m_targetSmooth, m_target, factor);
            m_distanceSmooth = glm::mix(m_distanceSmooth, m_distance, factor);
            m_yawSmooth = glm::mix(m_yawSmooth, m_yaw, factor);
            m_pitchSmooth = glm::mix(m_pitchSmooth, m_pitch, factor);
        } else {
            m_targetSmooth = m_target;
            m_distanceSmooth = m_distance;
            m_yawSmooth = m_yaw;
            m_pitchSmooth = m_pitch;
        }
    }

    // ========== CÁLCULOS DE POSICIÓN ==========
    glm::vec3 getPosition() const {
        float dist = m_enableSmoothing ? m_distanceSmooth : m_distance;
        float yaw = m_enableSmoothing ? m_yawSmooth : m_yaw;
        float pitch = m_enableSmoothing ? m_pitchSmooth : m_pitch;
        glm::vec3 target = m_enableSmoothing ? m_targetSmooth : m_target;
        
        float x = target.x + dist * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        float y = target.y + dist * sin(glm::radians(pitch));
        float z = target.z + dist * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        return glm::vec3(x, y, z);
    }

    glm::vec3 getForward() const {
        glm::vec3 target = m_enableSmoothing ? m_targetSmooth : m_target;
        return glm::normalize(target - getPosition());
    }

    glm::vec3 getRight() const {
        return glm::normalize(glm::cross(getForward(), glm::vec3(0.0f, 1.0f, 0.0f)));
    }

    glm::vec3 getUp() const {
        return glm::normalize(glm::cross(getRight(), getForward()));
    }

    glm::mat4 getViewMatrix() const {
        return glm::lookAt(getPosition(), 
                          m_enableSmoothing ? m_targetSmooth : m_target, 
                          glm::vec3(0.0f, 1.0f, 0.0f));
    }

    // ========== ENTRADA DE MOUSE ==========
    void processMouseMovement(float xpos, float ypos) {
        if (m_firstMouse) {
            m_lastX = xpos;
            m_lastY = ypos;
            m_firstMouse = false;
            return;
        }

        float xoffset = xpos - m_lastX;
        float yoffset = m_lastY - ypos;
        m_lastX = xpos;
        m_lastY = ypos;

        if (m_rightMousePressed) {
            xoffset *= ROTATE_SENSITIVITY;
            yoffset *= ROTATE_SENSITIVITY;

            m_yaw += xoffset;
            m_pitch += yoffset;
            m_pitch = std::clamp(m_pitch, MIN_PITCH, MAX_PITCH);
            
            m_currentPreset = ViewPreset::FREE;
        }
        else if (m_middleMousePressed) {
            glm::vec3 right = getRight();
            glm::vec3 up = getUp();
            float panSpeed = PAN_SENSITIVITY * m_distance;

            m_target -= right * xoffset * panSpeed;
            m_target += up * yoffset * panSpeed;
            
            clampTargetToBounds();
            m_currentPreset = ViewPreset::FREE;
        }
    }

    void processMouseButton(int button, int action) {
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            m_rightMousePressed = (action == GLFW_PRESS);
            if (action == GLFW_PRESS) m_firstMouse = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
            m_middleMousePressed = (action == GLFW_PRESS);
            if (action == GLFW_PRESS) m_firstMouse = true;
        }
    }

    void processMouseScroll(float yoffset) {
        float zoomAmount = yoffset * ZOOM_SENSITIVITY;
        m_distance -= zoomAmount;
        m_distance = std::clamp(m_distance, MIN_DISTANCE, MAX_DISTANCE);
    }

    // ========== ENTRADA DE TECLADO (CON DELTA TIME) ==========
    void processKeyboard(GLFWwindow* window, float deltaTime) {
        glm::vec3 forward = getForward();
        glm::vec3 right = getRight();
        glm::vec3 forwardFlat = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
        glm::vec3 rightFlat = glm::normalize(glm::vec3(right.x, 0.0f, right.z));

        float speed = KEYBOARD_SPEED * deltaTime;
        
        // Modificador de velocidad
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            speed *= 3.0f;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            speed *= 0.3f;  // Movimiento fino para precisión

        // WASD
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            m_target += forwardFlat * speed;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            m_target -= forwardFlat * speed;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            m_target -= rightFlat * speed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            m_target += rightFlat * speed;

        // Q/E vertical
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            m_target.y -= speed;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            m_target.y += speed;

        clampTargetToBounds();
        
        // Si se mueve con teclado, salir del preset
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
            m_currentPreset = ViewPreset::FREE;
        }
    }

    // ========== PRESETS DE VISTA MÉDICA ==========
    void setViewPreset(ViewPreset preset) {
        if (m_isTransitioning) return; // No interrumpir transiciones
        
        saveState(); // Guardar en undo
        
        CameraState targetState;
        targetState.preset = preset;
        
        switch (preset) {
            case ViewPreset::FRONTAL:
                targetState.target = glm::vec3(0.0f, 1.0f, 0.0f);
                targetState.distance = 6.0f;
                targetState.yaw = -90.0f;
                targetState.pitch = 0.0f;
                break;
                
            case ViewPreset::LATERAL_DERECHA:
                targetState.target = glm::vec3(0.0f, 1.0f, 0.0f);
                targetState.distance = 6.0f;
                targetState.yaw = 0.0f;
                targetState.pitch = 0.0f;
                break;
                
            case ViewPreset::LATERAL_IZQUIERDA:
                targetState.target = glm::vec3(0.0f, 1.0f, 0.0f);
                targetState.distance = 6.0f;
                targetState.yaw = -180.0f;
                targetState.pitch = 0.0f;
                break;
                
            case ViewPreset::SUPERIOR:
                targetState.target = glm::vec3(0.0f, 1.0f, 0.0f);
                targetState.distance = 5.0f;
                targetState.yaw = -90.0f;
                targetState.pitch = 89.0f;
                break;
                
            case ViewPreset::INFERIOR:
                targetState.target = glm::vec3(0.0f, 1.0f, 0.0f);
                targetState.distance = 5.0f;
                targetState.yaw = -90.0f;
                targetState.pitch = -89.0f;
                break;
                
            case ViewPreset::OCLUSAL:
                targetState.target = glm::vec3(0.0f, 1.0f, 0.0f);
                targetState.distance = 4.0f;
                targetState.yaw = -90.0f;
                targetState.pitch = 60.0f;
                break;
                
            case ViewPreset::PANORAMICA:
                targetState.target = glm::vec3(0.0f, 1.0f, 0.0f);
                targetState.distance = 12.0f;
                targetState.yaw = -45.0f;
                targetState.pitch = 20.0f;
                break;
                
            case ViewPreset::FREE:
                return; // No hacer nada
        }
        
        startTransition(getCurrentState(), targetState);
    }

    // ========== TRANSICIONES SUAVES ==========
    void startTransition(const CameraState& from, const CameraState& to) {
        m_transitionStart = from;
        m_transitionEnd = to;
        m_transitionProgress = 0.0f;
        m_isTransitioning = true;
    }

    void updateTransition(float deltaTime) {
        m_transitionProgress += deltaTime / m_transitionDuration;
        
        if (m_transitionProgress >= 1.0f) {
            m_transitionProgress = 1.0f;
            m_isTransitioning = false;
        }
        
        // Ease in-out cubic
        float t = m_transitionProgress;
        t = t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
        
        m_target = glm::mix(m_transitionStart.target, m_transitionEnd.target, t);
        m_distance = glm::mix(m_transitionStart.distance, m_transitionEnd.distance, t);
        m_yaw = glm::mix(m_transitionStart.yaw, m_transitionEnd.yaw, t);
        m_pitch = glm::mix(m_transitionStart.pitch, m_transitionEnd.pitch, t);
        
        if (!m_isTransitioning) {
            m_currentPreset = m_transitionEnd.preset;
        }
    }

    // ========== RESTRICCIONES ESPACIALES ==========
    void setBounds(const glm::vec3& minBounds, const glm::vec3& maxBounds) {
        m_minBounds = minBounds;
        m_maxBounds = maxBounds;
        m_enableBounds = true;
        clampTargetToBounds();
    }

    void disableBounds() {
        m_enableBounds = false;
    }

    void clampTargetToBounds() {
        if (!m_enableBounds) return;
        m_target = glm::clamp(m_target, m_minBounds, m_maxBounds);
    }

    // ========== UNDO/REDO ==========
    void saveState() {
        if (m_undoStack.size() >= MAX_HISTORY) {
            // Remover el más antiguo (implementación simple)
            std::stack<CameraState> temp;
            while (m_undoStack.size() > MAX_HISTORY - 1) {
                if (m_undoStack.size() > 1) {
                    temp.push(m_undoStack.top());
                }
                m_undoStack.pop();
            }
            while (!temp.empty()) {
                m_undoStack.push(temp.top());
                temp.pop();
            }
        }
        
        m_undoStack.push(getCurrentState());
        
        // Limpiar redo stack al hacer nueva acción
        while (!m_redoStack.empty()) m_redoStack.pop();
    }

    void undo() {
        if (m_undoStack.size() <= 1) return; // Mantener al menos el estado actual
        
        m_redoStack.push(m_undoStack.top());
        m_undoStack.pop();
        
        applyState(m_undoStack.top());
    }

    void redo() {
        if (m_redoStack.empty()) return;
        
        CameraState state = m_redoStack.top();
        m_redoStack.pop();
        m_undoStack.push(state);
        
        applyState(state);
    }

    // ========== GUARDAR/CARGAR ESTADO ==========
    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        CameraState state = getCurrentState();
        file.write(reinterpret_cast<const char*>(&state.target), sizeof(glm::vec3));
        file.write(reinterpret_cast<const char*>(&state.distance), sizeof(float));
        file.write(reinterpret_cast<const char*>(&state.yaw), sizeof(float));
        file.write(reinterpret_cast<const char*>(&state.pitch), sizeof(float));
        file.write(reinterpret_cast<const char*>(&state.preset), sizeof(ViewPreset));
        
        file.close();
        return true;
    }

    bool loadFromFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) return false;
        
        CameraState state;
        file.read(reinterpret_cast<char*>(&state.target), sizeof(glm::vec3));
        file.read(reinterpret_cast<char*>(&state.distance), sizeof(float));
        file.read(reinterpret_cast<char*>(&state.yaw), sizeof(float));
        file.read(reinterpret_cast<char*>(&state.pitch), sizeof(float));
        file.read(reinterpret_cast<char*>(&state.preset), sizeof(ViewPreset));
        
        file.close();
        
        applyState(state);
        return true;
    }

    // ========== UTILIDADES ==========
    void reset() {
        m_distance = DEFAULT_DISTANCE;
        m_yaw = DEFAULT_YAW;
        m_pitch = DEFAULT_PITCH;
        m_target = glm::vec3(0.0f, 1.0f, 0.0f);
        m_currentPreset = ViewPreset::FREE;
    }

    CameraState getCurrentState() const {
        CameraState state;
        state.target = m_target;
        state.distance = m_distance;
        state.yaw = m_yaw;
        state.pitch = m_pitch;
        state.preset = m_currentPreset;
        return state;
    }

    void applyState(const CameraState& state) {
        m_target = state.target;
        m_distance = state.distance;
        m_yaw = state.yaw;
        m_pitch = state.pitch;
        m_currentPreset = state.preset;
    }

    // ========== SETTERS Y GETTERS ==========
    void setSmoothFactor(float factor) { m_smoothFactor = std::clamp(factor, 0.0f, 1.0f); }
    void setEnableSmoothing(bool enable) { m_enableSmoothing = enable; }
    void setTransitionDuration(float duration) { m_transitionDuration = std::max(0.1f, duration); }
    void setTarget(const glm::vec3& target) {
        m_target = target;
        m_targetSmooth = target;
    }
    
    const glm::vec3& getTarget() const { return m_target; }
    float getDistance() const { return m_distance; }
    float getYaw() const { return m_yaw; }
    float getPitch() const { return m_pitch; }
    ViewPreset getCurrentPreset() const { return m_currentPreset; }
    bool isTransitioning() const { return m_isTransitioning; }
    bool canUndo() const { return m_undoStack.size() > 1; }
    bool canRedo() const { return !m_redoStack.empty(); }
    
    // Debug info
    void printDebugInfo() const {
        std::cout << "=== Camera Debug ===" << std::endl;
        std::cout << "Target: (" << m_target.x << ", " << m_target.y << ", " << m_target.z << ")" << std::endl;
        std::cout << "Distance: " << m_distance << std::endl;
        std::cout << "Yaw: " << m_yaw << " | Pitch: " << m_pitch << std::endl;
        std::cout << "Preset: " << static_cast<int>(m_currentPreset) << std::endl;
        std::cout << "Undo stack: " << m_undoStack.size() << " | Redo stack: " << m_redoStack.size() << std::endl;
    }
};