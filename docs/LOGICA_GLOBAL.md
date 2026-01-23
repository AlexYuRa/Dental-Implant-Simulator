# Lógica Global del Proyecto

## Introducción: El Panorama Completo

> 📝 **Nota:** Esta documentación describe la **versión original** del proyecto (`Application.h`).
> Si deseas conocer la versión refactorizada con principios POO modernos, consulta:
> - [POO_REFACTORIZACION.md](POO_REFACTORIZACION.md) - Arquitectura refactorizada
> - [ARQUITECTURA.md](ARQUITECTURA.md) - Comparación de ambas versiones

**¿Cómo funciona este proyecto de principio a fin?**

Este documento te lleva paso a paso desde que haces doble click en el ejecutable hasta que ves el cuadrúpedo animado en pantalla. Es el "mapa del tesoro" que conecta todas las piezas.

**La vista de 10,000 metros:**
```
Ejecutar programa → Inicializar todo → Loop infinito → Cerrar

Detalle del loop infinito (60 veces/seg):
    ↓
Procesar input (teclado/mouse)
    ↓
Actualizar estado (animación, movimiento)
    ↓
Renderizar (dibujar en pantalla)
    ↓
Repetir...
```

**Analogía con un videojuego:**
- **Inicialización** = Pantalla de carga (cargar modelos, compilar shaders)
- **Loop principal** = El juego ejecutándose (60 FPS)
- **Input** = Controles del jugador
- **Update** = Física y lógica del juego
- **Render** = Dibujar los gráficos

---

## Fase 1: Inicio del Programa (main.cpp)

### Punto de Entrada

```cpp
int main() {
    try {
        Application app;
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return -1;
    }
    return 0;
}
```

**¿Qué hace este código?**
1. Crea una instancia de `Application` (nuestra clase principal)
2. Llama a `app.run()` que contiene todo el programa
3. Si algo sale mal (excepción), imprime el error y termina

**Try-Catch para seguridad:**
Sin el `try-catch`, si algo falla (ej: archivo no encontrado), el programa crashearía sin mensaje. Con él, vemos el error en consola.

---

## Fase 2: Construcción de Application

### Constructor de Application

```cpp
Application::Application()
    : m_window(nullptr)
    , m_axisVAO(0), m_axisVBO(0)
    , m_camera(glm::vec3(0.0f, 2.0f, 5.0f))  // Cámara inicial
    , m_deltaTime(0.0f)
    , m_lastFrame(0.0f)
    , m_distancePerCycle(2.0f)
    , m_totalCycles(0.0f)
    , m_pauseStartTick(STOP_FRAME)
    , m_animationPaused(false)
    , m_spaceKeyPressed(false)
    , m_selectedObject(DraggableObject::NONE)
    , m_isDragging(false)
    , m_quadrupedPosition(glm::vec3(0.0f))
    , m_scenePosition(glm::vec3(0.0f))
    , m_dragOffset(glm::vec3(0.0f))
    , m_dragPlaneDistance(0.0f)
{
    s_instance = this;  // Guardar puntero para callbacks
}
```

**Inicialización de variables:**

**Punteros a nullptr:**
- `m_window = nullptr` - La ventana aún no existe

**Valores numéricos:**
- `m_deltaTime = 0.0f` - Tiempo entre frames (se calcula después)
- `m_distancePerCycle = 2.0f` - Cuánto avanza el cuadrúpedo por ciclo de animación

**Objetos con constructores:**
- `m_camera(glm::vec3(0.0f, 2.0f, 5.0f))` - Cámara a 2 unidades arriba, 5 adelante

**¿Por qué `s_instance = this`?**

GLFW usa callbacks C (funciones globales), pero queremos acceder a miembros de la clase. La solución: guardar un puntero estático a la instancia actual.

```cpp
// Callback estático (GLFW)
static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_instance) {  // Usar el puntero guardado
        s_instance->m_camera.processMouseMovement(xpos, ypos);
    }
}
```

---

## Fase 3: Inicialización (init())

