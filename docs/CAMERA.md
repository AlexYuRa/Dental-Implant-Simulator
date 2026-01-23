# Sistema de Cámara

> 📝 **Nota:** El archivo `Camera.h` es compartido por ambas versiones del proyecto (original y POO).
> La cámara es independiente y no requirió refactorización ya que sigue buenos principios de diseño.

## Introducción: Cómo Funciona la Cámara Virtual

**¿Qué es una cámara virtual?**

En gráficos 3D, la cámara determina desde dónde y hacia dónde estás mirando la escena. Es como una cámara de video real que puedes mover libremente en el espacio 3D.

**Analogía de cámara real:**
- **Posición** = Dónde está físicamente la cámara
- **Target** (objetivo) = Hacia dónde apunta la cámara
- **Distancia** = Qué tan cerca o lejos está del objetivo
- **Ángulos** (yaw/pitch) = La dirección de rotación

**¿Cómo se usa en este proyecto?**
- Click derecho + arrastrar = rotar alrededor del objetivo
- Scroll del mouse = acercar o alejar (zoom)
- WASD = mover el punto objetivo
- Q/E = mover objetivo arriba/abajo

**Diferencia con otras cámaras:**
- **FPS Camera** (primera persona): La cámara se mueve libremente, como en videojuegos de disparos
- **Orbital Camera** (este proyecto): La cámara gira alrededor de un punto fijo, como inspeccionar un objeto
- **Fixed Camera**: La cámara no se mueve en absoluto

---

## Tipo de Cámara: Orbital

La cámara implementada es del tipo **orbital** (también conocida como turntable o arcball), donde la cámara orbita alrededor de un punto objetivo (target).

**¿Por qué usar cámara orbital para este proyecto?**
- Perfecta para visualizar modelos 3D desde todos los ángulos
- Siempre mantiene el objeto en el centro de la vista
- Intuitiva para explorar objetos estáticos o animados
- Similar a herramientas profesionales como Blender

```
        Camera
          /|\
         / | \
        /  |  \
       /   |   \  Distance
      /    |    \
     /     |     \
    /      ↓      \
   -----Target-----
```

## Clase Camera

### Miembros Principales

```cpp
class Camera {
private:
    // Posición y objetivo
    glm::vec3 m_position;      // Posición actual de la cámara
    glm::vec3 m_target;        // Punto al que mira
    glm::vec3 m_worldUp;       // Vector "arriba" del mundo

    // Ángulos de Euler
    float m_yaw;               // Rotación horizontal (izq/der)
    float m_pitch;             // Rotación vertical (arriba/abajo)

    // Distancia
    float m_distance;          // Distancia desde target

    // Constantes
    static constexpr float MIN_DISTANCE = 1.0f;
    static constexpr float MAX_DISTANCE = 200.0f;
    static constexpr float DEFAULT_DISTANCE = 5.0f;
    static constexpr float SENSITIVITY = 0.1f;
    static constexpr float ZOOM_SENSITIVITY = 0.5f;
    static constexpr float MOVE_SPEED = 2.5f;
    static constexpr float MIN_PITCH = -89.0f;
    static constexpr float MAX_PITCH = 89.0f;

    // Estado de mouse
    bool m_firstMouse;
    bool m_rightButtonPressed;
    float m_lastX;
    float m_lastY;
};
```

### Constructor

```cpp
Camera::Camera(glm::vec3 position, float width, float height)
    : m_target(0.0f, 0.0f, 0.0f)
    , m_worldUp(0.0f, 1.0f, 0.0f)
    , m_yaw(-90.0f)            // Mirando hacia -Z
    , m_pitch(0.0f)
    , m_distance(DEFAULT_DISTANCE)
    , m_firstMouse(true)
    , m_rightButtonPressed(false)
    , m_lastX(width / 2.0f)
    , m_lastY(height / 2.0f)
{
    updateCameraVectors();
}
```

## Sistema de Coordenadas

### Ejes del Mundo
```
       Y (Verde - Arriba)
       |
       |
       |_______X (Rojo - Derecha)
      /
     /
    Z (Azul - Hacia Nosotros)
```

### Ángulos de Euler

**Yaw (Guiñada):**
- Rotación alrededor del eje Y
- 0° = mirando hacia +X
- -90° = mirando hacia -Z (frente por defecto)
- 90° = mirando hacia +Z (atrás)
- 180° = mirando hacia -X

