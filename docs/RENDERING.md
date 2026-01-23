# Sistema de Renderizado

> 📝 **Nota sobre versiones:**
> - **Versión original:** Renderizado en `Application.h` (funciones `render()`, `renderModel()`)
> - **Versión POO:** Renderizado en `src/managers/RenderManager.h`
> 
> Los conceptos de esta documentación aplican a ambas versiones.

## Introducción: ¿Cómo se Dibuja Todo en Pantalla?

**Para entender el concepto general:**

Renderizar significa "dibujar" los modelos 3D en la pantalla 2D. Cada frame (60 veces por segundo), el programa debe:
1. Borrar la pantalla anterior
2. Calcular dónde está cada punto de cada modelo
3. Aplicar texturas y materiales PBR
4. Aplicar iluminación
5. Renderizar el HUD de transformación
6. Mostrar el resultado final

**El Viaje de un Vértice (De 3D a 2D):**
```
Punto 3D en el modelo → Animación (mover huesos) → Transformaciones (posición/rotación/escala)
    ↓
Proyección (3D → 2D) → Rasterización (convertir a pixels) → Iluminación + Materiales
    ↓
Pixel final en pantalla
```

**¿Por qué es rápido?**
- La **GPU** (tarjeta gráfica) procesa millones de puntos en paralelo
- Los **shaders** son programas mini que corren en la GPU
- Todo está optimizado para hacer esto 60+ veces por segundo

---

## Sistema de Materiales PBR

### ¿Qué es PBR (Physically Based Rendering)?

PBR es una técnica de renderizado que simula cómo los materiales reales interactúan con la luz. En lugar de ajustar valores arbitrarios, usamos propiedades físicas reales.

### Estructura de Material

```cpp
struct Material {
    glm::vec3 baseColor;   // Color base del material (RGB)
    float metallic;        // 0 = no metálico, 1 = totalmente metálico
    float roughness;       // 0 = pulido (brillante), 1 = rugoso (mate)
    float opacity;         // 0 = transparente, 1 = opaco
};
```

### Extracción de Materiales desde GLB

El sistema extrae automáticamente las propiedades PBR de archivos GLTF/GLB:

```cpp
// En Model.h - processMesh()
aiColor4D baseColor;
if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS) {
    meshMaterial.baseColor = glm::vec3(baseColor.r, baseColor.g, baseColor.b);
}

float metallic;
if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
    meshMaterial.metallic = metallic;
}

float roughness;
if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
    meshMaterial.roughness = roughness;
}
```

### Uso en Shaders

```glsl
// static.frag / skeletal_animation.frag
struct MaterialProperties {
    vec3 baseColor;
    float metallic;
    float roughness;
    float opacity;
};
uniform MaterialProperties material;
uniform bool hasTexture;

void main() {
    vec3 objectColor;
    
    if (hasTexture) {
        objectColor = texture(texture_diffuse1, TexCoords).rgb * material.baseColor;
    } else {
        objectColor = material.baseColor;  // Usar solo color del material
    }
    
    // Specular ajustado por metallic y roughness
    float specularStrength = mix(0.3, 0.9, material.metallic);
    float shininess = mix(16.0, 128.0, 1.0 - material.roughness);
}
```

---

## Pipeline de Renderizado

### Visión General

El sistema utiliza el pipeline de renderizado moderno de OpenGL 4.6 Core Profile con vertex y fragment shaders programables.

**Explicación del Pipeline (paso a paso):**
- **Vertex Data**: Los puntos, normales y texturas del modelo (en RAM)
- **Vertex Shader**: Programa que calcula la posición final de cada punto (en GPU)
- **Rasterización**: OpenGL automáticamente convierte triángulos en pixels
- **Fragment Shader**: Programa que calcula el color de cada pixel (en GPU)
- **Framebuffer**: El resultado final que ves en pantalla

```
Vertex Data → Vertex Shader → Rasterización → Fragment Shader → Framebuffer
     ↓              ↓                                  ↓
  VAO/VBO    Transformaciones                    Iluminación
  Texturas   Skinning (bones)                    Color final
```

## Proceso de Renderizado Completo

### Frame Rendering Flow

```cpp
void Application::render() {
    // 1. Limpiar buffers
    glClearColor(0.53f, 0.81f, 0.92f, 1.0f);  // Fondo celeste
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 2. Calcular datos de animación
    float currentAnimTime = m_animator->GetCurrentTime();
    float autoMoveDistance = calculateMovement();

    // 3. Renderizar escena estática (fondo)
    renderStaticModel();

    // 4. Renderizar modelo animado (foreground)
    renderModel(autoMoveDistance);

    // 5. Renderizar ejes de coordenadas
    renderAxis();
}
```

## Shaders

**¿Qué son los Shaders?**

