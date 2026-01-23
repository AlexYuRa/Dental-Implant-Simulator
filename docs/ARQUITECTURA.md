# Arquitectura del Proyecto

## Introducción: Cómo Está Organizado el Código

**¿Qué es "arquitectura" en programación?**

La arquitectura es cómo organizas tu código en piezas (clases, archivos) y cómo esas piezas se comunican entre sí. Es como el plano de una casa: defines qué va en cada habitación y cómo se conectan.

> **🆕 Nota:** El proyecto ahora tiene dos versiones de arquitectura:
> - **Original** (`Application.h`) - Funcional, ~2000 líneas
> - **Refactorizada** (`ApplicationRefactored.h`) - POO, ~400 líneas
> 
> Ver **[POO_REFACTORIZACION.md](POO_REFACTORIZACION.md)** para la nueva arquitectura.

**Analogía de restaurante:**
- **Application** = El gerente (coordina todo)
- **Shader** = Las recetas (instrucciones de cómo cocinar)
- **Model** = Los ingredientes procesados (modelos 3D listos)
- **Animator** = El chef (hace que las cosas se muevan)
- **Camera** = El comensal (desde dónde ves todo)

**En la versión POO:**
- **SceneManager** = El inventario (gestiona todos los modelos)
- **RenderManager** = La cocina (solo renderiza)
- **InputManager** = El mesero (recibe órdenes del usuario)
- **SelectionManager** = La carta (qué está seleccionado)

**Beneficios de una buena arquitectura:**
- **Mantenibilidad**: Fácil encontrar y modificar código
- **Reutilización**: Puedes usar las mismas clases en otros proyectos
- **Escalabilidad**: Fácil agregar nuevas características
- **Debugging**: Más fácil encontrar errores

---

## Visión General

El proyecto sigue una arquitectura orientada a objetos con separación clara de responsabilidades. Cada componente del sistema tiene un propósito específico y se comunica con otros componentes a través de interfaces bien definidas.

**Principio clave: "Separation of Concerns" (Separación de Responsabilidades)**
- Cada clase hace UNA cosa y la hace bien
- **Application** no sabe cómo funciona un shader internamente, solo sabe cómo usarlo
- **Model** no sabe cómo renderizar, solo cómo almacenar datos
- **Shader** no sabe qué modelo está dibujando, solo ejecuta instrucciones

## Componentes Principales

### 1. Application (Application.h)

**Responsabilidad:** Clase principal que gestiona el ciclo de vida de la aplicación.

**Funciones clave:**
- Inicialización de OpenGL y GLFW
- Game loop principal
- Gestión de recursos (shaders, modelos, texturas)
- Coordinación entre componentes

**Ciclo de vida:**
```cpp
Application app;
app.initialize()     // Setup ventana, OpenGL, recursos
app.run()           // Game loop
  └─> updateDeltaTime()
  └─> processInput()
  └─> update()
  └─> render()
      └─> renderStaticModel()
      └─> renderModel()
      └─> renderAxis()
  └─> swap buffers
```

**Miembros importantes:**
```cpp
// Ventana
GLFWwindow* m_window;

// Componentes de renderizado
std::unique_ptr<Shader> m_shader;          // Shader animación esquelética
std::unique_ptr<Shader> m_staticShader;    // Shader modelos estáticos
std::unique_ptr<Shader> m_axisShader;      // Shader ejes coordenadas

// Modelos
std::unique_ptr<Model> m_model;            // Modelo cuadrúpedo
std::unique_ptr<Model> m_sceneModel;       // Modelo escena estática

// Animación
std::unique_ptr<Animation> m_animation;
std::unique_ptr<Animator> m_animator;

// Cámara
Camera m_camera;
```

### 2. Shader (Shader.h)

**Responsabilidad:** Gestión de programas shader (compilación, linking, uniforms).

**Funciones principales:**
```cpp
Shader(vertexPath, fragmentPath)  // Carga y compila shaders
use()                              // Activa el shader
setMat4(name, matrix)             // Establece uniform matriz 4x4
setVec3(name, vector)             // Establece uniform vector 3D
setInt(name, value)               // Establece uniform entero
```

**Proceso de compilación:**
1. Lee archivos de vertex y fragment shader
2. Compila cada shader individualmente
3. Linka en un programa shader
4. Verifica errores en cada paso

### 3. Model (Model.h/cpp)

**Responsabilidad:** Carga y gestión de modelos 3D usando ASSIMP.

**Pipeline de carga:**
```
GLB File → ASSIMP → aiScene → processNode() → Meshes → Model
                                    ↓
                            processMesh() → Mesh
                                    ↓
                            loadMaterialTextures() → Textures
```

**Funciones clave:**
```cpp
Model(path)                    // Carga modelo desde archivo
Draw(shader)                   // Renderiza todas las mallas
loadModel(path)               // Procesa archivo con ASSIMP
processNode(node, scene)      // Procesa jerarquía de nodos
processMesh(mesh, scene)      // Convierte aiMesh a Mesh
loadMaterialTextures(...)     // Carga texturas del material
```

