# Sistema de Animación Esquelética

> 📝 **Nota sobre versiones:**
> - **Versión original:** Animación manejada directamente en `Application.h`
> - **Versión POO:** Los objetos animados heredan de `AnimatedSceneObject` en `src/core/SceneObject.h`
> 
> Los archivos `Animation.h`, `Animator.h` y `Bone.h` son compartidos por ambas versiones.

## Introducción: Cómo Funciona la Animación en Este Proyecto

**Para entender el concepto general:**

Imagina que el cuadrúpedo es como un títere con un esqueleto interno. En lugar de mover manualmente cada punto (vértice) de su superficie, movemos los "huesos" internos, y los puntos de la superficie se mueven automáticamente siguiendo a los huesos que los controlan.

**¿Por qué es esto útil?**
- En vez de animar miles de vértices individualmente, solo animamos ~20-30 huesos
- Los artistas crean la animación una vez en Blender, y el programa la reproduce
- Podemos cambiar la velocidad, pausar, o combinar animaciones fácilmente

**El proceso completo en 3 pasos:**
1. **Carga** (se hace una vez al inicio): Lee el archivo GLB y extrae los huesos y sus movimientos
2. **Actualización** (60 veces por segundo): Calcula dónde debe estar cada hueso en el momento actual
3. **Aplicación** (en la GPU): Mueve cada vértice según los huesos que lo influencian

---

## Conceptos Fundamentales

### ¿Qué es Skeletal Animation?

La animación esquelética (skeletal animation o rigging) es una técnica donde un modelo 3D se deforma basándose en una estructura de "huesos" (bones) internos. Cada vértice del modelo está influenciado por uno o más huesos.

**Ventajas:**
- Animaciones fluidas y realistas
- Reutilización de animaciones
- Menor uso de memoria que vertex animation
- Interpolación automática entre keyframes

**¿Cómo se diferencia de otras técnicas?**
- **Vertex Animation**: Guarda la posición de CADA vértice en CADA frame (enorme archivo)
- **Skeletal Animation**: Guarda la transformación de cada hueso en keyframes (pequeño archivo, se interpola el resto)

### Componentes del Sistema

```
Animation → Jerarquía de Huesos → Keyframes → Animator → Transformaciones
                                                    ↓
                                              Vertex Shader
                                                    ↓
                                              Vértices Deformados
```

## Estructura de Datos

**Contexto para intermediarios:**

Antes de ver el código, es importante entender que necesitamos almacenar varios tipos de información:
- **BoneInfo**: Información estática de cada hueso (ID, matriz de offset)
- **Vertex**: Cada punto del modelo necesita saber qué huesos lo afectan y con qué intensidad
- **AssimpNodeData**: La jerarquía de huesos (quién es padre de quién)
- **Keyframes**: Posiciones específicas de los huesos en momentos clave de la animación

Piensa en esto como una receta: primero definimos los ingredientes (estructuras de datos), luego explicamos cómo usarlos (clases y funciones).

---

### 1. Bone (Hueso)

```cpp
struct BoneInfo {
    int id;                    // ID único del hueso
    glm::mat4 offset;         // Matriz de offset (bind pose)
};
```

**Explicando la Offset Matrix (matriz de offset):**

Esta es probablemente la parte más confusa para principiantes. Veamos qué hace paso a paso:

1. **Problema**: Los vértices del modelo están en "espacio del modelo" (coordenadas relativas al modelo completo)
2. **Necesidad**: Para animarlos, necesitamos trabajar en "espacio del hueso" (coordenadas relativas a cada hueso)
3. **Solución**: La offset matrix hace la conversión

**Analogía práctica:**
Imagina que tienes un brazo (hueso) que rota. Los puntos de la piel del brazo no están definidos como "10cm desde el hombro", sino como coordenadas absolutas en el modelo. La offset matrix convierte esas coordenadas absolutas a "relativas al hueso", para que cuando el hueso se mueva, los puntos se muevan correctamente con él.

**En términos matemáticos:** `offset = inverse(bindPoseTransform)`
- bindPoseTransform = posición inicial del hueso cuando el modelo está en pose neutra
- La inversa nos permite "deshacer" esa transformación inicial

### 2. Vertex (Vértice con Bones)

```cpp
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::ivec4 BoneIDs;       // Hasta 4 huesos por vértice
    glm::vec4 Weights;        // Pesos de influencia (suman 1.0)
};
```