Los shaders son programas pequeños escritos en GLSL (OpenGL Shading Language) que corren directamente en la GPU. Son como "mini-programas" que procesan cada vértice y cada pixel.

**¿Por qué necesitamos diferentes shaders?**
- **Skeletal Animation Shader**: Para el cuadrúpedo que se anima (necesita procesar huesos)
- **Static Shader**: Para la escena que no se anima (más simple y rápido)
- **Axis Shader**: Para dibujar los ejes XYZ (muy simple, solo líneas de colores)

**Tipos de shaders en este proyecto:**
1. **Vertex Shader**: Se ejecuta una vez POR CADA vértice (~10,000 veces por modelo)
2. **Fragment Shader**: Se ejecuta una vez POR CADA pixel visible (~500,000 veces por frame)

---

### 1. Skeletal Animation Shader

#### Vertex Shader (skeletal_animation.vert)

**Propósito:** Transformar vértices aplicando deformación esquelética (skinning).

**¿Qué hace en términos simples?**
1. Recibe la posición original del vértice
2. Recibe qué huesos afectan este vértice y cuánto (BoneIDs y Weights)
3. Combina las transformaciones de esos huesos
4. Calcula la nueva posición del vértice después de la animación
5. Proyecta esa posición 3D a 2D para la pantalla

**Inputs:**
```glsl
layout (location = 0) in vec3 aPos;         // Posición del vértice
layout (location = 1) in vec3 aNormal;      // Normal
layout (location = 2) in vec2 aTexCoords;   // Coordenadas de textura
layout (location = 3) in ivec4 aBoneIDs;    // IDs de huesos (4 max)
layout (location = 4) in vec4 aWeights;     // Pesos de influencia
```

**Uniforms:**
```glsl
uniform mat4 model;                         // Transformación del modelo
uniform mat4 view;                          // Transformación de vista
uniform mat4 projection;                    // Proyección perspectiva
uniform mat4 finalBonesMatrices[100];       // Matrices de animación
```

**Outputs:**
```glsl
out vec2 TexCoords;    // Para fragment shader
out vec3 FragPos;      // Posición en espacio mundo
out vec3 Normal;       // Normal transformada
```

**Proceso de Skinning:**
```glsl
// Calcular transformación combinada de bones
mat4 boneTransform = mat4(0.0);

for (int i = 0; i < 4; i++) {
    if (aBoneIDs[i] >= 0) {
        boneTransform += finalBonesMatrices[aBoneIDs[i]] * aWeights[i];
    }
}

// Si no hay bones válidos, usar matriz identidad
if (boneTransform == mat4(0.0)) {
    boneTransform = mat4(1.0);
}

// Aplicar deformación
vec4 localPosition = boneTransform * vec4(aPos, 1.0);
```

#### Fragment Shader (skeletal_animation.frag)

**Propósito:** Calcular color final con iluminación Phong.

**Inputs:**
```glsl
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
```

**Uniforms:**
```glsl
uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;      // Posición de la luz
uniform vec3 lightColor;    // Color de la luz
uniform vec3 viewPos;       // Posición de la cámara
```

**Modelo de Iluminación Phong:**

**¿Por qué usamos el modelo Phong?**
Es un modelo de iluminación realista pero eficiente que calcula cómo la luz afecta cada superficie. Combina 3 componentes para simular cómo vemos objetos en la vida real.

**Los 3 componentes explicados:**

1. **Ambient (Ambiente)** - La luz general que ilumina todo por igual
```glsl
float ambientStrength = 0.35;  // 35% de luz base
vec3 ambient = ambientStrength * lightColor;
```
**Analogía**: Como la luz del día que entra por la ventana, ilumina todo aunque no le dé luz directa.
**Efecto**: Sin ambient, las partes no iluminadas se verían completamente negras (irreal).

2. **Diffuse (Difusa)** - La luz que viene directamente de la fuente
```glsl
vec3 norm = normalize(Normal);              // Normal de la superficie
vec3 lightDir = normalize(lightPos - FragPos);  // Dirección hacia la luz
float diff = max(dot(norm, lightDir), 0.0);    // Ángulo entre normal y luz
vec3 diffuse = diff * lightColor;
```
**Analogía**: Como una lámpara apuntando a una pared. Donde apunta directamente es más brillante.
**Cálculo**: Usa el producto punto (dot product) - si la superficie mira hacia la luz, recibe más luz.
**Efecto**: Crea las áreas claras y oscuras que definen la forma del objeto.