**Gestión de huesos:**
```cpp
ExtractBoneWeightForVertices(vertices, mesh, scene)
SetVertexBoneDataToDefault(vertex)  // Para modelos sin bones
```

### 4. Mesh (Mesh.h/cpp)

**Responsabilidad:** Representación de una malla 3D individual.

**Estructura de datos:**
```cpp
struct Vertex {
    glm::vec3 Position;      // Posición del vértice
    glm::vec3 Normal;        // Normal para iluminación
    glm::vec2 TexCoords;     // Coordenadas de textura
    glm::ivec4 BoneIDs;      // IDs de huesos (hasta 4)
    glm::vec4 Weights;       // Pesos de influencia
};

struct Texture {
    unsigned int id;
    std::string type;        // texture_diffuse, texture_specular
    std::string path;
};
```

**Renderizado:**
```cpp
setupMesh()      // Configura VAO, VBO, EBO
Draw(shader)     // Bind texturas y dibuja geometría
```

### 5. Animation (Animation.h)

**Responsabilidad:** Carga y gestión de datos de animación.

**Estructura:**
```cpp
Animation {
    float m_Duration;              // Duración en ticks
    int m_TicksPerSecond;         // FPS de la animación
    AssimpNodeData m_RootNode;    // Jerarquía de nodos
    std::map<string, BoneInfo> m_BoneInfoMap;
}
```

**Funciones clave:**
```cpp
ReadHeirarchyData(rootNode, scene)    // Lee jerarquía
ReadMissingBones(animation, model)    // Asocia bones con modelo
```

### 6. Animator (Animator.h)

**Responsabilidad:** Actualiza y calcula transformaciones de animación.

**Funciones principales:**
```cpp
UpdateAnimation(deltaTime)           // Avanza animación
CalculateBoneTransform(node, parent) // Calcula transformaciones
GetFinalBoneMatrices()              // Obtiene matrices para shader
```

**Pipeline de actualización:**
```
UpdateAnimation → CalculateBoneTransform (recursivo) → FinalBoneMatrices
```

### 7. Camera (Camera.h)

**Responsabilidad:** Sistema de cámara orbital con controles.

**Sistema de coordenadas:**
```
Camera orbita alrededor de un target
- Target: Punto al que mira la cámara
- Distance: Distancia desde el target
- Yaw: Rotación horizontal
- Pitch: Rotación vertical
```

**Funciones:**
```cpp
getViewMatrix()              // Matriz de vista para shader
processMouseMovement()       // Rotación con mouse
processMouseScroll()         // Zoom
processKeyboard()            // Movimiento del target
reset()                      // Resetear posición
```

## Flujo de Datos

### Inicialización
```
main()
  → Application::initialize()
      → initWindow()
      → initOpenGL()
      → loadResources()
          → Cargar shaders
          → Cargar modelos (Model → ASSIMP → Meshes)
          → Cargar animaciones
          → Crear animator
      → setupAxisRenderer()
```

### Game Loop
```
Application::run()
  while (!shouldClose):
    → updateDeltaTime()
    → processInput()
        → Camera::processKeyboard()
        → Camera::processMouseMovement()
    → update()
        → Animator::UpdateAnimation()
    → render()
        → renderStaticModel()
            → Shader::use()
            → Set uniforms (matrices, luz)
            → Model::Draw()
                → Mesh::Draw() para cada malla
        → renderModel()
            → Similar a renderStaticModel()
        → renderAxis()
    → swapBuffers()
    → pollEvents()
```

### Renderizado de Modelo Animado
```
renderModel()
  → m_shader->use()
  → Obtener matrices de bones del Animator
  → Set bone matrices como uniforms (100 matrices max)
  → Set matrices MVP (Model, View, Projection)
  → Set parámetros de iluminación
  → m_model->Draw()
      → Para cada Mesh:
          → Bind texturas
          → Bind VAO
          → glDrawElements()
```

### Renderizado de Modelo Estático
```
renderStaticModel()
  → m_staticShader->use()
  → Set matrices MVP
  → Set parámetros de iluminación
  → m_sceneModel->Draw()
      → Similar a modelo animado pero sin bones
```

## Patrones de Diseño Utilizados

### 1. RAII (Resource Acquisition Is Initialization)
```cpp
std::unique_ptr<Shader> m_shader;      // Gestión automática de memoria
std::unique_ptr<Model> m_model;
```

### 2. Singleton (parcial)
```cpp
static Application* s_instance;  // Para callbacks GLFW
```

### 3. Composite (Jerarquía de Nodos)
```cpp
struct AssimpNodeData {
    aiMatrix4x4 transformation;
    std::string name;
    std::vector<AssimpNodeData> children;  // Recursivo
};
```

### 4. Strategy (Diferentes Shaders)
```cpp
// Shader para animación
m_shader->use();
// Shader para estáticos
m_staticShader->use();
```

---

## 🆕 Nueva Arquitectura POO (Refactorizada)

> **Nota:** Se ha implementado una versión refactorizada del proyecto que sigue los principios SOLID de POO.
> Ver **[POO_REFACTORIZACION.md](POO_REFACTORIZACION.md)** para documentación completa.