**Ejemplo de Weights:**
```cpp
// Vértice influenciado por 2 huesos
BoneIDs  = {5, 12, -1, -1}
Weights  = {0.7, 0.3, 0.0, 0.0}
// Resultado = 70% hueso 5 + 30% hueso 12
```

**¿Por qué necesitamos múltiples huesos por vértice?**

Imagina el codo de un brazo:
- Los vértices cerca del antebrazo deberían moverse 100% con el hueso del antebrazo
- Los vértices cerca del bícep deberían moverse 100% con el hueso del brazo superior
- **Los vértices EN EL CODO** necesitan moverse parcialmente con ambos huesos para crear una deformación suave

Sin weights múltiples, el codo se vería rígido y robótico. Con weights (70% brazo superior, 30% antebrazo), se crea una curva natural.

**Regla importante**: Los weights siempre deben sumar 1.0 (100%)
- ✅ Correcto: `{0.7, 0.3, 0.0, 0.0}` → 0.7 + 0.3 = 1.0
- ❌ Incorrecto: `{0.7, 0.7, 0.0, 0.0}` → 0.7 + 0.7 = 1.4

**ID -1** significa "no hay hueso" (slot vacío)

### 3. AssimpNodeData (Jerarquía)

```cpp
struct AssimpNodeData {
    glm::mat4 transformation;              // Transformación local
    std::string name;                      // Nombre del nodo
    int childrenCount;
    std::vector<AssimpNodeData> children;  // Hijos en jerarquía
};
```

**Ejemplo de jerarquía:**
```
Root
├── Spine
│   ├── Chest
│   │   ├── LeftShoulder
│   │   │   └── LeftArm
│   │   └── RightShoulder
│   │       └── RightArm
│   └── Neck
│       └── Head
├── LeftHip
│   └── LeftLeg
└── RightHip
    └── RightLeg
```

### 4. KeyPosition, KeyRotation, KeyScale (Keyframes)

**¿Qué son los Keyframes y por qué los usamos?**

En animación, un **keyframe** es un "momento clave" donde defines explícitamente dónde debe estar algo. El sistema automáticamente calcula (interpola) las posiciones entre keyframes.

**Ejemplo práctico:**
- Frame 0: El brazo está abajo (keyframe 1)
- Frame 30: El brazo está arriba (keyframe 2)
- Frames 1-29: El sistema calcula automáticamente posiciones intermedias

**¿Por qué es eficiente?**
- Sin keyframes: Necesitarías guardar la posición en CADA frame (234 frames × 30 huesos × 3 transformaciones = mucha data)
- Con keyframes: Solo guardas ~20-40 keyframes por hueso, el resto se calcula

**Los 3 tipos de transformación:**
- **Position** (traslación): Dónde está el hueso en el espacio (X, Y, Z)
- **Rotation** (rotación): En qué ángulo está (usando quaternions para evitar gimbal lock)
- **Scale** (escala): Qué tan grande es (raramente usado en skeletal animation)

```cpp
struct KeyPosition {
    glm::vec3 position;
    float timeStamp;
};

struct KeyRotation {
    glm::quat orientation;  // Quaternion para rotación
    float timeStamp;
};

struct KeyScale {
    glm::vec3 scale;
    float timeStamp;
};
```

## Clase Animation

### Responsabilidades
- Cargar datos de animación desde archivo GLB
- Almacenar jerarquía de huesos
- Almacenar keyframes de animación
- Proporcionar información de timing

### Miembros Principales

```cpp
class Animation {
private:
    float m_Duration;                              // Duración en ticks
    int m_TicksPerSecond;                         // FPS
    std::vector<Bone> m_Bones;                    // Lista de huesos
    AssimpNodeData m_RootNode;                    // Raíz de jerarquía
    std::map<std::string, BoneInfo> m_BoneInfoMap;

public:
    Animation(const std::string& animationPath, Model* model);
    float GetTicksPerSecond() { return m_TicksPerSecond; }
    float GetDuration() { return m_Duration; }
    const AssimpNodeData& GetRootNode() { return m_RootNode; }
    const std::map<std::string, BoneInfo>& GetBoneIDMap() {
        return m_BoneInfoMap;
    }
};
```

### Proceso de Carga