3. **Specular (Especular)** - El brillo en superficies brillantes/metálicas
```glsl
float specularStrength = 0.6;  // Intensidad del brillo
vec3 viewDir = normalize(viewPos - FragPos);    // Dirección hacia la cámara
vec3 reflectDir = reflect(-lightDir, norm);     // Reflexión de la luz
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);  // Shininess=64
vec3 specular = specularStrength * spec * lightColor;
```
**Analogía**: Como el reflejo del sol en un auto recién lavado o en metal pulido.
**Cálculo**: Verifica si estamos mirando en la dirección donde la luz se refleja.
**Shininess (64)**: Número más alto = brillo más concentrado (más metálico). Más bajo = brillo difuso.
**Efecto**: Crea los "puntos brillantes" que hacen que el cuadrúpedo se vea metálico.

4. **Combinación final**
```glsl
vec3 result = (ambient + diffuse + specular) * objectColor;
FragColor = vec4(result, 1.0);
```

### 2. Static Model Shader

**Diferencias con Skeletal Shader:**
- No procesa bones (más eficiente)
- No tiene inputs de BoneIDs ni Weights
- No necesita uniforms de finalBonesMatrices
- Mismo modelo de iluminación

**Vertex Shader (static.vert):**
```glsl
void main() {
    // Transformación simple sin skinning
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

### 3. Axis Shader

**Propósito:** Renderizar ejes de coordenadas RGB.

**Vertex Shader (axis.vert):**
```glsl
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform mat4 projection;
uniform mat4 view;

void main() {
    Color = aColor;
    gl_Position = projection * view * vec4(aPos, 1.0);
}
```

**Fragment Shader (axis.frag):**
```glsl
in vec3 Color;
out vec4 FragColor;

void main() {
    FragColor = vec4(Color, 1.0);
}
```

## Configuración de Iluminación

### Configuración Actual

**Posición de luz:**
```cpp
glm::vec3(0.0f, 5.0f, 20.0f)  // Desde el eje Z (atrás)
```

**Color de luz:**
```cpp
glm::vec3(1.3f, 1.3f, 1.3f)  // Blanco moderado para efecto metálico
```

**Parámetros de shader:**
- Ambient: 0.35 (35% iluminación base)
- Specular: 0.6 (60% reflejos)
- Shininess: 64 (reflejos nítidos)

### Modificar Iluminación

**Para cambiar intensidad:**
```cpp
// En Application.h, renderModel() o renderStaticModel()
m_shader->setVec3("lightColor", glm::vec3(intensidad, intensidad, intensidad));
```

**Para cambiar posición:**
```cpp
m_shader->setVec3("lightPos", glm::vec3(x, y, z));
```

**Para cambiar parámetros Phong:**
```glsl
// En skeletal_animation.frag o static.frag
float ambientStrength = 0.35;   // Luz base
float specularStrength = 0.6;   // Brillo
float shininess = 64;           // Nitidez de reflejos
```

## Sistema de Texturas

### Carga de Texturas

```cpp
unsigned int TextureFromFile(path, directory) {
    // 1. Generar textura OpenGL
    glGenTextures(1, &textureID);

    // 2. Cargar imagen con stb_image
    data = stbi_load(filename, &width, &height, &nrComponents);

    // 3. Determinar formato
    GLenum format = (nrComponents == 3) ? GL_RGB : GL_RGBA;

    // 4. Subir a GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
                 0, format, GL_UNSIGNED_BYTE, data);

    // 5. Generar mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);

    // 6. Configurar filtrado
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}
```

### Texturas Embebidas

Para texturas dentro del GLB:
```cpp
if (str.C_Str()[0] == '*') {
    int index = std::atoi(&str.C_Str()[1]);
    const aiTexture* embeddedTexture = scene->mTextures[index];

    // Cargar desde memoria
    data = stbi_load_from_memory(
        (unsigned char*)embeddedTexture->pcData,
        embeddedTexture->mWidth,
        &width, &height, &nrComponents, 0
    );
}
```

### Uso en Shaders

```glsl
uniform sampler2D texture_diffuse1;