### Secuencia de Inicialización

```cpp
void Application::run() {
    init();           // 1. Configurar todo
    mainLoop();       // 2. Loop principal
    cleanup();        // 3. Liberar recursos
}
```

### 3.1 Inicializar GLFW y Ventana

```cpp
void init() {
    // 1. Inicializar biblioteca GLFW
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    // 2. Configurar OpenGL 4.6 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 3. Crear ventana
    m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Cuadrupedo IIIU", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    // ...
}
```

**Paso a paso:**

**1. `glfwInit()`**
- Inicializa la biblioteca GLFW (ventanas, input, contexto OpenGL)
- Si falla → GPU no compatible o drivers incorrectos

**2. `glfwWindowHint()`**
- Configura el contexto OpenGL antes de crear la ventana
- `CONTEXT_VERSION_MAJOR/MINOR` = OpenGL 4.6
- `CORE_PROFILE` = Sin funciones obsoletas (solo OpenGL moderno)

**3. `glfwCreateWindow()`**
- Crea una ventana de 1280x720 píxeles
- Título: "Cuadrupedo IIIU"
- `nullptr, nullptr` = No fullscreen, no compartir contexto

**4. `glfwMakeContextCurrent()`**
- Hace que el contexto OpenGL de esta ventana sea el activo
- Todos los comandos OpenGL afectarán a esta ventana

### 3.2 Inicializar GLAD (Funciones OpenGL)

```cpp
if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    throw std::runtime_error("Failed to initialize GLAD");
}
```

**¿Qué es GLAD y por qué lo necesitamos?**

OpenGL es una especificación, no una biblioteca. Los drivers de la GPU implementan las funciones. GLAD "carga" estas funciones para que podamos usarlas.

**Sin GLAD:**
```cpp
glClear(...);  // ❌ Error: función no definida
```

**Con GLAD:**
```cpp
gladLoadGLLoader(...);  // Carga las funciones
glClear(...);           // ✅ Funciona
```

### 3.3 Configurar OpenGL

```cpp
glEnable(GL_DEPTH_TEST);           // Activar depth testing (Z-buffer)
glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);  // Tamaño de renderizado
```

**`GL_DEPTH_TEST`:**
Sin depth test, los objetos se dibujan en orden de código (no por distancia). Resultado: cosas lejanas encima de cosas cercanas.

Con depth test: OpenGL guarda la profundidad de cada pixel y solo dibuja si está más cerca.

**`glViewport`:**
Define el área de la ventana donde se renderizará. (0,0) = esquina inferior izquierda.

### 3.4 Configurar Callbacks

```cpp
glfwSetFramebufferSizeCallback(m_window, framebufferSizeCallback);
glfwSetCursorPosCallback(m_window, mouseCallback);
glfwSetMouseButtonCallback(m_window, mouseButtonCallback);
glfwSetScrollCallback(m_window, scrollCallback);
```

**¿Qué son los callbacks?**

Son funciones que GLFW llama cuando ocurre un evento:
- **framebufferSizeCallback**: Usuario redimensiona la ventana
- **mouseCallback**: Mouse se mueve
- **mouseButtonCallback**: Click de mouse
- **scrollCallback**: Rueda del mouse

**Patrón de diseño:**
```
Usuario mueve mouse
    ↓
GLFW detecta movimiento
    ↓
GLFW llama a mouseCallback()
    ↓
Nuestro código procesa el movimiento
```

### 3.5 Cargar Shaders

```cpp
m_shader = std::make_unique<Shader>(
    "shaders/skeletal_animation.vert",
    "shaders/skeletal_animation.frag"
);

m_staticShader = std::make_unique<Shader>(
    "shaders/static.vert",
    "shaders/static.frag"
);

m_axisShader = std::make_unique<Shader>(
    "shaders/axis.vert",
    "shaders/axis.frag"
);
```

**Tipos de shaders:**
1. **skeletal_animation** - Para el cuadrúpedo (con animación de huesos)
2. **static** - Para la escena (sin animación)
3. **axis** - Para los ejes de coordenadas (líneas simples)