```cpp
Animation::Animation(path, model) {
    // 1. Cargar archivo con ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate);

    // 2. Obtener primera animación
    aiAnimation* animation = scene->mAnimations[0];
    m_Duration = animation->mDuration;
    m_TicksPerSecond = animation->mTicksPerSecond;

    // 3. Leer jerarquía de nodos
    ReadHeirarchyData(m_RootNode, scene->mRootNode);

    // 4. Leer bones y keyframes
    ReadMissingBones(animation, *model);
}
```

### ReadHeirarchyData (Recursivo)

```cpp
void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
    dest.name = src->mName.data;
    dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
        src->mTransformation
    );
    dest.childrenCount = src->mNumChildren;

    // Recursivamente leer hijos
    for (int i = 0; i < src->mNumChildren; i++) {
        AssimpNodeData newData;
        ReadHeirarchyData(newData, src->mChildren[i]);
        dest.children.push_back(newData);
    }
}
```

## Clase Bone

### Responsabilidades
- Almacenar keyframes de un hueso específico
- Interpolar entre keyframes
- Calcular transformación en un tiempo dado

### Miembros

```cpp
class Bone {
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

public:
    void Update(float animationTime);
    glm::mat4 GetLocalTransform() { return m_LocalTransform; }
    std::string GetBoneName() const { return m_Name; }
    int GetBoneID() { return m_ID; }
};
```

### Interpolación de Keyframes

**Entendiendo la Interpolación - El Corazón de la Animación Suave**

La interpolación es el proceso de calcular valores intermedios entre dos keyframes. Es lo que hace que la animación se vea fluida en lugar de saltar bruscamente de una pose a otra.

**Ejemplo visual:**
```
Keyframe A (t=0.0s): Brazo en Y=0    ┃
                                     ┃  <- Aquí el sistema
Interpolación (t=0.5s): Brazo en Y=5 ┃     calcula valores
                                     ┃     intermedios
Keyframe B (t=1.0s): Brazo en Y=10   ┃
```

**Tipos de interpolación usados:**
1. **Lineal (mix)**: Para posición y escala - calcula promedio ponderado
2. **Esférica (slerp)**: Para rotación - interpolación sobre la superficie de una esfera

**¿Por qué slerp para rotaciones?**
Los quaternions (rotaciones) no se pueden interpolar linealmente porque causaría distorsiones. Slerp garantiza que la rotación sea suave y siga el camino más corto.

---

#### 1. Encontrar Keyframes
```cpp
int GetPositionIndex(float animationTime) {
    for (int index = 0; index < m_NumPositions - 1; ++index) {
        if (animationTime < m_Positions[index + 1].timeStamp)
            return index;
    }
    return 0;  // Último frame
}
```

#### 2. Interpolar Posición
```cpp
glm::mat4 InterpolatePosition(float animationTime) {
    if (1 == m_NumPositions)
        return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

    int p0Index = GetPositionIndex(animationTime);
    int p1Index = p0Index + 1;

    // Calcular factor de interpolación [0.0, 1.0]
    float scaleFactor = GetScaleFactor(
        m_Positions[p0Index].timeStamp,
        m_Positions[p1Index].timeStamp,
        animationTime
    );

    // Interpolación lineal
    glm::vec3 finalPosition = glm::mix(
        m_Positions[p0Index].position,
        m_Positions[p1Index].position,
        scaleFactor
    );

    return glm::translate(glm::mat4(1.0f), finalPosition);
}
```

#### 3. Interpolar Rotación (SLERP)
```cpp
glm::mat4 InterpolateRotation(float animationTime) {
    if (1 == m_NumRotations) {
        auto rotation = glm::normalize(m_Rotations[0].orientation);
        return glm::toMat4(rotation);
    }

    int p0Index = GetRotationIndex(animationTime);
    int p1Index = p0Index + 1;
    float scaleFactor = GetScaleFactor(...);

    // Spherical Linear Interpolation (SLERP)
    glm::quat finalRotation = glm::slerp(
        m_Rotations[p0Index].orientation,
        m_Rotations[p1Index].orientation,
        scaleFactor
    );

    return glm::toMat4(glm::normalize(finalRotation));
}
```

#### 4. Interpolar Escala
```cpp
glm::mat4 InterpolateScaling(float animationTime) {
    // Similar a posición, pero con glm::scale
    // ...
    return glm::scale(glm::mat4(1.0f), finalScale);
}
```

#### 5. Combinar Transformaciones
```cpp
void Bone::Update(float animationTime) {
    glm::mat4 translation = InterpolatePosition(animationTime);
    glm::mat4 rotation = InterpolateRotation(animationTime);
    glm::mat4 scale = InterpolateScaling(animationTime);

    // Orden: Scale → Rotate → Translate
    m_LocalTransform = translation * rotation * scale;
}
```