**Pitch (Cabeceo):**
- Rotación alrededor del eje X
- 0° = horizontal
- +89° = casi mirando hacia arriba
- -89° = casi mirando hacia abajo
- Limitado a ±89° para evitar gimbal lock

## Actualización de Vectores

### updateCameraVectors()

```cpp
void Camera::updateCameraVectors() {
    // Convertir ángulos de Euler a coordenadas cartesianas

    // Calcular nueva posición basada en yaw y pitch
    glm::vec3 offset;
    offset.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    offset.y = sin(glm::radians(m_pitch));
    offset.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));

    // Normalizar y escalar por distancia
    offset = glm::normalize(offset) * m_distance;

    // Posición = Target + Offset
    m_position = m_target + offset;
}
```

**Ejemplo numérico:**
```cpp
// Valores iniciales
m_yaw = -90.0f
m_pitch = 0.0f
m_distance = 5.0f
m_target = (0, 0, 0)

// Cálculo offset:
offset.x = cos(-90°) * cos(0°) = 0.0
offset.y = sin(0°) = 0.0
offset.z = sin(-90°) * cos(0°) = -1.0

offset normalizad = (0, 0, -1)
offset escalado = (0, 0, -5)

// Posición final
m_position = (0, 0, 0) + (0, 0, -5) = (0, 0, -5)
// Cámara está 5 unidades atrás en el eje Z
```

## Matriz de Vista

### getViewMatrix()

```cpp
glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_target, m_worldUp);
}
```

**glm::lookAt** crea una matriz de vista que:
1. Posiciona la cámara en `m_position`
2. Mira hacia `m_target`
3. Orienta usando `m_worldUp` como referencia vertical

## Controles de Mouse

### 1. Rotación (Clic Derecho + Arrastrar)

```cpp
void Camera::processMouseMovement(double xpos, double ypos) {
    if (m_firstMouse) {
        m_lastX = xpos;
        m_lastY = ypos;
        m_firstMouse = false;
        return;
    }

    if (!m_rightButtonPressed) {
        m_lastX = xpos;
        m_lastY = ypos;
        return;
    }

    // Calcular offset del mouse
    float xoffset = xpos - m_lastX;
    float yoffset = m_lastY - ypos;  // Invertido (Y crece hacia abajo)

    m_lastX = xpos;
    m_lastY = ypos;

    // Aplicar sensibilidad
    xoffset *= SENSITIVITY;
    yoffset *= SENSITIVITY;

    // Actualizar ángulos
    m_yaw += xoffset;
    m_pitch += yoffset;

    // Limitar pitch para evitar voltear
    if (m_pitch > MAX_PITCH)
        m_pitch = MAX_PITCH;
    if (m_pitch < MIN_PITCH)
        m_pitch = MIN_PITCH;

    // Recalcular posición
    updateCameraVectors();
}
```

### 2. Detección de Botón

```cpp
void Camera::processMouseButton(int button, int action) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            m_rightButtonPressed = true;
        } else if (action == GLFW_RELEASE) {
            m_rightButtonPressed = false;
        }
    }
}
```

### 3. Zoom (Scroll)

```cpp
void Camera::processMouseScroll(double yoffset) {
    // Reducir distancia = acercar
    m_distance -= (float)yoffset * ZOOM_SENSITIVITY;

    // Limitar rango de zoom
    if (m_distance < MIN_DISTANCE)
        m_distance = MIN_DISTANCE;
    if (m_distance > MAX_DISTANCE)
        m_distance = MAX_DISTANCE;

    // Actualizar posición
    updateCameraVectors();
}
```

## Controles de Teclado

### Movimiento del Target

```cpp
void Camera::processKeyboard(GLFWwindow* window) {
    float velocity = MOVE_SPEED * 0.016f;  // ~60 FPS

    // Calcular vectores de dirección
    glm::vec3 forward = glm::normalize(m_target - m_position);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_worldUp));

    // WASD: Movimiento horizontal
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_target += forward * velocity;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_target -= forward * velocity;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_target += right * velocity;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_target -= right * velocity;

    // Q/E: Movimiento vertical
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_target -= m_worldUp * velocity;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_target += m_worldUp * velocity;

    // Actualizar posición de cámara
    updateCameraVectors();
}
```