**¿Qué hace `Shader::Shader()`?**
1. Lee los archivos .vert y .frag
2. Compila el código GLSL
3. Enlaza vertex y fragment shader en un programa
4. Verifica errores de compilación

**Analogía:**
Un shader es como un pequeño programa que corre en la GPU. El vertex shader procesa cada vértice, el fragment shader cada pixel.

### 3.6 Cargar Modelos

```cpp
m_model = std::make_unique<Model>("assets/models/quadruped.glb");
m_sceneModel = std::make_unique<Model>("assets/models/scena.glb");
```

**¿Qué hace `Model::Model()`?**
1. Usa ASSIMP para leer el archivo GLB
2. Extrae mallas (meshes), texturas, materiales
3. Para cada mesh:
   - Lee vértices (posición, normal, UV, bone IDs, weights)
   - Lee índices (triángulos)
   - Crea buffers en GPU (VBO, EBO)
4. Carga texturas embebidas en memoria de GPU

**Resultado:**
Todos los datos del modelo están en la GPU, listos para renderizar.

### 3.7 Cargar Animación

```cpp
m_animation = std::make_unique<Animation>("assets/models/quadruped.glb", m_model.get());
m_animator = std::make_unique<Animator>(m_animation.get());
```

**`Animation::Animation()`:**
1. Lee jerarquía de huesos del GLB
2. Extrae keyframes (posición, rotación, escala en tiempos específicos)
3. Guarda información como duración, ticks por segundo

**`Animator::Animator()`:**
1. Toma una animación como input
2. Mantiene el tiempo actual de la animación
3. Calcula matrices de transformación de huesos interpolando keyframes

**Relación:**
- `Animation` = Los "datos" de la animación (qué hacer)
- `Animator` = El "reproductor" (en qué momento estamos)

### 3.8 Crear Ejes de Coordenadas

```cpp
initializeAxis();
```

Crea los buffers para dibujar 3 líneas (ejes X, Y, Z) en el centro del mundo. Útil para orientación.

---

## Fase 4: Loop Principal (mainLoop())

### Estructura del Loop

```cpp
void mainLoop() {
    while (!glfwWindowShouldClose(m_window)) {
        // 1. Calcular delta time
        float currentFrame = glfwGetTime();
        m_deltaTime = currentFrame - m_lastFrame;
        m_lastFrame = currentFrame;

        // 2. Procesar input
        processInput();

        // 3. Actualizar estado
        update();

        // 4. Renderizar
        render();

        // 5. Intercambiar buffers y procesar eventos
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}
```

**¿Por qué un loop infinito?**

Los programas gráficos necesitan actualizar la pantalla constantemente (60 FPS). El loop corre ~60 veces por segundo hasta que el usuario cierra la ventana.

### 4.1 Delta Time

```cpp
float currentFrame = glfwGetTime();       // Tiempo actual en segundos
m_deltaTime = currentFrame - m_lastFrame; // Tiempo desde último frame
m_lastFrame = currentFrame;               // Actualizar para próximo frame
```

**¿Por qué necesitamos delta time?**

**Problema:** En una PC lenta, el loop corre 30 veces/seg. En una rápida, 120 veces/seg.

**Sin delta time:**
```cpp
position += 1.0f;  // Se mueve 1 unidad por frame
// PC lenta: 30 unidades/seg
// PC rápida: 120 unidades/seg  ¡Inconsistente!
```

**Con delta time:**
```cpp
position += 60.0f * deltaTime;  // 60 unidades/segundo
// PC lenta (30 FPS, dt=0.033): position += 60*0.033 = 2.0 por frame → 60/seg
// PC rápida (120 FPS, dt=0.008): position += 60*0.008 = 0.5 por frame → 60/seg
// ¡Consistente!
```

### 4.2 Procesar Input (processInput())