## Clase Animator

### Responsabilidades
- Actualizar animación frame a frame
- Calcular transformaciones finales de todos los huesos
- Proporcionar matrices para el shader

### Miembros

```cpp
class Animator {
private:
    std::vector<glm::mat4> m_FinalBoneMatrices;  // 100 matrices max
    Animation* m_CurrentAnimation;
    float m_CurrentTime;
    float m_DeltaTime;

public:
    Animator(Animation* animation);
    void UpdateAnimation(float dt);
    void CalculateBoneTransform(const AssimpNodeData* node,
                                glm::mat4 parentTransform);
    std::vector<glm::mat4>& GetFinalBoneMatrices();
};
```

### Actualización de Animación

```cpp
void Animator::UpdateAnimation(float dt) {
    m_DeltaTime = dt;

    if (m_CurrentAnimation) {
        // Incrementar tiempo
        m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;

        // Loop de animación
        m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

        // Calcular transformaciones desde la raíz
        CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(),
                               glm::mat4(1.0f));
    }
}
```

### Cálculo de Transformaciones (Recursivo)

```cpp
void Animator::CalculateBoneTransform(const AssimpNodeData* node,
                                      glm::mat4 parentTransform) {
    std::string nodeName = node->name;
    glm::mat4 nodeTransform = node->transformation;

    // Buscar bone correspondiente
    Bone* bone = m_CurrentAnimation->FindBone(nodeName);

    if (bone) {
        // Actualizar transformación local del bone
        bone->Update(m_CurrentTime);
        nodeTransform = bone->GetLocalTransform();
    }

    // Transformación global = Parent * Local
    glm::mat4 globalTransformation = parentTransform * nodeTransform;

    // Si es un bone, calcular matriz final
    auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
    if (boneInfoMap.find(nodeName) != boneInfoMap.end()) {
        int index = boneInfoMap[nodeName].id;
        glm::mat4 offset = boneInfoMap[nodeName].offset;

        // FinalMatrix = GlobalTransform * OffsetMatrix
        m_FinalBoneMatrices[index] = globalTransformation * offset;
    }

    // Recursión: procesar hijos
    for (int i = 0; i < node->childrenCount; i++) {
        CalculateBoneTransform(&node->children[i], globalTransformation);
    }
}
```

### Matriz Final del Bone

**Entendiendo la Fórmula Más Importante de Skeletal Animation**

La matriz final de cada hueso se calcula como:

```
FinalMatrix = GlobalTransform × OffsetMatrix
```

**¿Por qué esta fórmula?** Vamos paso a paso:

1. **OffsetMatrix**: Lleva el vértice desde "espacio del modelo" a "espacio del hueso en bind pose"
   - Bind pose = la pose neutral/inicial del modelo
   - Ejemplo: Un vértice en posición absoluta (5, 10, 3) se convierte a posición relativa al hueso (0.2, 1.5, 0.1)

2. **GlobalTransform**: Aplica la transformación actual del hueso en la animación
   - Incluye la jerarquía completa (si el brazo se mueve, la mano también)
   - Esta es la pose animada actual

3. **FinalMatrix**: El resultado final que aplicamos al vértice
   - Combina ambas transformaciones
   - "Deshace" la pose inicial y aplica la pose animada

**Analogía:**
- OffsetMatrix = "Olvida dónde estabas originalmente"
- GlobalTransform = "Ahora muévete a esta nueva posición animada"
- FinalMatrix = La transformación completa

**Componentes detallados:**
- **OffsetMatrix**: Transforma vértices del espacio del modelo al espacio del hueso (bind pose)
- **GlobalTransform**: Transformación actual del hueso en la animación (incluye jerarquía)
- **FinalMatrix**: Transformación completa para aplicar al vértice

## Vertex Skinning (en Shader)

**¿Qué es Skinning y dónde ocurre?**

"Skinning" literalmente significa "poner piel" sobre el esqueleto. Es el proceso final donde se aplican todas las transformaciones de huesos a los vértices del modelo.

**¿Por qué en el shader (GPU)?**
- Tenemos miles de vértices (ej: 10,000 en el cuadrúpedo)
- La CPU calcularía uno por uno (lento)
- La GPU puede procesar miles en paralelo (rápido)