### Nuevos Componentes

#### Interfaces (src/interfaces/)
```cpp
IRenderable      // Contrato para objetos que se pueden dibujar
ITransformable   // Contrato para objetos con posición/rotación/escala
IUpdatable       // Contrato para objetos que se actualizan cada frame
```

#### Core (src/core/)
```cpp
TransformTypes   // Enums: Transform::Mode, Transform::Axis
InputState       // Estado centralizado de teclado/mouse
SceneObject      // Clase base para objetos de escena
  ├── AnimatedSceneObject   // Con animación esqueletal
  └── KeyframeSceneObject   // Con keyframes
SelectionManager // Gestión de selección de objetos
TransformController // Aplicación de transformaciones
```

#### Managers (src/managers/)
```cpp
SceneManager     // Gestión de todos los objetos
RenderManager    // Sistema de renderizado
InputManager     // Gestión de input con callbacks
```

### Diagrama de la Nueva Arquitectura

```
ApplicationRefactored (~400 líneas)
    │
    ├── SceneManager
    │   └── SceneObject* (múltiples)
    │       ├── AnimatedSceneObject
    │       └── KeyframeSceneObject
    │
    ├── RenderManager
    │   └── Shader* (múltiples)
    │
    ├── InputManager
    │   └── InputState
    │
    ├── SelectionManager
    │
    └── TransformController
```

### Comparación: Original vs Refactorizado

| Aspecto | Application.h | ApplicationRefactored.h |
|---------|---------------|------------------------|
| Líneas | ~2044 | ~400 |
| Responsabilidades | Múltiples | Una (coordinación) |
| Herencia | No | Sí (SceneObject) |
| Interfaces | No | Sí (IRenderable, etc.) |
| Testabilidad | Difícil | Fácil |

---

## Gestión de Memoria

### Smart Pointers
Todo se gestiona con `std::unique_ptr` para evitar leaks:
```cpp
std::unique_ptr<Shader> m_shader;
std::unique_ptr<Model> m_model;
std::unique_ptr<Animation> m_animation;
```

### Recursos OpenGL
- VAO, VBO, EBO se eliminan en destructores
- Texturas se gestionan con IDs de OpenGL
- Shaders se eliminan automáticamente

## Configuración de Constantes

```cpp
// Application.h
static constexpr unsigned int SCR_WIDTH = 1280;
static constexpr unsigned int SCR_HEIGHT = 720;
static constexpr float AXIS_LENGTH = 3.0f;
static constexpr float DISTANCE_ADJUSTMENT_STEP = 0.05f;
static constexpr float STOP_FRAME = 155.0f;
static constexpr float TOTAL_ANIMATION_FRAMES = 234.0f;

// Camera.h
static constexpr float MIN_DISTANCE = 1.0f;
static constexpr float MAX_DISTANCE = 200.0f;
static constexpr float SENSITIVITY = 0.1f;
static constexpr float MOVE_SPEED = 2.5f;

// Shaders
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
```

## Dependencias entre Componentes

### Arquitectura Original
```
Application
    ├── Camera
    ├── Shader (x3: skeletal, static, axis)
    ├── Model (x2: quadruped, scene)
    │   └── Mesh (múltiples)
    │       └── Texture (múltiples)
    ├── Animation
    │   └── Bone
    └── Animator
        └── Animation
```

### Arquitectura Refactorizada (POO)
```
ApplicationRefactored
    ├── Camera
    ├── SceneManager
    │   └── SceneObject[]
    │       └── Model + Animation + Animator
    ├── RenderManager
    │   └── Shader[]
    ├── InputManager
    │   └── InputState
    ├── SelectionManager
    └── TransformController
```

## Threading y Sincronización

**Modelo:** Single-threaded
- Todo ocurre en el hilo principal
- No hay concurrencia ni locks necesarios
- Renderizado y actualización son secuenciales

## Extensibilidad

### Para agregar nuevo tipo de objeto (POO):
```cpp
// 1. Crear subclase de SceneObject
class MiObjeto : public SceneObject {
    void Update(float dt) override { /* ... */ }
    void Draw(Shader& shader) override { /* ... */ }
};

// 2. Agregar al SceneManager
sceneManager->AddStaticObject("modelo.glb", "Nombre");
```

### Para agregar nuevo modelo (original):
1. Agregar `std::unique_ptr<Model>` en Application.h
2. Cargar en `loadResources()`
3. Crear método `renderNuevoModelo()`
4. Llamar desde `render()`

### Para agregar nuevo shader:
1. Crear archivos .vert y .frag en /shaders
2. Agregar `std::unique_ptr<Shader>` en Application.h
3. Cargar en `loadResources()`
4. Usar con `shader->use()` donde sea necesario

### Para modificar iluminación:
1. Ajustar uniforms en `renderModel()` y `renderStaticModel()`
2. Modificar fragment shaders para cambiar modelo de iluminación

---

**Próximo:** [Refactorización POO](POO_REFACTORIZACION.md) | [Sistema de Renderizado](RENDERING.md)