```cpp
void processInput() {
    // Tecla ESC → Cerrar
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);

    // Movimiento de cámara (WASD, Q, E)
    m_camera.processKeyboard(m_window);

    // Toggle pausa de animación (ESPACIO)
    if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!m_spaceKeyPressed) {
            m_animationPaused = !m_animationPaused;
            m_spaceKeyPressed = true;
            std::cout << (m_animationPaused ? "Animacion PAUSADA" : "Animacion REANUDADA") << std::endl;
        }
    } else {
        m_spaceKeyPressed = false;
    }

    // Reset cámara (R)
    if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS) {
        m_camera.reset();
        m_totalCycles = 0.0f;
    }

    // Ajustar distancia por ciclo (+/-)
    if (glfwGetKey(m_window, GLFW_KEY_EQUAL) == GLFW_PRESS)
        m_distancePerCycle += DISTANCE_ADJUSTMENT_STEP;

    if (glfwGetKey(m_window, GLFW_KEY_MINUS) == GLFW_PRESS)
        m_distancePerCycle = std::max(0.0f, m_distancePerCycle - DISTANCE_ADJUSTMENT_STEP);
}
```

**Orden de procesamiento:**
1. **Escape** → Salir del programa
2. **Cámara** → Delegar a Camera::processKeyboard()
3. **Pausa** → Toggle con debouncing (evitar múltiples activaciones)
4. **Reset** → Volver cámara y ciclos a estado inicial
5. **Ajustes** → Modificar parámetros de movimiento

**Debouncing de ESPACIO:**
```cpp
if (!m_spaceKeyPressed) {  // Solo si no estaba presionado antes
    m_animationPaused = !m_animationPaused;
    m_spaceKeyPressed = true;  // Marcar como presionado
}
// ...
else {
    m_spaceKeyPressed = false;  // Se soltó la tecla
}
```

Sin esto, mantener ESPACIO causaría pausar/reanudar 60 veces por segundo.

### 4.3 Actualizar Estado (update())

```cpp
void update() {
    // Solo actualizar animación si no está pausada
    if (!m_animationPaused) {
        m_animator->UpdateAnimation(m_deltaTime);
    }

    // Detectar ciclos de animación completos
    float currentAnimTime = m_animator->GetCurrentTime();
    static float lastAnimTime = 0.0f;

    if (currentAnimTime < lastAnimTime) {  // La animación se reinició
        m_totalCycles += 1.0f;
    }
    lastAnimTime = currentAnimTime;
}
```

**`m_animator->UpdateAnimation(m_deltaTime)`:**

1. Avanza el tiempo de la animación: `m_currentTime += deltaTime * ticksPerSecond`
2. Si llegó al final: `m_currentTime = 0` (loop)
3. Calcula matrices de transformación de huesos en el tiempo actual
4. Interpola entre keyframes

**Detección de ciclos:**

```
Tiempo de animación:
0s -----> 3.9s (fin) -----> 0s (reinicio) -----> ...
                ↑
                Aquí: currentAnimTime (0) < lastAnimTime (3.9)
                → ¡Ciclo completo! → m_totalCycles++
```

**Uso de `m_totalCycles`:**
Para calcular el desplazamiento total del cuadrúpedo:
```cpp
float autoMoveDistance = m_totalCycles * m_distancePerCycle + currentCycleDistance;
```

### 4.4 Renderizar (render())

```cpp
void render() {
    // 1. Limpiar pantalla
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // Color celeste
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Calcular movimiento automático
    float currentAnimTime = m_animator->GetCurrentTime();
    float currentCycleDistance = calculateCycleDistance(currentAnimTime);
    float autoMoveDistance = m_totalCycles * m_distancePerCycle + currentCycleDistance;

    // 3. Renderizar modelos (en orden de profundidad)
    // renderStaticModel();              // Escena (fondo) - COMENTADO
    renderModel(autoMoveDistance);       // Cuadrúpedo (principal)
    renderAxis();                        // Ejes de coordenadas
}
```

**Orden de renderizado:**
1. **Limpiar** → Borrar frame anterior
2. **Calcular posiciones** → Basado en animación
3. **Renderizar de atrás hacia adelante** → Escena, cuadrúpedo, ejes