**El flujo completo:**
1. **CPU** calcula las ~30 matrices finales de huesos → envía a GPU
2. **GPU** aplica esas matrices a los ~10,000 vértices en paralelo
3. Resultado: Todos los vértices transformados en microsegundos

**Recuerda**: Cada vértice puede estar influenciado por hasta 4 huesos, así que el cálculo combina las transformaciones con pesos.

---

### En el Vertex Shader

```glsl
const int MAX_BONE_INFLUENCE = 4;

// Para cada vértice
mat4 boneTransform = mat4(0.0);

for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
    if (aBoneIDs[i] >= 0) {
        boneTransform += finalBonesMatrices[aBoneIDs[i]] * aWeights[i];
    }
}

// Posición final del vértice
vec4 localPosition = boneTransform * vec4(aPos, 1.0);
```

**Ejemplo numérico:**
```
Vértice influenciado por 2 huesos:
BoneIDs = [5, 12, -1, -1]
Weights = [0.7, 0.3, 0.0, 0.0]

boneTransform = finalBonesMatrices[5] * 0.7
              + finalBonesMatrices[12] * 0.3

newPosition = boneTransform * originalPosition
```

## Integración con Application

### Inicialización

```cpp
void Application::loadResources() {
    // 1. Cargar modelo
    m_model = std::make_unique<Model>("quadruped.glb");

    // 2. Cargar animación (mismo archivo)
    m_animation = std::make_unique<Animation>("quadruped.glb", m_model.get());

    // 3. Crear animator
    m_animator = std::make_unique<Animator>(m_animation.get());
}
```

### Game Loop

```cpp
void Application::update() {
    // Actualizar animación cada frame
    m_animator->UpdateAnimation(m_deltaTime);
}

void Application::renderModel(float autoMoveDistance) {
    m_shader->use();

    // Obtener matrices de bones
    auto& transforms = m_animator->GetFinalBoneMatrices();

    // Enviar al shader (hasta 100 bones)
    for (size_t i = 0; i < transforms.size(); ++i) {
        m_shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]",
                         transforms[i]);
    }

    // Renderizar modelo
    m_model->Draw(*m_shader);
}
```

## Control de Animación

### Pausa en Frame Específico

```cpp
// Constantes
static constexpr float STOP_FRAME = 155.0f;
static constexpr float TOTAL_ANIMATION_FRAMES = 234.0f;

// Calcular tick de pausa
m_pauseStartTick = (STOP_FRAME / TOTAL_ANIMATION_FRAMES) * m_animation->GetDuration();

// En update()
float currentAnimTime = m_animator->GetCurrentTime();
if (currentAnimTime >= m_pauseStartTick) {
    // Pausar o reducir velocidad
}
```

### Movimiento Sincronizado

```cpp
float calculateCycleDistance(float currentAnimTime) {
    if (currentAnimTime <= m_pauseStartTick) {
        // Movimiento proporcional a la animación
        float activeProgress = currentAnimTime / m_pauseStartTick;
        return activeProgress * m_distancePerCycle;
    }
    return m_distancePerCycle;  // Movimiento completo
}
```

## Limitaciones y Consideraciones

### Límites del Sistema

```cpp
const int MAX_BONES = 100;              // Máximo de huesos
const int MAX_BONE_INFLUENCE = 4;       // Influencias por vértice
```

### Optimización

1. **Culling de Bones**: No renderizar bones fuera de vista
2. **LOD**: Usar menos bones a mayor distancia
3. **Caching**: Reutilizar cálculos cuando sea posible

### Debugging

```cpp
// Verificar número de bones
std::cout << "Numero de bones: " << m_model->GetBoneCount() << std::endl;

// Verificar duración
std::cout << "Duracion: " << m_animation->GetDuration() << " ticks" << std::endl;
std::cout << "TPS: " << m_animation->GetTicksPerSecond() << std::endl;
```

## Troubleshooting Común

### Problema: Modelo deformado
**Causa:** Matrices de bones incorrectas o offset matrix mal calculada
**Solución:** Verificar que bind pose coincida con modelo

### Problema: Animación muy rápida/lenta
**Causa:** TicksPerSecond incorrecto
**Solución:** Ajustar `m_TicksPerSecond` o multiplicador de `dt`

### Problema: Algunos vértices no se mueven
**Causa:** BoneIDs = -1 o Weights = 0
**Solución:** Verificar rigging en Blender, todos los vértices deben tener weights

---

**Próximo:** [Modelos y Texturas](MODELS.md)