### Reset

```cpp
void Camera::reset() {
    // Restaurar valores por defecto
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_yaw = -90.0f;
    m_pitch = 0.0f;
    m_distance = DEFAULT_DISTANCE;

    updateCameraVectors();
}
```

## Integración con Application

### Setup de Callbacks

```cpp
void Application::initWindow() {
    // ...

    // Registrar callbacks
    glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
    glfwSetCursorPosCallback(m_window, mouseCallback);
    glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
}

// Callbacks estáticos
static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_instance) {
        s_instance->m_camera.processMouseMovement(xpos, ypos);
    }
}

static void mouseButtonCallback(GLFWwindow* window, int button,
                                int action, int mods) {
    if (s_instance) {
        s_instance->m_camera.processMouseButton(button, action);
    }
}

static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (s_instance) {
        s_instance->m_camera.processMouseScroll(yoffset);
    }
}
```

### Game Loop

```cpp
void Application::processInput() {
    // Tecla ESC para salir
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);

    // Movimiento de cámara (WASD, Q, E)
    m_camera.processKeyboard(m_window);

    // Reset con R
    if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
        m_camera.reset();
        m_totalCycles = 0.0f;  // También reset de animación
    }
}
```

## Uso en Shaders

```cpp
void Application::renderModel(float autoMoveDistance) {
    // Obtener matriz de vista desde la cámara
    glm::mat4 view = m_camera.getViewMatrix();

    // Enviar al shader
    m_shader->setMat4("view", view);

    // También enviar posición para iluminación
    m_shader->setVec3("viewPos", m_camera.getPosition());

    // ...
}
```

## Configuración y Personalización

### Ajustar Sensibilidad

```cpp
// En Camera.h
static constexpr float SENSITIVITY = 0.1f;      // Rotación
static constexpr float ZOOM_SENSITIVITY = 0.5f; // Zoom
static constexpr float MOVE_SPEED = 2.5f;       // Movimiento
```

**Valores recomendados:**
- Sensibilidad baja (0.05 - 0.1): Control preciso
- Sensibilidad media (0.1 - 0.2): Balance
- Sensibilidad alta (0.2 - 0.5): Movimiento rápido

### Ajustar Rango de Zoom

```cpp
static constexpr float MIN_DISTANCE = 1.0f;     // Más cerca
static constexpr float MAX_DISTANCE = 200.0f;   // Más lejos
```

### Cambiar Posición Inicial

```cpp
Camera::Camera(glm::vec3 position, float width, float height) {
    // Cambiar ángulo inicial
    m_yaw = -90.0f;    // -90° = mirando hacia -Z (frente)
    m_pitch = 30.0f;   // 30° = ligeramente desde arriba

    // Cambiar distancia inicial
    m_distance = 10.0f;  // Más lejos

    // Cambiar target inicial
    m_target = glm::vec3(0.0f, 2.0f, 0.0f);  // Ligeramente elevado
}
```

## Troubleshooting

### Problema: Cámara voltea al mirar arriba
**Causa:** Pitch > 90° o < -90° (gimbal lock)
**Solución:** Límites de pitch ya implementados (±89°)

### Problema: Movimiento errático con mouse
**Causa:** `m_firstMouse` no se maneja correctamente
**Solución:** Ya manejado en `processMouseMovement()`

### Problema: Cámara muy lenta/rápida
**Causa:** Sensibilidad o velocidad incorrecta
**Solución:** Ajustar constantes SENSITIVITY y MOVE_SPEED

### Problema: Zoom insuficiente
**Causa:** MAX_DISTANCE muy pequeño
**Solución:** Aumentar MAX_DISTANCE (actualmente 200.0f)

## Alternativas de Cámara

### Primera Persona (FPS)
```cpp
// No implementado, pero sería:
class FPSCamera {
    glm::vec3 m_position;  // Posición directa
    glm::vec3 m_front;     // Vector dirección
    // Sin target ni distancia
};
```

### Seguimiento (Follow)
```cpp
// No implementado, pero sería:
class FollowCamera {
    glm::vec3* m_targetObject;  // Seguir objeto
    glm::vec3 m_offset;         // Offset desde objeto
};
```

---

**Próximo:** [Controles del Usuario](CONTROLES.md)