**¿Por qué limpiar `GL_DEPTH_BUFFER_BIT`?**

El depth buffer guarda la profundidad de cada pixel. Si no lo limpias, el depth del frame anterior interfiere con el nuevo.

#### Renderizar Modelo Animado

```cpp
void renderModel(float autoMoveDistance) {
    m_shader->use();  // Activar shader de skeletal animation

    // 1. Enviar matrices de huesos al shader
    auto& transforms = m_animator->GetFinalBoneMatrices();
    for (size_t i = 0; i < transforms.size(); ++i) {
        m_shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);
    }

    // 2. Enviar matrices de transformación
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), ASPECT_RATIO, 0.1f, 100.0f);
    glm::mat4 view = m_camera.getViewMatrix();
    m_shader->setMat4("projection", projection);
    m_shader->setMat4("view", view);

    // 3. Transformación del modelo (posición arrastrable + movimiento automático)
    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat, m_quadrupedPosition + glm::vec3(0.0f, 0.0f, autoMoveDistance));
    m_shader->setMat4("model", modelMat);

    // 4. Parámetros de iluminación
    m_shader->setVec3("lightPos", glm::vec3(0.0f, 5.0f, 20.0f));
    m_shader->setVec3("lightColor", glm::vec3(1.3f, 1.3f, 1.3f));
    m_shader->setVec3("viewPos", m_camera.getPosition());

    // 5. Dibujar el modelo
    m_model->Draw(*m_shader);
}
```

**Flujo de transformaciones (MVP):**

```
Vertex en espacio local (modelo)
    ↓ [Matriz Model]
Vertex en espacio del mundo
    ↓ [Matriz View]
Vertex en espacio de cámara
    ↓ [Matriz Projection]
Vertex en clip space (-1 a 1)
    ↓ [GPU automático]
Pixel en pantalla
```

**Matrices explicadas:**

**Projection (Perspectiva):**
```cpp
glm::perspective(glm::radians(45.0f), ASPECT_RATIO, 0.1f, 100.0f)
```
- `45.0f` = FOV (Field of View) - Ángulo de visión
- `ASPECT_RATIO` = 16:9 (para que no se deforme)
- `0.1f, 100.0f` = Near/Far planes (qué tan cerca/lejos se puede ver)

**View (Cámara):**
```cpp
m_camera.getViewMatrix()
```
Transformación inversa de la posición/rotación de la cámara. Si la cámara está en (0, 2, 5), la matriz mueve el mundo a (-0, -2, -5).

**Model (Objeto):**
```cpp
glm::translate(modelMat, position)
```
Dónde está el objeto en el mundo.

**Iluminación Phong:**
```cpp
m_shader->setVec3("lightPos", glm::vec3(0.0f, 5.0f, 20.0f));    // Posición de la luz
m_shader->setVec3("lightColor", glm::vec3(1.3f, 1.3f, 1.3f));   // Color blanco brillante
m_shader->setVec3("viewPos", m_camera.getPosition());           // Posición de cámara (para specular)
```

**`m_model->Draw()`:**
1. Por cada mesh del modelo:
   - Activar VAO (Vertex Array Object)
   - Bind texturas
   - `glDrawElements()` → GPU dibuja los triángulos
   - Unbind texturas

### 4.5 Swap Buffers y Poll Events

```cpp
glfwSwapBuffers(m_window);
glfwPollEvents();
```

**Double Buffering:**

```
GPU renderiza en → [Back Buffer]
Usuario ve       → [Front Buffer]

Al terminar frame:
    glfwSwapBuffers() → Intercambia buffers

Ahora:
GPU renderiza en → [Front Buffer (antes back)]
Usuario ve       → [Back Buffer (antes front)]
```

**¿Por qué?** Sin double buffering, verías el dibujo en progreso (tearing, flickering).

**`glfwPollEvents()`:**

Procesa eventos pendientes (clicks, teclas, redimensionar). Los callbacks se ejecutan aquí.

