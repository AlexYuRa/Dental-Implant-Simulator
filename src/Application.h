/**
 * @file Application.h
 * @brief Clase principal de la aplicación (versión original)
 * 
 * Controla el ciclo de vida completo del programa:
 * - Inicialización de GLFW, OpenGL y recursos
 * - Bucle principal (input → update → render)
 * - Manejo de cámara, modelos, animación, interacción
 * 
 * NOTA: Esta es la versión monolítica original (~2000 líneas).
 * Ver ApplicationRefactored.h para la versión modular POO.
 * 
 * Contiene:
 * - Sistema de cámara orbital
 * - Carga de modelos GLB con ASSIMP
 * - Animación esquelética
 * - Sistema de selección y transformación de objetos
 * - Renderizado con shaders PBR
 */

#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <memory>
#include <algorithm>
#include <vector>

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "Animation.h"
#include "Animator.h"
#include "InteractionSystem.h"

class Application {
private:

    // Constantes
    static constexpr unsigned int SCR_WIDTH = 1280;
    static constexpr unsigned int SCR_HEIGHT = 720;
    static constexpr float AXIS_LENGTH = 3.0f;
    static constexpr float DISTANCE_ADJUSTMENT_STEP = 0.05f;
    static constexpr float STOP_FRAME = 155.0f;
    static constexpr float TOTAL_ANIMATION_FRAMES = 234.0f;

    // Ventana y recursos OpenGL
    GLFWwindow* m_window;
    unsigned int m_axisVAO, m_axisVBO;
    unsigned int m_hudVAO, m_hudVBO;  // Para renderizar HUD

    // Componentes principales
    Camera m_camera;
    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<Shader> m_staticShader;
    std::unique_ptr<Shader> m_axisShader;
    std::unique_ptr<Shader> m_hudShader;  // Shader para HUD

    // Sistema de drag & drop integrado
    struct DragState {
        bool isDragging;
        bool isStatic;  // true = staticModels, false = animatedModels
        int modelIndex;
        glm::vec3 dragOffset;
        float dragPlaneDistance;

        DragState() : isDragging(false), isStatic(true), modelIndex(-1),
                        dragOffset(0.0f), dragPlaneDistance(0.0f) {}
    };
    DragState m_dragState;

    // Sistema de doble clic para resetear modelos
    double m_lastLeftClickTime = 0.0;
    double m_doubleClickThreshold = 0.3; // 300ms para considerar doble clic

    // === SISTEMA DE TRANSFORMACIÓN DE OBJETOS ===
    enum class TransformMode {
        NONE,       // Sin modo activo
        TRANSLATE,  // Mover (1)
        ROTATE      // Rotar (2)
    };
    
    enum class TransformAxis {
        ALL,    // Todos los ejes (libre)
        X,      // Solo eje X
        Y,      // Solo eje Y
        Z       // Solo eje Z
    };
    
    struct SelectionState {
        bool hasSelection;
        bool isStatic;
        int modelIndex;
        TransformMode mode;
        TransformAxis axis;
        float transformSpeed;
        
        // Para mostrar controles en pantalla
        bool showControls;
        
        SelectionState() : hasSelection(false), isStatic(true), modelIndex(-1),
                            mode(TransformMode::NONE), axis(TransformAxis::ALL),
                            transformSpeed(5.0f), showControls(false) {}
    };
    SelectionState m_selection;

    // === SISTEMA DE MODELOS MÚLTIPLES ===

    // Estructura para animación por keyframes (transformaciones)
    struct TransformKeyframe {
        glm::vec3 position;
        glm::quat rotationQuat;  // Quaternion para rotación correcta
        glm::vec3 scale;

        TransformKeyframe(glm::vec3 pos = glm::vec3(0.0f),
                         glm::quat rot = glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                         glm::vec3 scl = glm::vec3(1.0f))
            : position(pos), rotationQuat(rot), scale(scl) {}
    };

    struct ModelData {
        std::unique_ptr<Model> model;
        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;  // Rotación en grados (x, y, z)
        bool isAnimated;
        std::unique_ptr<Animation> animation;
        std::unique_ptr<Animator> animator;

        // Drag & drop
        bool isDraggable;
        float autoMoveDistance;  // Para modelos animados que se mueven automáticamente

        // Sistema de colisiones
        bool isCollider;  // true = actúa como barrera para otros objetos

        // Guardar estado original para reset
        glm::vec3 originalPosition;
        glm::vec3 originalScale;
        glm::vec3 originalRotation;

        // Sistema de animación por keyframes (para objetos interactivos)
        bool hasKeyframeAnimation;
        std::vector<TransformKeyframe> keyframes;
        int currentKeyframeIndex;
        int targetKeyframeIndex;
        bool isAnimatingKeyframes;
        float keyframeAnimationTime;
        float keyframeAnimationDuration;  // Duración de transición entre keyframes
        glm::vec3 animPosOffset;          // OFFSET de posición (0 en base, delta en destino)
        glm::quat animRotQuat;            // Quaternion de rotación interpolado
        
        // Transformación ACTUAL interpolada (valores absolutos de Blender)
        glm::vec3 animCurrentPos;         // Posición actual interpolada
        glm::quat animCurrentRot;         // Rotación actual interpolada
        glm::vec3 animCurrentScale;       // Escala actual interpolada
        
        // Contador de clics para regresar (requiere 2 clics para ir de 1 a 0)
        int returnClickCount;

        ModelData()
            : position(0.0f), scale(1.0f), rotation(0.0f), isAnimated(false)
            , isDraggable(false), autoMoveDistance(0.0f), isCollider(false)
            , originalPosition(0.0f), originalScale(1.0f), originalRotation(0.0f)
            , hasKeyframeAnimation(false), currentKeyframeIndex(0), targetKeyframeIndex(0)
            , isAnimatingKeyframes(false), keyframeAnimationTime(0.0f)
            , keyframeAnimationDuration(1.0f), animPosOffset(0.0f)
            , animRotQuat(1.0f, 0.0f, 0.0f, 0.0f)
            , animCurrentPos(0.0f), animCurrentRot(1.0f, 0.0f, 0.0f, 0.0f), animCurrentScale(1.0f)
            , returnClickCount(0) {}
    };

    std::vector<ModelData> m_staticModels;   // Lista de modelos estáticos
    std::vector<ModelData> m_animatedModels; // Lista de modelos animados

    // Estado de la aplicación
    float m_deltaTime;
    float m_lastFrame;
    float m_distancePerCycle;
    float m_totalCycles;
    float m_pauseStartTick;
    bool m_animationPaused;        // Estado de pausa de la animación
    bool m_spaceKeyPressed;        // Para evitar múltiples toggles
    
    // Control de teclas para transformaciones (evitar repetición)
    bool m_keyG_pressed;
    bool m_keyR_pressed;
    bool m_keyS_pressed;
    bool m_keyX_pressed;
    bool m_keyY_pressed;
    bool m_keyZ_pressed;

    // Control de teclas para cámara (evitar repetición)
    bool m_keyU_pressed;
    bool m_keyI_pressed;
    bool m_keyK_pressed;
    bool m_keyL_pressed;
    bool m_keyT_pressed;
    bool m_keyNumpad1_pressed;
    bool m_keyNumpad2_pressed;
    bool m_keyNumpad3_pressed;
    bool m_keyNumpad4_pressed;
    bool m_keyNumpad5_pressed;
    bool m_keyNumpad6_pressed;
    bool m_keyNumpad7_pressed;
    bool m_keyNumpad8_pressed;

    // Puntero estático para callbacks
    static Application* s_instance;

public:
    Application()
        : m_window(nullptr)
        , m_axisVAO(0)
        , m_axisVBO(0)
        , m_camera(glm::vec3(0.0f, 1.5f, 0.0f), SCR_WIDTH, SCR_HEIGHT)  // Mejor posición inicial
        , m_deltaTime(0.0f)
        , m_lastFrame(0.0f)
        , m_distancePerCycle(2.0f)
        , m_totalCycles(0.0f)
        , m_pauseStartTick(155.0f)
        , m_animationPaused(false)
        , m_spaceKeyPressed(false)
        , m_keyG_pressed(false)
        , m_keyR_pressed(false)
        , m_keyS_pressed(false)
        , m_keyX_pressed(false)
        , m_keyY_pressed(false)
        , m_keyZ_pressed(false)
        , m_keyU_pressed(false)
        , m_keyI_pressed(false)
        , m_keyK_pressed(false)
        , m_keyL_pressed(false)
        , m_keyT_pressed(false)
        , m_keyNumpad1_pressed(false)
        , m_keyNumpad2_pressed(false)
        , m_keyNumpad3_pressed(false)
        , m_keyNumpad4_pressed(false)
        , m_keyNumpad5_pressed(false)
        , m_keyNumpad6_pressed(false)
        , m_keyNumpad7_pressed(false)
        , m_keyNumpad8_pressed(false)
        , m_dragState()
        , m_selection()
    {
        s_instance = this;
    }

    ~Application() {
        cleanup();
    }

    bool initialize() {
        if (!initWindow()) return false;
        if (!initOpenGL()) return false;
        loadResources();
        setupAxisRenderer();
        updateCameraTargetToBody();
        printControls();
        return true;
    }