void main() {
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    // ...
}
```

## Optimizaciones de Renderizado

### 1. Depth Testing
```cpp
glEnable(GL_DEPTH_TEST);  // Activado en initOpenGL()
```

### 2. Face Culling (No activado actualmente)
```cpp
// Para activar:
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
```

### 3. VAO State Management
```cpp
// Cada Mesh mantiene su propio VAO
glBindVertexArray(VAO);  // Bind una vez
glDrawElements(...);     // Dibujar
glBindVertexArray(0);    // Unbind
```

### 4. Shader Switching
```cpp
// Minimizar cambios de shader
renderStaticModel();  // Usa m_staticShader
renderModel();        // Usa m_shader (skeletal)
renderAxis();         // Usa m_axisShader
```

## Matrices de Transformación

### Model Matrix

**Para modelo animado (cuadrúpedo):**
```cpp
glm::mat4 modelMat = glm::mat4(1.0f);  // Identidad
modelMat = glm::translate(modelMat, glm::vec3(0.0f, 0.0f, autoMoveDistance));
// Sin escala = tamaño original
```

**Para modelo estático (escena):**

**Configuración actual (escalado 100x con rotación):**
```cpp
glm::mat4 modelMat = glm::mat4(1.0f);
float rotationAngle = glfwGetTime() * 10.0f;  // Rotar 10 grados/segundo
modelMat = glm::rotate(modelMat, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
modelMat = glm::scale(modelMat, glm::vec3(100.0f, 100.0f, 100.0f));
```

**Para escena normal (sin rotación ni escala):**
```cpp
glm::mat4 modelMat = glm::mat4(1.0f);  // Solo matriz identidad
// Sin transformaciones = tamaño original, sin rotación
```

**Personalización:**

*Cambiar escala:*
```cpp
// Escala 10x
modelMat = glm::scale(modelMat, glm::vec3(10.0f, 10.0f, 10.0f));

// Escala 50x
modelMat = glm::scale(modelMat, glm::vec3(50.0f, 50.0f, 50.0f));

// Escala diferente por eje (ej: más ancho en X)
modelMat = glm::scale(modelMat, glm::vec3(150.0f, 100.0f, 100.0f));
```

*Cambiar velocidad de rotación:*
```cpp
// Más lento (5 grados/segundo)
float rotationAngle = glfwGetTime() * 5.0f;

// Más rápido (20 grados/segundo)
float rotationAngle = glfwGetTime() * 20.0f;

// Sin rotación (comentar la línea)
// float rotationAngle = glfwGetTime() * 10.0f;
// modelMat = glm::rotate(...);
```

*Cambiar eje de rotación:*
```cpp
// Rotar en eje Y (actual - vertical)
modelMat = glm::rotate(modelMat, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));

// Rotar en eje X (horizontal)
modelMat = glm::rotate(modelMat, glm::radians(rotationAngle), glm::vec3(1.0f, 0.0f, 0.0f));

// Rotar en eje Z (profundidad)
modelMat = glm::rotate(modelMat, glm::radians(rotationAngle), glm::vec3(0.0f, 0.0f, 1.0f));
```

**Nota importante sobre planos de proyección:**
Si escalas la escena a valores muy grandes (>50x), ajusta también los planos de proyección:

```cpp
// Para escenas muy grandes (100x o más)
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),
    aspectRatio,
    1.0f,      // Near plane (aumentado de 0.1f)
    10000.0f   // Far plane (aumentado de 100.0f)
);

// Para escenas normales
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),
    aspectRatio,
    0.1f,    // Near plane
    100.0f   // Far plane
);
```

### View Matrix
```cpp
glm::mat4 view = m_camera.getViewMatrix();

// Internamente en Camera:
glm::lookAt(position, target, worldUp);
```

### Projection Matrix
```cpp
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),              // FOV
    (float)SCR_WIDTH / SCR_HEIGHT,    // Aspect ratio
    0.1f,                             // Near plane
    100.0f                            // Far plane
);
```

### Cálculo MVP en Shader
```glsl
gl_Position = projection * view * model * vec4(aPos, 1.0);
```

## Ejes de Coordenadas

### Configuración de Vértices
```cpp
float axisVertices[] = {
    // Posición        Color (RGB)
    0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Eje X origen (Rojo)
    3.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Eje X fin

    0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Eje Y origen (Verde)
    0.0f, 3.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // Eje Y fin

    0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Eje Z origen (Azul)
    0.0f, 0.0f, 3.0f,  0.0f, 0.0f, 1.0f   // Eje Z fin
};
```

### Renderizado
```cpp
glLineWidth(3.0f);
glDrawArrays(GL_LINES, 0, 6);  // 6 vértices = 3 líneas
glLineWidth(1.0f);
```

## Estado de OpenGL

### Inicialización
```cpp
void Application::initOpenGL() {
    gladLoadGL(...);
    glEnable(GL_DEPTH_TEST);
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION);
}
```

### Por Frame
```cpp
// Limpiar
glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

// Renderizar...

// Swap
glfwSwapBuffers(m_window);
```

## Debugging de Renderizado

### Verificar Shaders
```cpp
// Shader compila automáticamente y reporta errores en consola
Shader shader("vertex.vert", "fragment.frag");
```

### Verificar Modelos
```cpp
std::cout << "Modelo cargado: " << m_model->getMeshes().size() << " mallas";
```

### Verificar Texturas
```cpp
void verifyTextures() {
    int totalTextures = 0;
    for (const auto& mesh : m_model->getMeshes()) {
        totalTextures += mesh.getTextures().size();
    }
    if (totalTextures == 0) {
        std::cout << "ADVERTENCIA: No se cargaron texturas";
    }
}
```

---

**Próximo:** [Sistema de Animación](ANIMATION.md)