---

## Fase 5: Cierre del Programa (cleanup())

```cpp
void cleanup() {
    // Liberar buffers de ejes
    if (m_axisVAO != 0) {
        glDeleteVertexArrays(1, &m_axisVAO);
        glDeleteBuffers(1, &m_axisVBO);
    }

    // Terminar GLFW
    if (m_window) {
        glfwTerminate();
        std::cout << "Programa finalizado correctamente" << std::endl;
    }
}
```

**Liberación de recursos:**
1. **OpenGL buffers** → `glDeleteVertexArrays`, `glDeleteBuffers`
2. **GLFW** → `glfwTerminate()` libera ventana y contexto

**¿Qué pasa con shaders, modelos, etc.?**

Se usan `std::unique_ptr`, que automáticamente llaman a los destructores:
```cpp
m_shader.reset();      // Llama a ~Shader() → glDeleteProgram()
m_model.reset();       // Llama a ~Model() → libera meshes
m_animation.reset();   // etc.
```

---

## Flujo de Datos Completo

### Del Disco a la Pantalla

```
1. CARGA (init())
   Disco: quadruped.glb
      ↓ [ASSIMP]
   RAM: Vertices, Indices, Textures, Bones, Keyframes
      ↓ [glBufferData(), glTexImage2D()]
   GPU: VBO, EBO, Texture Objects

2. ANIMACIÓN (update())
   RAM: Keyframes
      ↓ [Interpolación]
   RAM: Bone Matrices (30 matrices 4x4)
      ↓ [glUniformMatrix4fv()]
   GPU: Shader Uniforms

3. RENDERIZADO (render())
   GPU: VBO (vértices)
      ↓ [Vertex Shader]
   GPU: Vértices transformados + skinning
      ↓ [Rasterización]
   GPU: Fragmentos
      ↓ [Fragment Shader]
   GPU: Pixels con color e iluminación
      ↓ [Depth Test + Blend]
   GPU: Framebuffer
      ↓ [glfwSwapBuffers()]
   Pantalla: Imagen final
```

---

## Sistemas Interconectados

### Mapa de Dependencias

```
Application (orquestador)
    ├── GLFW (ventana, input)
    ├── GLAD (funciones OpenGL)
    ├── Camera (transformaciones de vista)
    ├── Shaders (programas GPU)
    │   ├── skeletal_animation
    │   ├── static
    │   └── axis
    ├── Modelos
    │   ├── Model (cuadrúpedo)
    │   │   └── Mesh[] (mallas)
    │   │       ├── Vertices (posiciones, normales, UVs, bones)
    │   │       ├── Indices (triángulos)
    │   │       └── Textures
    │   └── Model (escena)
    ├── Animación
    │   ├── Animation (datos: keyframes, jerarquía)
    │   └── Animator (estado: tiempo actual, matrices)
    └── Interactividad
        ├── Ray Casting (mouse → 3D)
        ├── AABB (detección de colisiones)
        └── Drag & Drop (mover objetos)
```

### Flujo de Input

```
[Usuario presiona ESPACIO]
    ↓
GLFW detecta en glfwPollEvents()
    ↓
Application::processInput() revisa estado
    ↓
m_animationPaused = !m_animationPaused
    ↓
Próximo frame: update() salta UpdateAnimation() si pausado
    ↓
Cuadrúpedo se congela en frame actual

[Usuario mueve mouse]
    ↓
GLFW llama a mouseCallback()
    ↓
¿Está arrastrando?
    ├─ SÍ → handleMouseDrag() → actualiza posición
    └─ NO → handleMouseHover() → muestra info en consola
    ↓
m_camera.processMouseMovement() (siempre)
    ↓
Próximo frame: render() usa nueva posición de cámara/objeto
```

---

## Ciclo de Vida de un Frame (60 FPS)

### Frame N