    void run() {
        while (!glfwWindowShouldClose(m_window)) {
            updateDeltaTime();
            processInput();
            update();
            render();
            glfwSwapBuffers(m_window);
            glfwPollEvents();
        }
    }

private:
    bool initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
                                    "Simulador de Implantes Dentales", nullptr, nullptr);
        if (!m_window) {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return false;
        }

        glfwMakeContextCurrent(m_window);
        glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
        glfwSetCursorPosCallback(m_window, mouseCallback);
        glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
        glfwSetScrollCallback(m_window, scrollCallback);

        return true;
    }

    bool initOpenGL() {
        if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return false;
        }

        glEnable(GL_DEPTH_TEST);

        // SOLUCIÓN TEMPORAL: Deshabilitar face culling para renderizar ambos lados
        // Esto soluciona el problema de normales invertidas en la cara
        // Si quieres optimizar después, puedes habilitar: glEnable(GL_CULL_FACE);
        // glDisable(GL_CULL_FACE);

        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
        return true;
    }

    void loadResources() {
        std::cout << "Cargando recursos..." << std::endl;

        m_shader = std::make_unique<Shader>("shaders/skeletal_animation.vert", "shaders/skeletal_animation.frag");
        m_staticShader = std::make_unique<Shader>("shaders/static.vert", "shaders/static.frag");
        m_axisShader = std::make_unique<Shader>("shaders/axis.vert", "shaders/axis.frag");
        m_hudShader = std::make_unique<Shader>("shaders/hud.vert", "shaders/hud.frag");
        
        // Configurar VAO para HUD
        setupHUD();

        // ========================================================================
        // CARGA DE MODELOS DE LA ESCENA
        // ========================================================================
        std::cout << "\n=== CARGANDO MODELOS DE LA ESCENA ===" << std::endl;

        // Consultorio (ambiente)
        addStaticModel("assets/models/consultorio.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // TALADRO con animación por keyframes
        addStaticModel("assets/models/TALADRO.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // CABEZAL_IMPLANTE1 con animación por keyframes
        addStaticModel("assets/models/CABEZAL_IMPLANTE1.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // IMPLANTE_BASE1 con animación por keyframes
        addStaticModel("assets/models/IMPLANTE_BASE1.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // CABEZAL1 con animación por keyframes
        addStaticModel("assets/models/CABEZAL1.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Cuerpo del paciente (estático)
        addStaticModel("assets/models/CUERPO.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // Capa estéril
        addStaticModel("assets/models/CAPA ESTERIL1.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // Dentaduras
        addStaticModel("assets/models/doscaras1.001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/doscaras1.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // Mobiliario
        addStaticModel("assets/models/SILLA GRIS.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/MUEBLE BLANCO.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/MUEBLE NEGRO.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/LAVADERO.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/ESTANTE1.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // Lámparas
        addStaticModel("assets/models/LamparaFotocurado.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/MangoLuz.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/LINTARNA1.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // Decoración
        addStaticModel("assets/models/CUADRO.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/CORTINA3.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/BOTELLAS.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        addStaticModel("assets/models/SOPORTEX.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // Instrumentos
        addStaticModel("assets/models/Bandeja1.001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/Bisturi1.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/Bisturi3.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/alicate1.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/tijera1.001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/tijera1.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/aguja1.001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/forcet_1_(creo).001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/Hisopo.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Más agujas
        addStaticModel("assets/models/aguja1.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/aguja1.003.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/aguja1.004.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Más alicates
        addStaticModel("assets/models/alicate_3.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Más bandejas
        addStaticModel("assets/models/Bandeja1.003.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Más bisturíes
        addStaticModel("assets/models/Bisturi4.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Tercera dentadura
        addStaticModel("assets/models/doscaras1.003.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);

        // Más fórceps
        addStaticModel("assets/models/_forcet_2_(creo).001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/forcet_3_(cre).001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Jaladores
        addStaticModel("assets/models/jalador1_.001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/jalador1_.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        // Más tijera
        addStaticModel("assets/models/tijera1.006.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/tijera2.001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/tijera2.003.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/tijera3.001.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/tijera3.004.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Series de implantes con keyframes
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.003.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.004.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.005.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.006.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.007.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.008.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.009.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.010.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.011.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/CABEZAL-IMPLANTE1.012.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        addStaticModel("assets/models/IMPLANTE1.002.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.003.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.004.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.005.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.006.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.007.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.008.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.009.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.010.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.011.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);
        addStaticModel("assets/models/IMPLANTE1.012.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        // Otros modelos faltantes
        addStaticModel("assets/models/DENTADURA.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);
        addStaticModel("assets/models/MESA DE BOTELLAS.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);
        addStaticModel("assets/models/SOPORTE.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), false);
        addStaticModel("assets/models/TAPA.glb",
                        glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.0f), true);

        std::cout << "=== MODELOS CARGADOS ===" << std::endl;
        std::cout << "Total modelos estaticos: " << m_staticModels.size() << std::endl;
        std::cout << "Total modelos animados: " << m_animatedModels.size() << std::endl;

        // ========================================================================
        // CONFIGURAR ANIMACIONES POR KEYFRAMES
        // ========================================================================
        std::cout << "\n=== CONFIGURANDO ANIMACIONES DE KEYFRAMES ===" << std::endl;
        std::cout << "Total de modelos estaticos: " << m_staticModels.size() << std::endl;

        // Índices de modelos:
        // 0 = consultorio (sin animación)
        // 1 = TALADRO
        // 2 = CABEZAL_IMPLANTE1
        // 3 = IMPLANTE_BASE1
        // 4 = CABEZAL1

        // Configurar animaciones para todos los modelos con keyframes
        for (int i = 1; i <= 4; i++) {
            if (i < (int)m_staticModels.size()) {
                std::cout << "[Keyframes] Configurando modelo indice " << i << "..." << std::endl;
                setupKeyframeAnimationFromModel(i, 0.5f);  // Animación de 0.5 segundos
            }
        }
    }


    void setupAxisRenderer() {
        float axisVertices[] = {
            0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Eje X (Rojo)
            AXIS_LENGTH, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Eje Y (Verde)
            0.0f, AXIS_LENGTH, 0.0f,  0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Eje Z (Azul)
            0.0f, 0.0f, AXIS_LENGTH,  0.0f, 0.0f, 1.0f
        };

        glGenVertexArrays(1, &m_axisVAO);
        glGenBuffers(1, &m_axisVBO);
        glBindVertexArray(m_axisVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_axisVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(axisVertices), axisVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }
    
    void setupHUD() {
        // Quad para renderizar elementos del HUD
        float hudVertices[] = {
            // pos        // tex
                -1.0f,  1.0f,  0.0f, 1.0f,
                -1.0f, -1.0f,  0.0f, 0.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                -1.0f,  1.0f,  0.0f, 1.0f,
                1.0f, -1.0f,  1.0f, 0.0f,
                1.0f,  1.0f,  1.0f, 1.0f
        };
        
        glGenVertexArrays(1, &m_hudVAO);
        glGenBuffers(1, &m_hudVBO);
        glBindVertexArray(m_hudVAO);
        glBindBuffer(GL_ARRAY_BUFFER, m_hudVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(hudVertices), hudVertices, GL_DYNAMIC_DRAW);
        
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
    }
    
    // Renderiza un rectángulo en coordenadas de pantalla normalizadas (-1 a 1)
    void renderQuad(float x, float y, float width, float height, glm::vec4 color) {
        float vertices[] = {
            x,         y + height,  0.0f, 1.0f,
            x,         y,           0.0f, 0.0f,
            x + width, y,           1.0f, 0.0f,
            x,         y + height,  0.0f, 1.0f,
            x + width, y,           1.0f, 0.0f,
            x + width, y + height,  1.0f, 1.0f
        };
        
        glBindBuffer(GL_ARRAY_BUFFER, m_hudVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        
        m_hudShader->use();
        m_hudShader->setVec4("color", color);
        m_hudShader->setBool("useTexture", false);
        
        glBindVertexArray(m_hudVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
    
    // Renderiza el HUD con información del objeto seleccionado
    void renderHUD() {
        if (!m_selection.hasSelection) return;
        
        // Desactivar depth test para que el HUD siempre se vea
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // Panel de fondo semi-transparente en la esquina superior izquierda
        renderQuad(-0.98f, 0.55f, 0.55f, 0.42f, glm::vec4(0.0f, 0.0f, 0.0f, 0.7f));
        
        // Indicadores de modo (barras de colores)
        float barY = 0.90f;
        float barHeight = 0.06f;
        float barWidth = 0.15f;
        float spacing = 0.08f;
        
        // Modo Trasladar (1) - Verde si activo
        glm::vec4 translateColor = (m_selection.mode == TransformMode::TRANSLATE)
            ? glm::vec4(0.2f, 0.9f, 0.2f, 1.0f) : glm::vec4(0.3f, 0.3f, 0.3f, 0.8f);
        renderQuad(-0.95f, barY, barWidth * 1.5f, barHeight, translateColor);

        // Modo Rotar (2) - Azul si activo
        glm::vec4 rotateColor = (m_selection.mode == TransformMode::ROTATE)
            ? glm::vec4(0.2f, 0.5f, 1.0f, 1.0f) : glm::vec4(0.3f, 0.3f, 0.3f, 0.8f);
        renderQuad(-0.95f + (barWidth * 1.5f) + 0.02f, barY, barWidth * 1.5f, barHeight, rotateColor);
        
        // Indicadores de eje restringido
        float axisY = barY - spacing - 0.02f;
        float axisWidth = 0.08f;
        
        // Eje X - Rojo
        glm::vec4 xColor = (m_selection.axis == TransformAxis::X || m_selection.axis == TransformAxis::ALL) 
            ? glm::vec4(1.0f, 0.2f, 0.2f, 1.0f) : glm::vec4(0.4f, 0.2f, 0.2f, 0.5f);
        renderQuad(-0.95f, axisY, axisWidth, barHeight, xColor);
        
        // Eje Y - Verde
        glm::vec4 yColor = (m_selection.axis == TransformAxis::Y || m_selection.axis == TransformAxis::ALL) 
            ? glm::vec4(0.2f, 1.0f, 0.2f, 1.0f) : glm::vec4(0.2f, 0.4f, 0.2f, 0.5f);
        renderQuad(-0.95f + axisWidth + 0.02f, axisY, axisWidth, barHeight, yColor);
        
        // Eje Z - Azul
        glm::vec4 zColor = (m_selection.axis == TransformAxis::Z || m_selection.axis == TransformAxis::ALL) 
            ? glm::vec4(0.2f, 0.2f, 1.0f, 1.0f) : glm::vec4(0.2f, 0.2f, 0.4f, 0.5f);
        renderQuad(-0.95f + (axisWidth + 0.02f) * 2, axisY, axisWidth, barHeight, zColor);
        
        // Borde indicador de selección activa
        renderQuad(-0.99f, 0.54f, 0.02f, 0.44f, glm::vec4(1.0f, 0.8f, 0.0f, 1.0f));
        
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

    void updateCameraTargetToBody() {
        // Buscar el modelo CUERPO.glb y actualizar el target de la cámara
        for (const auto& modelData : m_staticModels) {
            // Verificar si el path contiene "CUERPO.glb"
            // No podemos acceder al path directamente, así que usamos el índice conocido
            // El CUERPO está en la posición aproximada después de CAPA ESTERIL
            break;
        }

        // Alternativa: usar un índice conocido o buscar por propiedades
        // Como CUERPO.glb es uno de los últimos modelos cargados, busquemos hacia el final
        if (m_staticModels.size() >= 2) {
            // CUERPO.glb debería estar cerca del final (antes de las 2 dentaduras)
            for (int i = m_staticModels.size() - 4; i < m_staticModels.size(); i++) {
                if (i >= 0 && i < m_staticModels.size()) {
                    const auto& modelData = m_staticModels[i];
                    // Obtener la posición del modelo incluyendo su transformación root
                    glm::mat4 rootTransform = modelData.model->GetRootTransform();
                    glm::vec3 modelCenter = glm::vec3(rootTransform[3]);
                    glm::vec3 bodyPosition = modelData.position + modelCenter;

                    // Actualizar el target de la cámara
                    // Usamos la posición Y del cuerpo para centrar en la cabeza/boca
                    m_camera.setTarget(bodyPosition);
                    std::cout << "[Camera] Target actualizado a posicion del cuerpo: ("
                              << bodyPosition.x << ", " << bodyPosition.y << ", "
                              << bodyPosition.z << ")" << std::endl;
                    break;
                }
            }
        }
    }

    void printControls() {
        std::cout << "Inicializacion completa. Iniciando simulador..." << std::endl;
        std::cout << "\n============================================" << std::endl;
        std::cout << "  SIMULADOR DE IMPLANTES DENTALES" << std::endl;
        std::cout << "============================================\n" << std::endl;

        std::cout << "=== CONTROLES DE CAMARA BASICOS ===" << std::endl;
        std::cout << "- Click DERECHO + Arrastrar: Rotar camara (orbital)" << std::endl;
        std::cout << "- Click MEDIO + Arrastrar: Pan (desplazar vista)" << std::endl;
        std::cout << "- Scroll: Zoom in/out" << std::endl;
        std::cout << "- WASD: Mover (relativo a orientacion de camara)" << std::endl;
        std::cout << "- Q/E: Subir/Bajar camara" << std::endl;
        std::cout << "- SHIFT + WASD: Mover mas rapido (3x)" << std::endl;
        std::cout << "- CTRL + WASD: Mover mas lento (precision)" << std::endl;
        std::cout << "- R: Resetear camara a posicion inicial" << std::endl;

        std::cout << "\n=== VISTAS MEDICAS (TECLADO NUMERICO) ===" << std::endl;
        std::cout << "- Numpad 1: Vista FRONTAL" << std::endl;
        std::cout << "- Numpad 2: Vista LATERAL DERECHA" << std::endl;
        std::cout << "- Numpad 3: Vista LATERAL IZQUIERDA" << std::endl;
        std::cout << "- Numpad 4: Vista SUPERIOR (oclusal superior)" << std::endl;
        std::cout << "- Numpad 5: Vista INFERIOR (oclusal inferior)" << std::endl;
        std::cout << "- Numpad 6: Vista OCLUSAL (directa)" << std::endl;
        std::cout << "- Numpad 7: Vista PANORAMICA (toda la boca)" << std::endl;
        std::cout << "- Numpad 8: Mostrar debug info de camara" << std::endl;

        std::cout << "\n=== FUNCIONES AVANZADAS DE CAMARA ===" << std::endl;
        std::cout << "- U: Deshacer movimiento de camara (Undo)" << std::endl;
        std::cout << "- I: Rehacer movimiento de camara (Redo)" << std::endl;
        std::cout << "- K: Guardar posicion de camara (camera_state.dat)" << std::endl;
        std::cout << "- L: Cargar estado de camara guardado" << std::endl;
        std::cout << "- T: Toggle suavizado ON/OFF" << std::endl;


        std::cout << "\n=== CONTROLES DE OBJETOS ===" << std::endl;
        std::cout << "- Click IZQUIERDO: Seleccionar y arrastrar objeto" << std::endl;
        std::cout << "  (Al soltar se activa el panel de transformacion)" << std::endl;
        std::cout << "\n=== OBJETOS CON ANIMACION DE KEYFRAMES ===" << std::endl;
        std::cout << "- Click IZQUIERDO (en keyframe 0): Anima al ultimo keyframe" << std::endl;
        std::cout << "- Click IZQUIERDO (en keyframe N): Permite interactividad/arrastre" << std::endl;
        std::cout << "- SHIFT + Click DERECHO: Regresar al keyframe inicial" << std::endl;
        
        std::cout << "\n=== TRANSFORMACIONES (con objeto seleccionado) ===" << std::endl;
        std::cout << "- 1: Modo TRASLADAR (barra verde en HUD)" << std::endl;
        std::cout << "- 2: Modo ROTAR (barra azul en HUD)" << std::endl;
        std::cout << "- X/Y/Z: Restringir a eje especifico (presionar de nuevo = todos)" << std::endl;
        std::cout << "\nTRASLACION:" << std::endl;
        std::cout << "  Izq/Der: Mover en X | Arriba/Abajo: Mover en Z | PgUp/PgDn: Mover en Y" << std::endl;
        std::cout << "ROTACION:" << std::endl;
        std::cout << "  Izq/Der: Rotar en Y | Arriba/Abajo: Rotar en X | PgUp/PgDn: Rotar en Z" << std::endl;
        std::cout << "- SHIFT: Transformar 3x mas rapido" << std::endl;
        std::cout << "- ESC: Deseleccionar objeto" << std::endl;

        std::cout << "\n=== HUD (esquina superior izquierda) ===" << std::endl;
        std::cout << "- Barras superiores: Modo activo (Verde=Trasladar, Azul=Rotar)" << std::endl;
        std::cout << "- Barras inferiores: Eje activo (Rojo=X, Verde=Y, Azul=Z)" << std::endl;
        
        std::cout << "\n=== EJES DE COORDENADAS ===" << std::endl;
        std::cout << "- Eje X: ROJO (horizontal)" << std::endl;
        std::cout << "- Eje Y: VERDE (vertical)" << std::endl;
        std::cout << "- Eje Z: AZUL (profundidad)\n" << std::endl;
    }

    void updateDeltaTime() {
        float currentFrame = glfwGetTime();
        m_deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;
    }

    // === SISTEMA DE MODELOS MÚLTIPLES - FUNCIONES HELPER ===

    // Agregar modelo estático (sin animación)
    void addStaticModel(const std::string& filePath,
                        glm::vec3 position = glm::vec3(0.0f),
                        glm::vec3 scale = glm::vec3(1.0f),
                        glm::vec3 rotation = glm::vec3(0.0f),
                        bool isDraggable = false,
                        bool isCollider = false) {
        ModelData data;
        data.model = std::make_unique<Model>(filePath);
        data.position = position;
        data.scale = scale;
        data.rotation = rotation;
        data.isAnimated = false;
        data.isDraggable = isDraggable;
        data.isCollider = isCollider;
        // Guardar estado original
        data.originalPosition = position;
        data.originalScale = scale;
        data.originalRotation = rotation;
        m_staticModels.push_back(std::move(data));
        std::cout << "Modelo estatico agregado: " << filePath
                    << " - Escala: (" << scale.x << ", " << scale.y << ", " << scale.z << ")"
                    << (isDraggable ? " (arrastrable)" : "")
                    << (isCollider ? " (colisionador)" : "") << std::endl;
    }

    // Agregar modelo animado (con esqueleto)
    void addAnimatedModel(const std::string& filePath,
                            glm::vec3 position = glm::vec3(0.0f),
                            glm::vec3 scale = glm::vec3(1.0f),
                            glm::vec3 rotation = glm::vec3(0.0f),
                            bool isDraggable = false) {
        ModelData data;
        data.model = std::make_unique<Model>(filePath);
        data.animation = std::make_unique<Animation>(filePath, data.model.get());
        data.animator = std::make_unique<Animator>(data.animation.get());
        data.position = position;
        data.scale = scale;
        data.rotation = rotation;
        data.isAnimated = true;
        data.isDraggable = isDraggable;
        // Guardar estado original
        data.originalPosition = position;
        data.originalScale = scale;
        data.originalRotation = rotation;
        m_animatedModels.push_back(std::move(data));
        std::cout << "Modelo animado agregado: " << filePath
                    << (isDraggable ? " (arrastrable)" : "") << std::endl;
    }

    // Extraer posición, rotación (en grados) y escala desde una matriz de transformación
    void extractTransformFromMatrix(const glm::mat4& matrix, glm::vec3& position, glm::vec3& rotationDeg, glm::vec3& scale) {
        // Extraer posición
        position = glm::vec3(matrix[3]);

        // Extraer escala
        scale.x = glm::length(glm::vec3(matrix[0]));
        scale.y = glm::length(glm::vec3(matrix[1]));
        scale.z = glm::length(glm::vec3(matrix[2]));

        // Crear matriz de rotación eliminando escala
        glm::mat4 rotationMatrix = matrix;
        rotationMatrix[0] /= scale.x;
        rotationMatrix[1] /= scale.y;
        rotationMatrix[2] /= scale.z;
        rotationMatrix[3] = glm::vec4(0, 0, 0, 1);

        // Extraer ángulos de Euler (en radianes) de la matriz de rotación
        // Orden: YXZ (Yaw, Pitch, Roll)
        float sy = sqrt(rotationMatrix[0][0] * rotationMatrix[0][0] + rotationMatrix[1][0] * rotationMatrix[1][0]);
        bool singular = sy < 1e-6;

        float x, y, z;
        if (!singular) {
            x = atan2(rotationMatrix[2][1], rotationMatrix[2][2]);
            y = atan2(-rotationMatrix[2][0], sy);
            z = atan2(rotationMatrix[1][0], rotationMatrix[0][0]);
        } else {
            x = atan2(-rotationMatrix[1][2], rotationMatrix[1][1]);
            y = atan2(-rotationMatrix[2][0], sy);
            z = 0;
        }

        // Convertir de radianes a grados
        rotationDeg.x = glm::degrees(x);
        rotationDeg.y = glm::degrees(y);
        rotationDeg.z = glm::degrees(z);
    }

    // Agregar keyframes extrayéndolos automáticamente del modelo .glb
    void setupKeyframeAnimationFromModel(int modelIndex, float duration = 1.0f) {
        std::cout << "\n[setupKeyframeAnimationFromModel] Iniciando para modelo #" << modelIndex << std::endl;

        if (modelIndex < 0 || modelIndex >= (int)m_staticModels.size()) {
            std::cout << "[setupKeyframeAnimationFromModel] ERROR: Indice fuera de rango" << std::endl;
            return;
        }

        auto& modelData = m_staticModels[modelIndex];

        // Usar la nueva función que devuelve quaterniones directamente
        std::cout << "[setupKeyframeAnimationFromModel] Llamando a GetAnimationKeyframesQuat()..." << std::endl;
        auto rawKeyframes = modelData.model->GetAnimationKeyframesQuat();

        std::cout << "[setupKeyframeAnimationFromModel] Keyframes encontrados: " << rawKeyframes.size() << std::endl;

        if (rawKeyframes.empty()) {
            std::cout << "[Keyframes] ADVERTENCIA: No se encontraron animaciones en el modelo #" << modelIndex << std::endl;
            return;
        }

        // Convertir a nuestro formato TransformKeyframe
        std::vector<TransformKeyframe> keyframes;

        // Obtener rootTransform para comparación
        glm::mat4 rootTransform = modelData.model->GetRootTransform();
        glm::vec3 rootPos = glm::vec3(rootTransform[3]);

        std::cout << "\n========================================" << std::endl;
        std::cout << "  KEYFRAMES EXTRAIDOS DEL MODELO #" << modelIndex << std::endl;
        std::cout << "  RootTransform pos: (" << rootPos.x << ", " << rootPos.y << ", " << rootPos.z << ")" << std::endl;
        std::cout << "========================================" << std::endl;

        for (size_t i = 0; i < rawKeyframes.size(); i++) {
            TransformKeyframe kf;
            kf.position = rawKeyframes[i].position;
            kf.rotationQuat = rawKeyframes[i].rotation;
            kf.scale = rawKeyframes[i].scale;
            keyframes.push_back(kf);

            // Mostrar Euler solo para debug (no lo usamos para animación)
            glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(kf.rotationQuat));
            std::cout << "\n[KEYFRAME " << i << "]" << std::endl;
            std::cout << "  Posicion: (" << kf.position.x << ", " << kf.position.y << ", " << kf.position.z << ")" << std::endl;
            std::cout << "  Diferencia con root: (" << (kf.position.x - rootPos.x) << ", " 
                      << (kf.position.y - rootPos.y) << ", " << (kf.position.z - rootPos.z) << ")" << std::endl;
            std::cout << "  Quaternion: (w=" << kf.rotationQuat.w << ", x=" << kf.rotationQuat.x 
                      << ", y=" << kf.rotationQuat.y << ", z=" << kf.rotationQuat.z << ")" << std::endl;
            std::cout << "  Euler(debug): (" << eulerDeg.x << ", " << eulerDeg.y << ", " << eulerDeg.z << ") grados" << std::endl;
            std::cout << "  Escala  : (" << kf.scale.x << ", " << kf.scale.y << ", " << kf.scale.z << ")" << std::endl;
        }

        std::cout << "\n========================================" << std::endl;
        std::cout << "Total: " << keyframes.size() << " keyframes (duracion: " << duration << "s)" << std::endl;
        std::cout << "========================================\n" << std::endl;

        // Configurar animación de keyframes
        modelData.hasKeyframeAnimation = true;
        modelData.keyframes = keyframes;
        modelData.keyframeAnimationDuration = duration;
        modelData.currentKeyframeIndex = 0;
        modelData.targetKeyframeIndex = 0;
        
        // Iniciar con los valores del keyframe 0 (posición inicial de Blender)
        modelData.animCurrentPos = keyframes[0].position;
        modelData.animCurrentRot = keyframes[0].rotationQuat;
        modelData.animCurrentScale = keyframes[0].scale;
        
        // Legacy (por si se usan en algún lado)
        modelData.animPosOffset = glm::vec3(0.0f);
        modelData.animRotQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        std::cout << "[Keyframes] Modelo en posicion BASE (keyframe 0)" << std::endl;
        if (keyframes.size() >= 2) {
            glm::vec3 posDelta = keyframes[1].position - keyframes[0].position;
            std::cout << "  Al animar -> Delta Pos: (" << posDelta.x << ", " << posDelta.y << ", " << posDelta.z << ")" << std::endl;
        }
    }

    // Agregar keyframes manualmente a un modelo estático (por índice)
    void addKeyframesToStaticModel(int modelIndex, const std::vector<TransformKeyframe>& keyframes, float duration = 1.0f) {
        if (modelIndex < 0 || modelIndex >= (int)m_staticModels.size()) return;

        auto& model = m_staticModels[modelIndex];
        model.hasKeyframeAnimation = true;
        model.keyframes = keyframes;
        model.keyframeAnimationDuration = duration;
        model.currentKeyframeIndex = 0;
        model.targetKeyframeIndex = 0;

        // Para keyframes manuales, inicializar offsets en 0
        model.animPosOffset = glm::vec3(0.0f);
        model.animRotQuat = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);

        std::cout << "[Keyframes] " << keyframes.size() << " keyframes agregados manualmente al modelo estatico #"
                  << modelIndex << " (duracion: " << duration << "s)" << std::endl;
    }

    // Activar animación hacia el siguiente keyframe (para clic en objeto)
    void triggerNextKeyframe(bool isStatic, int modelIndex) {
        ModelData* model = nullptr;

        if (isStatic && modelIndex < (int)m_staticModels.size()) {
            model = &m_staticModels[modelIndex];
        } else if (!isStatic && modelIndex < (int)m_animatedModels.size()) {
            model = &m_animatedModels[modelIndex];
        }

        if (!model || !model->hasKeyframeAnimation || model->keyframes.empty()) return;

        // Ya está animando, ignorar
        if (model->isAnimatingKeyframes) {
            std::cout << "[Keyframe] Click ignorado - ya animando" << std::endl;
            return;
        }

        std::cout << "[Keyframe] Click! currentIndex=" << model->currentKeyframeIndex
                  << " / " << (model->keyframes.size() - 1) << " total keyframes" << std::endl;

        // Avanzar: si está en keyframe 0 -> va al último; si está en el último -> vuelve a 0
        if (model->keyframes.size() >= 2) {
            int lastIndex = model->keyframes.size() - 1;
            int targetIndex;

            if (model->currentKeyframeIndex == 0) {
                // Del keyframe 0 -> ir al último (animar a través de todos)
                targetIndex = lastIndex;
            } else {
                // Si está en cualquier otro keyframe -> volver a 0
                targetIndex = 0;
            }

            model->targetKeyframeIndex = targetIndex;
            model->isAnimatingKeyframes = true;
            model->keyframeAnimationTime = 0.0f;
            std::cout << "[Keyframe] Animando del keyframe " << model->currentKeyframeIndex
                      << " al keyframe " << targetIndex << std::endl;
        }
    }

    // Resetear animación a la posición inicial (para clic derecho)
    void resetKeyframeAnimation(bool isStatic, int modelIndex) {
        ModelData* model = nullptr;

        if (isStatic && modelIndex < (int)m_staticModels.size()) {
            model = &m_staticModels[modelIndex];
        } else if (!isStatic && modelIndex < (int)m_animatedModels.size()) {
            model = &m_animatedModels[modelIndex];
        }

        if (!model || !model->hasKeyframeAnimation || model->keyframes.empty()) return;

        // Ya está animando, ignorar
        if (model->isAnimatingKeyframes) {
            std::cout << "[Keyframe] Click ignorado - ya animando" << std::endl;
            return;
        }

        // Regresar al keyframe inicial (posición 0)
        if (model->keyframes.size() >= 2) {
            model->targetKeyframeIndex = 0;
            model->isAnimatingKeyframes = true;
            model->keyframeAnimationTime = 0.0f;
            std::cout << "[Keyframe] Regresando del keyframe " << model->currentKeyframeIndex
                      << " al keyframe 0 (posicion inicial)" << std::endl;
        }
    }

    // Actualizar todas las animaciones de keyframes
    void updateKeyframeAnimations(float deltaTime) {
        // Actualizar modelos estáticos
        for (size_t idx = 0; idx < m_staticModels.size(); idx++) {
            auto& model = m_staticModels[idx];
            if (!model.hasKeyframeAnimation || !model.isAnimatingKeyframes) continue;

            model.keyframeAnimationTime += deltaTime;
            float t = std::min(model.keyframeAnimationTime / model.keyframeAnimationDuration, 1.0f);

            // Interpolación suave (ease in-out)
            t = t * t * (3.0f - 2.0f * t);

            // Keyframes: desde ACTUAL hacia TARGET (valores ABSOLUTOS)
            const TransformKeyframe& from = model.keyframes[model.currentKeyframeIndex];
            const TransformKeyframe& to = model.keyframes[model.targetKeyframeIndex];

            // Interpolar directamente los valores absolutos de Blender
            model.animCurrentPos = glm::mix(from.position, to.position, t);
            model.animCurrentRot = glm::slerp(from.rotationQuat, to.rotationQuat, t);
            model.animCurrentScale = glm::mix(from.scale, to.scale, t);

            // Debug: mostrar progreso (solo cada 20%)
            static float lastT = -0.2f;
            if (t - lastT >= 0.2f || t >= 1.0f) {
                std::cout << "[Anim " << idx << "] t=" << t 
                          << " pos=(" << model.animCurrentPos.x << ", " << model.animCurrentPos.y << ", " << model.animCurrentPos.z << ")" << std::endl;
                lastT = t;
                if (t >= 1.0f) lastT = -0.2f;
            }

            // Animación completa
            if (t >= 1.0f) {
                model.currentKeyframeIndex = model.targetKeyframeIndex;
                model.isAnimatingKeyframes = false;
                model.keyframeAnimationTime = 0.0f;
                std::cout << "[Keyframe] Animacion completada." << std::endl;
            }
        }

        // Actualizar modelos animados (si alguno tiene keyframes)
        for (auto& model : m_animatedModels) {
            if (!model.hasKeyframeAnimation || !model.isAnimatingKeyframes) continue;

            model.keyframeAnimationTime += deltaTime;
            float t = std::min(model.keyframeAnimationTime / model.keyframeAnimationDuration, 1.0f);

            // Interpolación suave (ease in-out)
            t = t * t * (3.0f - 2.0f * t);

            // Interpolar offsets (igual que modelos estáticos)
            const TransformKeyframe& base = model.keyframes[0];
            const TransformKeyframe& target = model.keyframes[model.targetKeyframeIndex];

            glm::vec3 posDelta = target.position - base.position;
            model.animPosOffset = posDelta * t;

            glm::quat baseRot = base.rotationQuat;
            glm::quat targetRot = target.rotationQuat;
            glm::quat relativeRot = targetRot * glm::inverse(baseRot);
            model.animRotQuat = glm::slerp(glm::quat(1.0f, 0.0f, 0.0f, 0.0f), relativeRot, t);

            // Animación completa
            if (t >= 1.0f) {
                model.currentKeyframeIndex = model.targetKeyframeIndex;
                model.isAnimatingKeyframes = false;
                model.keyframeAnimationTime = 0.0f;
            }
        }
    }

    // Renderizar todos los modelos estáticos
    void renderAllStaticModels() {
        m_staticShader->use();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 10000.0f);
        glm::mat4 view = m_camera.getViewMatrix();
        m_staticShader->setMat4("projection", projection);
        m_staticShader->setMat4("view", view);

        m_staticShader->setVec3("lightPos", glm::vec3(0.0f, 5.0f, 20.0f));
        m_staticShader->setVec3("lightColor", glm::vec3(1.3f, 1.3f, 1.3f));
        m_staticShader->setVec3("viewPos", m_camera.getPosition());

        for (auto& modelData : m_staticModels) {
            glm::mat4 modelMat = glm::mat4(1.0f);
            
            // Obtener transformación raíz del modelo
            glm::mat4 rootTransform = modelData.model->GetRootTransform();
            
            // === MODELOS CON ANIMACIÓN DE KEYFRAMES ===
            // Usar directamente la posición/rotación interpolada del keyframe
            if (modelData.hasKeyframeAnimation && !modelData.keyframes.empty()) {
                // Posición interpolada actual (absoluta de Blender)
                glm::vec3 currentPos = modelData.animCurrentPos;
                glm::quat currentRot = modelData.animCurrentRot;
                glm::vec3 currentScale = modelData.animCurrentScale;
                
                // Construir matriz: Translate * Rotate * Scale
                modelMat = glm::translate(modelMat, currentPos);
                modelMat = modelMat * glm::mat4_cast(currentRot);
                modelMat = glm::scale(modelMat, currentScale);
            }
            // === MODELOS SIN ANIMACIÓN DE KEYFRAMES ===
            else {
                glm::vec3 modelCenter = glm::vec3(rootTransform[3]); // Posición del origen en Blender
                
                // 1. Trasladar a posición final
                modelMat = glm::translate(modelMat, modelData.position);
                
                // 2. Trasladar al centro del modelo para rotar localmente
                modelMat = glm::translate(modelMat, modelCenter);
                
                // 3. Aplicar rotación (ahora rota sobre el centro del modelo)
                modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.x), glm::vec3(1, 0, 0));
                modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.y), glm::vec3(0, 1, 0));
                modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.z), glm::vec3(0, 0, 1));
                
                // 4. Trasladar de vuelta
                modelMat = glm::translate(modelMat, -modelCenter);
                
                // 5. Aplicar escala
                modelMat = glm::scale(modelMat, modelData.scale);
                
                // 6. Aplicar transformación raíz del modelo (posición de Blender)
                modelMat = modelMat * rootTransform;
            }
            
            m_staticShader->setMat4("model", modelMat);

            modelData.model->Draw(*m_staticShader);
        }
    }

    // Renderizar todos los modelos animados
    void renderAllAnimatedModels() {
        m_shader->use();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = m_camera.getViewMatrix();
        m_shader->setMat4("projection", projection);
        m_shader->setMat4("view", view);

        m_shader->setVec3("lightPos", glm::vec3(0.0f, 5.0f, 20.0f));
        m_shader->setVec3("lightColor", glm::vec3(1.3f, 1.3f, 1.3f));
        m_shader->setVec3("viewPos", m_camera.getPosition());

        for (auto& modelData : m_animatedModels) {
            auto& transforms = modelData.animator->GetFinalBoneMatrices();
            for (size_t i = 0; i < transforms.size(); ++i) {
                m_shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
            }

            // Posición = posición base + movimiento automático (en eje Z)
            glm::vec3 finalPosition = modelData.position + glm::vec3(0.0f, 0.0f, modelData.autoMoveDistance);

            glm::mat4 modelMat = glm::mat4(1.0f);
            modelMat = glm::translate(modelMat, finalPosition);
            modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.x), glm::vec3(1, 0, 0));
            modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.y), glm::vec3(0, 1, 0));
            modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.z), glm::vec3(0, 0, 1));
            modelMat = glm::scale(modelMat, modelData.scale);
            m_shader->setMat4("model", modelMat);

            modelData.model->Draw(*m_shader);
        }
    }

    // Actualizar todas las animaciones
    void updateAllAnimations() {
        for (auto& modelData : m_animatedModels) {
            if (modelData.animator) {
                modelData.animator->UpdateAnimation(m_deltaTime);
            }
        }
    }

    // === SISTEMA DE DRAG & DROP INTEGRADO ===

    void handleMouseClick(double mouseX, double mouseY) {
        glm::vec3 rayOrigin = m_camera.getPosition();
        glm::vec3 rayDir = RayCaster::screenToWorldRay(mouseX, mouseY, SCR_WIDTH, SCR_HEIGHT, m_camera);

        float closestDist = FLT_MAX;
        bool foundHit = false;

        // Test contra modelos animados (arrastrables)
        for (size_t i = 0; i < m_animatedModels.size(); ++i) {
            if (!m_animatedModels[i].isDraggable) continue;

            // Calcular la transformación completa (igual que en renderizado)
            glm::vec3 finalPos = m_animatedModels[i].position +
                                glm::vec3(0.0f, 0.0f, m_animatedModels[i].autoMoveDistance);
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, finalPos);
            transform = glm::rotate(transform, glm::radians(m_animatedModels[i].rotation.x), glm::vec3(1, 0, 0));
            transform = glm::rotate(transform, glm::radians(m_animatedModels[i].rotation.y), glm::vec3(0, 1, 0));
            transform = glm::rotate(transform, glm::radians(m_animatedModels[i].rotation.z), glm::vec3(0, 0, 1));
            transform = glm::scale(transform, m_animatedModels[i].scale);
            
            AABB aabb = RayCaster::calculateTransformedAABB(m_animatedModels[i].model.get(), transform);

            float dist;
            if (RayCaster::rayAABBIntersection(rayOrigin, rayDir, aabb, dist)) {
                if (dist < closestDist) {
                    closestDist = dist;
                    m_dragState.isStatic = false;
                    m_dragState.modelIndex = i;
                    m_dragState.dragPlaneDistance = dist;
                    foundHit = true;
                }
            }
        }

        // Test contra modelos estáticos (arrastrables)
        for (size_t i = 0; i < m_staticModels.size(); ++i) {
            if (!m_staticModels[i].isDraggable) continue;

            glm::mat4 transform = glm::mat4(1.0f);
            
            // Si tiene keyframes, usar transformación absoluta interpolada
            if (m_staticModels[i].hasKeyframeAnimation && !m_staticModels[i].keyframes.empty()) {
                transform = glm::translate(transform, m_staticModels[i].animCurrentPos);
                transform = transform * glm::mat4_cast(m_staticModels[i].animCurrentRot);
                transform = glm::scale(transform, m_staticModels[i].animCurrentScale);
            }
            // Sin keyframes, usar sistema tradicional
            else {
                glm::mat4 rootTransform = m_staticModels[i].model->GetRootTransform();
                glm::vec3 modelCenter = glm::vec3(rootTransform[3]);
                
                transform = glm::translate(transform, m_staticModels[i].position);
                transform = glm::translate(transform, modelCenter);
                transform = glm::rotate(transform, glm::radians(m_staticModels[i].rotation.x), glm::vec3(1, 0, 0));
                transform = glm::rotate(transform, glm::radians(m_staticModels[i].rotation.y), glm::vec3(0, 1, 0));
                transform = glm::rotate(transform, glm::radians(m_staticModels[i].rotation.z), glm::vec3(0, 0, 1));
                transform = glm::translate(transform, -modelCenter);
                transform = glm::scale(transform, m_staticModels[i].scale);
                transform = transform * rootTransform;
            }
            
            AABB aabb = RayCaster::calculateTransformedAABB(m_staticModels[i].model.get(), transform);

            float dist;
            if (RayCaster::rayAABBIntersection(rayOrigin, rayDir, aabb, dist)) {
                if (dist < closestDist) {
                    closestDist = dist;
                    m_dragState.isStatic = true;
                    m_dragState.modelIndex = i;
                    m_dragState.dragPlaneDistance = dist;
                    foundHit = true;
                }
            }
        }

        if (foundHit) {
            // Verificar si el objeto tiene animación por keyframes
            bool hasKeyframes = false;
            int currentKF = 0;
            ModelData* modelPtr = nullptr;
            
            if (m_dragState.isStatic && m_dragState.modelIndex < (int)m_staticModels.size()) {
                modelPtr = &m_staticModels[m_dragState.modelIndex];
                hasKeyframes = modelPtr->hasKeyframeAnimation;
                currentKF = modelPtr->currentKeyframeIndex;
            } else if (!m_dragState.isStatic && m_dragState.modelIndex < (int)m_animatedModels.size()) {
                modelPtr = &m_animatedModels[m_dragState.modelIndex];
                hasKeyframes = modelPtr->hasKeyframeAnimation;
                currentKF = modelPtr->currentKeyframeIndex;
            }

            // Si tiene keyframes: gestionar comportamiento según keyframe actual
            if (hasKeyframes && modelPtr) {
                int lastKeyframeIndex = modelPtr->keyframes.size() - 1;

                // CASO 1: Está en keyframe 0 → Animar al último keyframe
                if (currentKF == 0) {
                    triggerNextKeyframe(m_dragState.isStatic, m_dragState.modelIndex);
                    std::cout << "[Interaccion] Animando del keyframe 0 al keyframe " << lastKeyframeIndex << std::endl;
                    return;  // No iniciar drag, solo animar
                }
                // CASO 2: Está en cualquier otro keyframe → Permitir interactividad
                // Nota: El doble clic para regresar se maneja automáticamente en mouseButtonCallback
                else {
                    std::cout << "[Interaccion] Permitiendo interactividad en keyframe " << currentKF << std::endl;
                    // Continuar al código de drag abajo (no hacer return)
                }
            }

            // Comportamiento normal de drag
            glm::vec3 hitPoint = RayCaster::projectToViewPlane(rayOrigin, rayDir, closestDist);
            if (m_dragState.isStatic) {
                // Para modelos con keyframes, usar la posición animada actual
                if (m_staticModels[m_dragState.modelIndex].hasKeyframeAnimation) {
                    m_dragState.dragOffset = m_staticModels[m_dragState.modelIndex].animCurrentPos - hitPoint;
                } else {
                    m_dragState.dragOffset = m_staticModels[m_dragState.modelIndex].position - hitPoint;
                }
            } else {
                m_dragState.dragOffset = m_animatedModels[m_dragState.modelIndex].position - hitPoint;
            }
            m_dragState.isDragging = true;
            std::cout << "Objeto seleccionado: "
                        << (m_dragState.isStatic ? "Estatico" : "Animado")
                        << " [" << m_dragState.modelIndex << "]" << std::endl;
        }
    }

    // Calcula el AABB transformado de un modelo (misma lógica que renderizado)
    AABB getTransformedAABB(const ModelData& modelData) {
        glm::mat4 transform = glm::mat4(1.0f);
        glm::mat4 rootTransform = modelData.model->GetRootTransform();
        glm::vec3 modelCenter = glm::vec3(rootTransform[3]); // Posición del origen en Blender
        
        // 1. Trasladar a posición final
        transform = glm::translate(transform, modelData.position);
        // 2. Trasladar al centro del modelo para rotar localmente
        transform = glm::translate(transform, modelCenter);
        // 3. Aplicar rotación
        transform = glm::rotate(transform, glm::radians(modelData.rotation.x), glm::vec3(1, 0, 0));
        transform = glm::rotate(transform, glm::radians(modelData.rotation.y), glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, glm::radians(modelData.rotation.z), glm::vec3(0, 0, 1));
        // 4. Trasladar de vuelta
        transform = glm::translate(transform, -modelCenter);
        // 5. Aplicar escala
        transform = glm::scale(transform, modelData.scale);
        // 6. Aplicar transformación raíz
        transform = transform * rootTransform;
        
        return RayCaster::calculateTransformedAABB(modelData.model.get(), transform);
    }

    // Verifica si dos AABBs colisionan
    bool aabbIntersects(const AABB& a, const AABB& b) {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
                (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
                (a.min.z <= b.max.z && a.max.z >= b.min.z);
    }

    // Verifica colisión del objeto arrastrado con todos los colisionadores
    bool checkCollision(const glm::vec3& newPosition, int draggedIndex, bool isStatic) {
        // Obtener el modelo que se está arrastrando
        ModelData* draggedModel = nullptr;
        if (isStatic && draggedIndex < (int)m_staticModels.size()) {
            draggedModel = &m_staticModels[draggedIndex];
        } else if (!isStatic && draggedIndex < (int)m_animatedModels.size()) {
            draggedModel = &m_animatedModels[draggedIndex];
        }
        if (!draggedModel) return false;

        // Crear AABB temporal con la nueva posición
        glm::mat4 transform = glm::mat4(1.0f);
        transform = glm::translate(transform, newPosition);
        transform = glm::rotate(transform, glm::radians(draggedModel->rotation.x), glm::vec3(1, 0, 0));
        transform = glm::rotate(transform, glm::radians(draggedModel->rotation.y), glm::vec3(0, 1, 0));
        transform = glm::rotate(transform, glm::radians(draggedModel->rotation.z), glm::vec3(0, 0, 1));
        transform = glm::scale(transform, draggedModel->scale);
        transform = transform * draggedModel->model->GetRootTransform();
        AABB draggedAABB = RayCaster::calculateTransformedAABB(draggedModel->model.get(), transform);

        // Verificar contra todos los modelos estáticos que son colisionadores
        for (size_t i = 0; i < m_staticModels.size(); ++i) {
            // No colisionar con uno mismo
            if (isStatic && (int)i == draggedIndex) continue;
            
            // Solo verificar contra colisionadores
            if (!m_staticModels[i].isCollider) continue;

            AABB colliderAABB = getTransformedAABB(m_staticModels[i]);
            
            if (aabbIntersects(draggedAABB, colliderAABB)) {
                // Debug: mostrar qué colisionador bloquea
                // std::cout << "Colision con modelo " << i << std::endl;
                return true;  // Hay colisión, no mover
            }
        }

        return false;  // No hay colisión
    }

    void handleMouseDrag(double mouseX, double mouseY) {
        if (!m_dragState.isDragging || m_dragState.modelIndex < 0) return;

        glm::vec3 rayDir = RayCaster::screenToWorldRay(mouseX, mouseY, SCR_WIDTH, SCR_HEIGHT, m_camera);
        glm::vec3 newPosition = RayCaster::projectToViewPlane(
            m_camera.getPosition(),
            rayDir,
            m_dragState.dragPlaneDistance
        ) + m_dragState.dragOffset;

        // COLISIONES DESACTIVADAS - Mover directamente sin verificar
        if (m_dragState.isStatic && m_dragState.modelIndex < (int)m_staticModels.size()) {
            auto& model = m_staticModels[m_dragState.modelIndex];
            // Para modelos con keyframes, actualizar animCurrentPos
            if (model.hasKeyframeAnimation) {
                model.animCurrentPos = newPosition;
            } else {
                model.position = newPosition;
            }
        } else if (!m_dragState.isStatic && m_dragState.modelIndex < (int)m_animatedModels.size()) {
            m_animatedModels[m_dragState.modelIndex].position = newPosition;
        }
    }

    void handleMouseRelease() {
        if (m_dragState.isDragging) {
            // Al soltar, mantener la selección activa
            m_selection.hasSelection = true;
            m_selection.isStatic = m_dragState.isStatic;
            m_selection.modelIndex = m_dragState.modelIndex;
            m_selection.showControls = true;
            printTransformControls();
        }
        m_dragState.isDragging = false;
    }

    void processInput() {
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            if (m_selection.hasSelection) {
                // Cancelar selección
                clearSelection();
            } else {
                glfwSetWindowShouldClose(m_window, true);
            }
        }

        // Solo procesar cámara si no hay objeto seleccionado con modo activo
        if (!m_selection.hasSelection || m_selection.mode == TransformMode::NONE) {
            m_camera.processKeyboard(m_window, m_deltaTime);
        }

        // Reset cámara solo si no hay selección
        if (!m_selection.hasSelection && glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
            m_camera.reset();
        }

        // === CONTROLES AVANZADOS DE CÁMARA ===

        // Undo/Redo de cámara (U/I)
        if (glfwGetKey(m_window, GLFW_KEY_U) == GLFW_PRESS && !m_keyU_pressed) {
            m_keyU_pressed = true;
            if (m_camera.canUndo()) {
                m_camera.undo();
                std::cout << "[Camera] Undo aplicado" << std::endl;
            } else {
                std::cout << "[Camera] No hay estados para deshacer" << std::endl;
            }
        }
        if (glfwGetKey(m_window, GLFW_KEY_U) == GLFW_RELEASE) m_keyU_pressed = false;

        if (glfwGetKey(m_window, GLFW_KEY_I) == GLFW_PRESS && !m_keyI_pressed) {
            m_keyI_pressed = true;
            if (m_camera.canRedo()) {
                m_camera.redo();
                std::cout << "[Camera] Redo aplicado" << std::endl;
            } else {
                std::cout << "[Camera] No hay estados para rehacer" << std::endl;
            }
        }
        if (glfwGetKey(m_window, GLFW_KEY_I) == GLFW_RELEASE) m_keyI_pressed = false;

        // Guardar/Cargar estados de cámara (K/L)
        if (glfwGetKey(m_window, GLFW_KEY_K) == GLFW_PRESS && !m_keyK_pressed) {
            m_keyK_pressed = true;
            if (m_camera.saveToFile("camera_state.dat")) {
                std::cout << "[Camera] Estado guardado en camera_state.dat" << std::endl;
            } else {
                std::cout << "[Camera] Error al guardar estado" << std::endl;
            }
        }
        if (glfwGetKey(m_window, GLFW_KEY_K) == GLFW_RELEASE) m_keyK_pressed = false;

        if (glfwGetKey(m_window, GLFW_KEY_L) == GLFW_PRESS && !m_keyL_pressed) {
            m_keyL_pressed = true;
            if (m_camera.loadFromFile("camera_state.dat")) {
                std::cout << "[Camera] Estado cargado desde camera_state.dat" << std::endl;
            } else {
                std::cout << "[Camera] Error al cargar estado (archivo no encontrado)" << std::endl;
            }
        }
        if (glfwGetKey(m_window, GLFW_KEY_L) == GLFW_RELEASE) m_keyL_pressed = false;

        // Toggle suavizado (T)
        if (glfwGetKey(m_window, GLFW_KEY_T) == GLFW_PRESS && !m_keyT_pressed) {
            m_keyT_pressed = true;
            static bool smoothingEnabled = true;
            smoothingEnabled = !smoothingEnabled;
            m_camera.setEnableSmoothing(smoothingEnabled);
            std::cout << "[Camera] Suavizado: " << (smoothingEnabled ? "ON" : "OFF") << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_T) == GLFW_RELEASE) m_keyT_pressed = false;

        // === VISTAS MÉDICAS PREDEFINIDAS (NUMPAD) ===

        // Numpad 1 - Vista Frontal
        if (glfwGetKey(m_window, GLFW_KEY_KP_1) == GLFW_PRESS && !m_keyNumpad1_pressed) {
            m_keyNumpad1_pressed = true;
            m_camera.setViewPreset(Camera::ViewPreset::FRONTAL);
            std::cout << "[Camera] Vista: FRONTAL" << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_1) == GLFW_RELEASE) m_keyNumpad1_pressed = false;

        // Numpad 2 - Vista Lateral Derecha
        if (glfwGetKey(m_window, GLFW_KEY_KP_2) == GLFW_PRESS && !m_keyNumpad2_pressed) {
            m_keyNumpad2_pressed = true;
            m_camera.setViewPreset(Camera::ViewPreset::LATERAL_DERECHA);
            std::cout << "[Camera] Vista: LATERAL DERECHA" << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_2) == GLFW_RELEASE) m_keyNumpad2_pressed = false;

        // Numpad 3 - Vista Lateral Izquierda
        if (glfwGetKey(m_window, GLFW_KEY_KP_3) == GLFW_PRESS && !m_keyNumpad3_pressed) {
            m_keyNumpad3_pressed = true;
            m_camera.setViewPreset(Camera::ViewPreset::LATERAL_IZQUIERDA);
            std::cout << "[Camera] Vista: LATERAL IZQUIERDA" << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_3) == GLFW_RELEASE) m_keyNumpad3_pressed = false;

        // Numpad 4 - Vista Superior
        if (glfwGetKey(m_window, GLFW_KEY_KP_4) == GLFW_PRESS && !m_keyNumpad4_pressed) {
            m_keyNumpad4_pressed = true;
            m_camera.setViewPreset(Camera::ViewPreset::SUPERIOR);
            std::cout << "[Camera] Vista: SUPERIOR (Oclusal superior)" << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_4) == GLFW_RELEASE) m_keyNumpad4_pressed = false;

        // Numpad 5 - Vista Inferior
        if (glfwGetKey(m_window, GLFW_KEY_KP_5) == GLFW_PRESS && !m_keyNumpad5_pressed) {
            m_keyNumpad5_pressed = true;
            m_camera.setViewPreset(Camera::ViewPreset::INFERIOR);
            std::cout << "[Camera] Vista: INFERIOR (Oclusal inferior)" << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_5) == GLFW_RELEASE) m_keyNumpad5_pressed = false;

        // Numpad 6 - Vista Oclusal
        if (glfwGetKey(m_window, GLFW_KEY_KP_6) == GLFW_PRESS && !m_keyNumpad6_pressed) {
            m_keyNumpad6_pressed = true;
            m_camera.setViewPreset(Camera::ViewPreset::OCLUSAL);
            std::cout << "[Camera] Vista: OCLUSAL (directa)" << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_6) == GLFW_RELEASE) m_keyNumpad6_pressed = false;

        // Numpad 7 - Vista Panorámica
        if (glfwGetKey(m_window, GLFW_KEY_KP_7) == GLFW_PRESS && !m_keyNumpad7_pressed) {
            m_keyNumpad7_pressed = true;
            m_camera.setViewPreset(Camera::ViewPreset::PANORAMICA);
            std::cout << "[Camera] Vista: PANORAMICA (toda la boca)" << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_7) == GLFW_RELEASE) m_keyNumpad7_pressed = false;

        // Numpad 8 - Debug info
        if (glfwGetKey(m_window, GLFW_KEY_KP_8) == GLFW_PRESS && !m_keyNumpad8_pressed) {
            m_keyNumpad8_pressed = true;
            m_camera.printDebugInfo();
        }
        if (glfwGetKey(m_window, GLFW_KEY_KP_8) == GLFW_RELEASE) m_keyNumpad8_pressed = false;

        // === CONTROLES DE TRANSFORMACIÓN ===
        if (m_selection.hasSelection) {
            processTransformInput();
        }
    }
    
    void processTransformInput() {
        // === CAMBIAR MODO CON NÚMEROS 1, 2 ===
        // 1 - Trasladar
        if (glfwGetKey(m_window, GLFW_KEY_1) == GLFW_PRESS && !m_keyG_pressed) {
            m_keyG_pressed = true;
            if (m_selection.mode == TransformMode::TRANSLATE) {
                m_selection.mode = TransformMode::NONE;  // Toggle off
            } else {
                m_selection.mode = TransformMode::TRANSLATE;
                m_selection.axis = TransformAxis::ALL;
                std::cout << "[Transform] Modo TRASLADAR activado" << std::endl;
            }
        }
        if (glfwGetKey(m_window, GLFW_KEY_1) == GLFW_RELEASE) m_keyG_pressed = false;

        // 2 - Rotar
        if (glfwGetKey(m_window, GLFW_KEY_2) == GLFW_PRESS && !m_keyR_pressed) {
            m_keyR_pressed = true;
            if (m_selection.mode == TransformMode::ROTATE) {
                m_selection.mode = TransformMode::NONE;
            } else {
                m_selection.mode = TransformMode::ROTATE;
                m_selection.axis = TransformAxis::ALL;
                std::cout << "[Transform] Modo ROTAR activado" << std::endl;
            }
        }
        if (glfwGetKey(m_window, GLFW_KEY_2) == GLFW_RELEASE) m_keyR_pressed = false;
        
        // === RESTRINGIR EJES CON X, Y, Z ===
        if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_PRESS && !m_keyX_pressed) {
            m_keyX_pressed = true;
            m_selection.axis = (m_selection.axis == TransformAxis::X) ? TransformAxis::ALL : TransformAxis::X;
            std::cout << "[Transform] Eje: " << (m_selection.axis == TransformAxis::X ? "X" : "TODOS") << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_X) == GLFW_RELEASE) m_keyX_pressed = false;

        if (glfwGetKey(m_window, GLFW_KEY_Y) == GLFW_PRESS && !m_keyY_pressed) {
            m_keyY_pressed = true;
            m_selection.axis = (m_selection.axis == TransformAxis::Y) ? TransformAxis::ALL : TransformAxis::Y;
            std::cout << "[Transform] Eje: " << (m_selection.axis == TransformAxis::Y ? "Y" : "TODOS") << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_Y) == GLFW_RELEASE) m_keyY_pressed = false;

        if (glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_PRESS && !m_keyZ_pressed) {
            m_keyZ_pressed = true;
            m_selection.axis = (m_selection.axis == TransformAxis::Z) ? TransformAxis::ALL : TransformAxis::Z;
            std::cout << "[Transform] Eje: " << (m_selection.axis == TransformAxis::Z ? "Z" : "TODOS") << std::endl;
        }
        if (glfwGetKey(m_window, GLFW_KEY_Z) == GLFW_RELEASE) m_keyZ_pressed = false;

        // === APLICAR TRANSFORMACIONES CON FLECHAS ===
        if (m_selection.mode != TransformMode::NONE) {
            float speed = m_selection.transformSpeed * m_deltaTime;

            // Velocidad más rápida con SHIFT
            if (glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
                speed *= 3.0f;
            }

            glm::vec3 translateDelta(0.0f);
            glm::vec3 rotateDelta(0.0f);

            // MODO TRASLACIÓN
            if (m_selection.mode == TransformMode::TRANSLATE) {
                // Izquierda/Derecha = X
                if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) translateDelta.x -= speed;
                if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) translateDelta.x += speed;

                // Arriba/Abajo = Z (profundidad)
                if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) translateDelta.z -= speed;
                if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) translateDelta.z += speed;

                // PageUp/PageDown = Y (altura)
                if (glfwGetKey(m_window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) translateDelta.y += speed;
                if (glfwGetKey(m_window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) translateDelta.y -= speed;
            }

            // MODO ROTACIÓN
            else if (m_selection.mode == TransformMode::ROTATE) {
                float rotSpeed = speed * 50.0f;  // Más rápido para rotación

                // Izquierda/Derecha = rotar en Y
                if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS) rotateDelta.y -= rotSpeed;
                if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS) rotateDelta.y += rotSpeed;

                // Arriba/Abajo = rotar en X
                if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS) rotateDelta.x += rotSpeed;
                if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS) rotateDelta.x -= rotSpeed;

                // PageUp/PageDown = rotar en Z
                if (glfwGetKey(m_window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) rotateDelta.z += rotSpeed;
                if (glfwGetKey(m_window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) rotateDelta.z -= rotSpeed;
            }

            // Aplicar transformaciones
            applyTransformation(translateDelta, rotateDelta);
        }
    }
    
    void applyTransformation(glm::vec3 posDelta, glm::vec3 rotDelta) {
        ModelData* model = getSelectedModel();
        if (!model) return;

        // Aplicar traslación
        if (m_selection.mode == TransformMode::TRANSLATE) {
            switch (m_selection.axis) {
                case TransformAxis::X:
                    model->position.x += posDelta.x;
                    break;
                case TransformAxis::Y:
                    model->position.y += posDelta.y;
                    break;
                case TransformAxis::Z:
                    model->position.z += posDelta.z;
                    break;
                case TransformAxis::ALL:
                    model->position += posDelta;
                    break;
            }
        }

        // Aplicar rotación
        else if (m_selection.mode == TransformMode::ROTATE) {
            switch (m_selection.axis) {
                case TransformAxis::X:
                    model->rotation.x += rotDelta.x;
                    break;
                case TransformAxis::Y:
                    model->rotation.y += rotDelta.y;
                    break;
                case TransformAxis::Z:
                    model->rotation.z += rotDelta.z;
                    break;
                case TransformAxis::ALL:
                    // Si no hay eje específico, aplicar la rotación que tenga valores
                    model->rotation += rotDelta;
                    break;
            }
        }
    }
    
    ModelData* getSelectedModel() {
        if (!m_selection.hasSelection) return nullptr;
        
        if (m_selection.isStatic && m_selection.modelIndex < (int)m_staticModels.size()) {
            return &m_staticModels[m_selection.modelIndex];
        } else if (!m_selection.isStatic && m_selection.modelIndex < (int)m_animatedModels.size()) {
            return &m_animatedModels[m_selection.modelIndex];
        }
        return nullptr;
    }
    
    void resetModelByIndex(bool isStatic, int index) {
        ModelData* model = nullptr;

        if (isStatic && index < (int)m_staticModels.size()) {
            model = &m_staticModels[index];
        } else if (!isStatic && index < (int)m_animatedModels.size()) {
            model = &m_animatedModels[index];
        }

        if (!model) return;

        // Restaurar valores originales
        model->position = model->originalPosition;
        model->scale = model->originalScale;
        model->rotation = model->originalRotation;

        std::cout << "[Reset] Objeto restaurado a su posicion original" << std::endl;
    }

    void handleRightMouseClick(double mouseX, double mouseY) {
        std::cout << "[DoubleClick] handleRightMouseClick llamado" << std::endl;

        // Crear rayo desde la cámara hacia el mundo
        glm::vec3 rayOrigin = m_camera.getPosition();
        glm::vec3 rayDir = RayCaster::screenToWorldRay(mouseX, mouseY, SCR_WIDTH, SCR_HEIGHT, m_camera);

        float closestDist = FLT_MAX;
        bool foundHit = false;
        bool isStaticHit = true;
        int hitIndex = -1;

        // Test contra modelos animados (arrastrables)
        for (size_t i = 0; i < m_animatedModels.size(); ++i) {
            if (!m_animatedModels[i].isDraggable) continue;

            // Calcular la transformación completa (igual que en renderizado)
            glm::vec3 finalPos = m_animatedModels[i].position +
                                glm::vec3(0.0f, 0.0f, m_animatedModels[i].autoMoveDistance);
            glm::mat4 transform = glm::mat4(1.0f);
            transform = glm::translate(transform, finalPos);
            transform = glm::rotate(transform, glm::radians(m_animatedModels[i].rotation.x), glm::vec3(1, 0, 0));
            transform = glm::rotate(transform, glm::radians(m_animatedModels[i].rotation.y), glm::vec3(0, 1, 0));
            transform = glm::rotate(transform, glm::radians(m_animatedModels[i].rotation.z), glm::vec3(0, 0, 1));
            transform = glm::scale(transform, m_animatedModels[i].scale);

            AABB aabb = RayCaster::calculateTransformedAABB(m_animatedModels[i].model.get(), transform);

            float dist;
            if (RayCaster::rayAABBIntersection(rayOrigin, rayDir, aabb, dist)) {
                if (dist < closestDist) {
                    closestDist = dist;
                    foundHit = true;
                    isStaticHit = false;
                    hitIndex = static_cast<int>(i);
                }
            }
        }

        // Test contra modelos estáticos (arrastrables)
        for (size_t i = 0; i < m_staticModels.size(); ++i) {
            if (!m_staticModels[i].isDraggable) continue;

            glm::mat4 transform = glm::mat4(1.0f);

            // Si tiene keyframes, usar la transformación animada actual
            if (m_staticModels[i].hasKeyframeAnimation && !m_staticModels[i].keyframes.empty()) {
                transform = glm::translate(transform, m_staticModels[i].animCurrentPos);
                transform = transform * glm::mat4_cast(m_staticModels[i].animCurrentRot);
                transform = glm::scale(transform, m_staticModels[i].animCurrentScale);
            }
            // Sin keyframes, usar el sistema tradicional
            else {
                glm::mat4 rootTransform = m_staticModels[i].model->GetRootTransform();
                glm::vec3 modelCenter = glm::vec3(rootTransform[3]);

                // 1. Trasladar a posición final
                transform = glm::translate(transform, m_staticModels[i].position);
                // 2. Trasladar al centro del modelo para rotar localmente
                transform = glm::translate(transform, modelCenter);
                // 3. Aplicar rotación (rota sobre el centro del modelo)
                transform = glm::rotate(transform, glm::radians(m_staticModels[i].rotation.x), glm::vec3(1, 0, 0));
                transform = glm::rotate(transform, glm::radians(m_staticModels[i].rotation.y), glm::vec3(0, 1, 0));
                transform = glm::rotate(transform, glm::radians(m_staticModels[i].rotation.z), glm::vec3(0, 0, 1));
                // 4. Volver a trasladar en sentido inverso (para que la rotación sea local)
                transform = glm::translate(transform, -modelCenter);
                // 5. Aplicar escala
                transform = glm::scale(transform, m_staticModels[i].scale);
            }

            AABB aabb = RayCaster::calculateTransformedAABB(m_staticModels[i].model.get(), transform);

            float dist;
            if (RayCaster::rayAABBIntersection(rayOrigin, rayDir, aabb, dist)) {
                if (dist < closestDist) {
                    closestDist = dist;
                    foundHit = true;
                    isStaticHit = true;
                    hitIndex = static_cast<int>(i);
                }
            }
        }

        // Si encontramos un objeto, verificar si tiene animación de keyframes
        if (foundHit) {
            std::cout << "[DoubleClick] Objeto encontrado: " << (isStaticHit ? "Estatico" : "Animado") << " [" << hitIndex << "]" << std::endl;

            // Verificar si el objeto tiene animación por keyframes
            bool hasKeyframes = false;
            if (isStaticHit && hitIndex < (int)m_staticModels.size()) {
                hasKeyframes = m_staticModels[hitIndex].hasKeyframeAnimation;
            } else if (!isStaticHit && hitIndex < (int)m_animatedModels.size()) {
                hasKeyframes = m_animatedModels[hitIndex].hasKeyframeAnimation;
            }

            std::cout << "[DoubleClick] hasKeyframes = " << (hasKeyframes ? "true" : "false") << std::endl;

            if (hasKeyframes) {
                // Resetear a la posición inicial de la animación
                resetKeyframeAnimation(isStaticHit, hitIndex);
            } else {
                // Resetear posición manual
                resetModelByIndex(isStaticHit, hitIndex);
            }
        } else {
            std::cout << "[DoubleClick] No se encontro objeto en raycast" << std::endl;
        }
    }

    void clearSelection() {
        m_selection.hasSelection = false;
        m_selection.modelIndex = -1;
        m_selection.mode = TransformMode::NONE;
        m_selection.axis = TransformAxis::ALL;
        m_selection.showControls = false;
        std::cout << "\n[Seleccion cancelada]" << std::endl;
    }
    
    void printTransformControls() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "  OBJETO SELECCIONADO - CONTROLES" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "  1 - Modo TRASLADAR (mover)" << std::endl;
        std::cout << "  2 - Modo ROTAR" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "  X/Y/Z - Restringir a eje especifico" << std::endl;
        std::cout << "\n  TRASLACION:" << std::endl;
        std::cout << "    Izq/Der: X | Arriba/Abajo: Z | PgUp/PgDn: Y" << std::endl;
        std::cout << "  ROTACION:" << std::endl;
        std::cout << "    Izq/Der: Y | Arriba/Abajo: X | PgUp/PgDn: Z" << std::endl;
        std::cout << "\n  SHIFT - Transformar 3x mas rapido" << std::endl;
        std::cout << "  ESC - Cancelar seleccion" << std::endl;
        std::cout << "========================================\n" << std::endl;
    }

    void update() {
        // Actualizar cámara (transiciones, interpolación)
        m_camera.update(m_deltaTime);

        // Actualizar animaciones de keyframes
        updateKeyframeAnimations(m_deltaTime);

        // Solo actualizar animación si no está pausada
        if (!m_animationPaused) {
            updateAllAnimations();  // Actualizar todos los modelos animados
        }
    }

    void render() {
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // Color celeste claro (sky blue)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Renderizar todo usando el sistema moderno
        renderAllStaticModels();          // Todos los modelos estáticos
        renderAllAnimatedModels();        // Todos los modelos animados
        renderAxis();
        
        // Renderizar HUD con información de transformación (al final para que esté encima)
        renderHUD();
    }


    void renderAxis() {
        m_axisShader->use();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                                (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                0.1f, 100.0f);
        glm::mat4 view = m_camera.getViewMatrix();
        m_axisShader->setMat4("projection", projection);
        m_axisShader->setMat4("view", view);

        glLineWidth(3.0f);
        glBindVertexArray(m_axisVAO);
        glDrawArrays(GL_LINES, 0, 6);
        glBindVertexArray(0);
        glLineWidth(1.0f);
    }

    void cleanup() {
        if (m_axisVAO != 0) {
            glDeleteVertexArrays(1, &m_axisVAO);
            glDeleteBuffers(1, &m_axisVBO);
        }
        if (m_hudVAO != 0) {
            glDeleteVertexArrays(1, &m_hudVAO);
            glDeleteBuffers(1, &m_hudVBO);
        }
        if (m_window) {
            glfwTerminate();
            std::cout << "Programa finalizado correctamente" << std::endl;
        }
    }

    // Callbacks estáticos
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    }

    static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
        if (s_instance) {
            // Si está arrastrando, actualizar posición del objeto
            if (s_instance->m_dragState.isDragging) {
                s_instance->handleMouseDrag(xpos, ypos);
            }
            // La cámara solo se mueve con click derecho
            s_instance->m_camera.processMouseMovement(xpos, ypos);
        }
    }

    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        if (s_instance) {
            // Click izquierdo para drag & drop y doble clic para resetear
            if (button == GLFW_MOUSE_BUTTON_LEFT) {
                if (action == GLFW_PRESS) {
                    double currentTime = glfwGetTime();
                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    
                    // Detectar doble clic
                    if (currentTime - s_instance->m_lastLeftClickTime < s_instance->m_doubleClickThreshold) {
                        // Doble clic: resetear modelo a posición original
                        s_instance->handleRightMouseClick(xpos, ypos);
                        s_instance->m_lastLeftClickTime = 0.0; // Resetear para evitar triple clic
                    } else {
                        // Clic simple: drag & drop normal
                        s_instance->handleMouseClick(xpos, ypos);
                        s_instance->m_lastLeftClickTime = currentTime;
                    }
                } else if (action == GLFW_RELEASE) {
                    s_instance->handleMouseRelease();
                }
            }

            // Shift + Clic derecho: Resetear animación de keyframes
            if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift está presionado: resetear animación
                    double xpos, ypos;
                    glfwGetCursorPos(window, &xpos, &ypos);
                    s_instance->handleRightMouseClick(xpos, ypos);
                    return; // No pasar el evento a la cámara
                }
            }

            // Siempre procesar para cámara (comportamiento normal)
            s_instance->m_camera.processMouseButton(button, action);
        }
    }

    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
        if (s_instance) {
            s_instance->m_camera.processMouseScroll(yoffset);
        }
    }
};

// Definición del puntero estático
Application* Application::s_instance = nullptr;