```
T = 0.000s
    ↓
[glfwPollEvents()]
    → Usuario hizo click izquierdo
    → mouseButtonCallback() ejecuta
    → handleMouseClick()
        → screenToWorldRay()
        → rayAABBIntersection() → HIT!
        → m_isDragging = true
    ↓
[processInput()]
    → ESC no presionado
    → WASD no presionado
    → Nada que hacer
    ↓
[update()]
    → m_animator->UpdateAnimation(0.016s)
        → m_currentTime += 0.016 * 24 ticks/s = +0.384 ticks
        → Interpolar keyframes
        → Calcular matrices de huesos
    → currentAnimTime = 2.5s, lastAnimTime = 2.48s
    → No hubo ciclo (2.5 >= 2.48)
    ↓
[render()]
    → glClear() → Pantalla celeste
    → autoMoveDistance = 0 * 2.0 + 1.25 = 1.25
    → renderModel(1.25)
        → Enviar 30 matrices de huesos al shader
        → Enviar MVP matrices
        → Posición = (0, 0, 0) + (0, 0, 1.25) = (0, 0, 1.25)
        → m_model->Draw()
            → Por cada mesh:
                → glDrawElements(triangleCount)
                → GPU: Vertex Shader procesa vértices
                → GPU: Fragment Shader pinta pixels
    → renderAxis()
        → Dibujar 3 líneas RGB
    ↓
[glfwSwapBuffers()]
    → Back buffer → Front buffer
    → Usuario ve el cuadrúpedo en posición (0, 0, 1.25)
    ↓
T = 0.016s (16 ms después, ~60 FPS)
```

---

## Optimizaciones del Sistema

### 1. Skinning en GPU

**Sin skinning en GPU (lento):**
```
CPU: Por cada vértice (10,000):
    - Calcular boneTransform de 4 huesos
    - Aplicar a posición, normal
    - Enviar a GPU
```

**Con skinning en GPU (rápido):**
```
CPU:
    - Calcular 30 matrices de huesos
    - Enviar al shader
GPU: Por cada vértice en paralelo:
    - Leer BoneIDs y Weights
    - Calcular transformación
```

**Ganancia:** ~300x más rápido (30 matrices vs 10,000 vértices)

### 2. Buffers Persistentes

Los vértices se cargan una vez en GPU (`glBufferData` en `Mesh::setupMesh()`). No se envían cada frame.

**Frame 1:**
```
CPU → GPU: 10,000 vértices (80 KB)
```

**Frame 2-infinito:**
```
CPU → GPU: Solo 30 matrices (1.9 KB)
```

### 3. Culling y Depth Test

**Face Culling** (no implementado):
No renderizar triángulos que miran hacia atrás. Ahorra ~50% de fragmentos.

**Depth Test** (activo):
No procesar fragmentos ocultos por otros. Ahorra procesamiento en fragment shader.

---

## Configuración y Parámetros Clave

### Constantes de Aplicación

```cpp
// Application.h
static constexpr unsigned int SCR_WIDTH = 1280;
static constexpr unsigned int SCR_HEIGHT = 720;
static constexpr float AXIS_LENGTH = 3.0f;
static constexpr float DISTANCE_ADJUSTMENT_STEP = 0.05f;
static constexpr float STOP_FRAME = 155.0f;
static constexpr float TOTAL_ANIMATION_FRAMES = 234.0f;
```

**Modificar estos valores afecta:**
- `SCR_WIDTH/HEIGHT` → Tamaño de ventana, aspect ratio
- `DISTANCE_ADJUSTMENT_STEP` → Sensibilidad de +/- para ajustar movimiento
- `STOP_FRAME` → Frame donde termina el movimiento forward del cuadrúpedo
- `TOTAL_ANIMATION_FRAMES` → Duración total de la animación

### Parámetros de Cámara

```cpp
// Camera.h (valores iniciales)
Position: (0, 2, 5)           // 2 arriba, 5 hacia Z
Yaw: -90°                     // Mirando hacia -Z
Pitch: 0°                     // Horizontal
Distance: 5.0                 // Zoom inicial
MOVE_SPEED: 2.5               // Velocidad de WASD
SENSITIVITY: 0.1              // Sensibilidad de mouse
```

### Parámetros de Iluminación

```cpp
// En renderModel()
lightPos: (0, 5, 20)          // Luz desde arriba y atrás
lightColor: (1.3, 1.3, 1.3)   // Blanco brillante
ambient: 0.35                 // Luz ambiente (en shader)
specular: 0.6                 // Brillo metálico
shininess: 64                 // Concentración del brillo
```

---

## Debugging y Profiling

### Mensajes de Consola

**Al iniciar:**
```
Cuadrupedo_IIIU iniciado
=== CONTROLES ===
- Clic IZQUIERDO + Arrastrar: Mover objetos en 3D
- Clic DERECHO + Arrastrar: Rotar camara
...
```

**Durante ejecución:**
```
=== HOVER INFO ===
  Cuadrupedo: HIT (distancia: 8.234)
  >> SE SELECCIONARIA: CUADRUPEDO <<
==================

=== CLICK DEBUG ===
Cuadrupedo: HIT (dist: 8.234)
>>> CUADRUPEDO SELECCIONADO <<<
===================

Animacion PAUSADA
Animacion REANUDADA

Objeto liberado en posicion: (2.5, 0.0, 15.3)
```

**Al cerrar:**
```
Programa finalizado correctamente
```

### Puntos de Medición

**Para medir FPS:**
```cpp
// En mainLoop()
static int frameCount = 0;
static double lastTime = glfwGetTime();

frameCount++;
if (glfwGetTime() - lastTime >= 1.0) {  // Cada segundo
    std::cout << "FPS: " << frameCount << std::endl;
    frameCount = 0;
    lastTime = glfwGetTime();
}
```

**Para medir tiempo de renderizado:**
```cpp
double startTime = glfwGetTime();
render();
double renderTime = (glfwGetTime() - startTime) * 1000.0;  // ms
std::cout << "Render: " << renderTime << " ms" << std::endl;
```

---

## Resumen del Flujo Completo

```
INICIO
  ↓
main()
  ↓
Application app
  ├─ Constructor → Inicializar variables
  ↓
app.run()
  ├─ init()
  │   ├─ GLFW/GLAD
  │   ├─ OpenGL config
  │   ├─ Callbacks
  │   ├─ Shaders
  │   ├─ Modelos (CPU→GPU)
  │   ├─ Animación
  │   └─ Ejes
  ↓
  ├─ mainLoop()
  │   └─ while (!windowShouldClose) {
  │         ├─ calcular deltaTime
  │         ├─ processInput()     [Teclado/Mouse]
  │         ├─ update()           [Animación, Ciclos]
  │         ├─ render()           [GPU dibuja]
  │         ├─ swapBuffers()      [Mostrar frame]
  │         └─ pollEvents()       [Callbacks ejecutan]
  │       }
  ↓
  └─ cleanup()
      ├─ Liberar buffers OpenGL
      ├─ glfwTerminate()
      └─ Destructores automáticos
  ↓
return 0
  ↓
FIN
```

---

## Documentación Relacionada

### Sistemas Individuales

- [Animación Esquelética](ANIMATION.md) - Cómo funcionan los huesos y keyframes
- [Sistema de Renderizado](RENDERING.md) - Pipeline gráfico detallado
- [Cámara Orbital](CAMERA.md) - Transformaciones de vista
- [Interactividad](INTERACTIVIDAD.md) - Ray casting y drag & drop
- [Iluminación Phong](ILUMINACION.md) - Modelo de iluminación
- [Modelos y Texturas](MODELS.md) - Carga de GLB y texturas
- [Shaders](SHADERS.md) - Código GLSL

### Guías Prácticas

- [Controles del Usuario](CONTROLES.md) - Teclas y mouse
- [Configuración](CONFIGURACION.md) - Compilar el proyecto
- [Guía para Principiantes](GUIA_PRINCIPIANTES.md) - Conceptos básicos

---

**Última actualización:** Diciembre 2024
