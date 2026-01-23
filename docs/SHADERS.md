# Referencia de Shaders

> 📝 **Nota:** Los shaders y `Shader.h` son compartidos por ambas versiones del proyecto.
> En la versión POO, la gestión de shaders está centralizada en `src/managers/RenderManager.h`.

## Introducción: Entendiendo los Shaders

**¿Qué son los shaders en términos simples?**

Los shaders son programas mini que corren en tu tarjeta gráfica (GPU), no en el procesador principal (CPU). Se escriben en GLSL (OpenGL Shading Language), un lenguaje similar a C.

**¿Por qué necesitamos shaders?**
- La GPU puede procesar millones de vértices y pixels en paralelo
- Es MUCHO más rápido que hacer lo mismo en la CPU
- Cada frame (~60 veces por segundo), los shaders procesan:
  - ~10,000 vértices por modelo
  - ~500,000 pixels en pantalla

**Los dos tipos principales:**

1. **Vertex Shader** - Se ejecuta una vez por cada vértice (punto del modelo)
   - Calcula dónde debe aparecer cada punto en la pantalla
   - Aplica transformaciones (mover, rotar, escalar)
   - Para modelos animados: aplica deformación de huesos

2. **Fragment Shader** - Se ejecuta una vez por cada pixel visible
   - Calcula el color final de cada pixel
   - Aplica texturas (imágenes)
   - Calcula iluminación

**El flujo:**
```
CPU envía datos → GPU recibe → Vertex Shader procesa puntos → Rasterización → Fragment Shader colorea pixels → Pantalla
```

---

## Visión General

El proyecto utiliza 4 programas shader diferentes, cada uno con su propósito específico:

| Shader | Vertex | Fragment | Propósito |
|--------|--------|----------|-----------|
| Skeletal Animation | skeletal_animation.vert | skeletal_animation.frag | Modelos con animación de huesos (200 bones) |
| Static | static.vert | static.frag | Modelos estáticos con materiales PBR |
| Axis | axis.vert | axis.frag | Ejes de coordenadas RGB |
| HUD | hud.vert | hud.frag | Interfaz de usuario (barras de transformación) |

## Sistema de Materiales PBR en Shaders

### MaterialProperties Struct

Todos los shaders de modelos usan esta estructura para materiales:

```glsl
struct MaterialProperties {
    vec3 baseColor;    // Color base del material
    float metallic;    // Factor metálico (0-1)
    float roughness;   // Rugosidad (0-1)
    float opacity;     // Transparencia (0-1)
};
uniform MaterialProperties material;
uniform bool hasTexture;  // ¿Tiene textura difusa?
```

### Lógica de Color

```glsl
vec3 objectColor;
float alpha = material.opacity;

if (hasTexture) {
    vec4 texColor = texture(texture_diffuse1, TexCoords);
    objectColor = texColor.rgb * material.baseColor;  // Combinar textura con color base
    alpha *= texColor.a;
} else {
    objectColor = material.baseColor;  // Solo usar color del material
}
```

### Iluminación Especular con PBR

```glsl
// Metallic afecta intensidad del especular
float specularStrength = mix(0.3, 0.9, material.metallic);

// Roughness afecta nitidez del brillo
float shininess = mix(16.0, 128.0, 1.0 - material.roughness);

// Para materiales metálicos, el especular toma el color del objeto
if (material.metallic > 0.5) {
    specular *= objectColor;
}
```

## 1. Skeletal Animation Shader

### skeletal_animation.vert

**Propósito:** Deformar vértices según huesos de animación (vertex skinning).

#### Inputs (Attributes)

```glsl
layout (location = 0) in vec3 aPos;         // Posición del vértice
layout (location = 1) in vec3 aNormal;      // Vector normal
layout (location = 2) in vec2 aTexCoords;   // Coordenadas UV
layout (location = 3) in ivec4 aBoneIDs;    // IDs de huesos (4 max)
layout (location = 4) in vec4 aWeights;     // Pesos de influencia
```

**Formato de datos:**
```cpp
// En CPU (Vertex struct)
struct Vertex {
    glm::vec3 Position;      // 12 bytes
    glm::vec3 Normal;        // 12 bytes
    glm::vec2 TexCoords;     // 8 bytes
    glm::ivec4 BoneIDs;      // 16 bytes (enteros)
    glm::vec4 Weights;       // 16 bytes (floats)
};  // Total: 64 bytes
```

#### Uniforms

```glsl
uniform mat4 model;                      // Transformación del modelo
uniform mat4 view;                       // Matriz de vista (cámara)
uniform mat4 projection;                 // Proyección perspectiva
uniform mat4 finalBonesMatrices[200];    // Matrices de animación (200 bones máximo)
```

**Enviado desde CPU:**
```cpp
m_shader->setMat4("model", modelMatrix);
m_shader->setMat4("view", viewMatrix);
m_shader->setMat4("projection", projectionMatrix);

// Para cada bone
for (size_t i = 0; i < transforms.size(); ++i) {
    m_shader->setMat4("finalBonesMatrices[" + std::to_string(i) + "]",
                     transforms[i]);
}
```

#### Outputs

```glsl
out vec2 TexCoords;     // Para fragment shader
out vec3 FragPos;       // Posición en espacio mundo
out vec3 Normal;        // Normal transformada
```

#### Código Completo

```glsl
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIDs;
layout (location = 4) in vec4 aWeights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int MAX_BONES = 100;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    // VERTEX SKINNING
    // Calcular transformación combinada de bones
    mat4 boneTransform = mat4(0.0);

    for (int i = 0; i < 4; i++)
    {
        if (aBoneIDs[i] >= 0)  // Validar bone ID
        {
            boneTransform += finalBonesMatrices[aBoneIDs[i]] * aWeights[i];
        }
    }

    // Si no hay bones válidos, usar matriz identidad
    if (boneTransform == mat4(0.0))
    {
        boneTransform = mat4(1.0);
    }

    // Aplicar deformación del esqueleto
    vec4 localPosition = boneTransform * vec4(aPos, 1.0);

    // Calcular posición final en espacio del mundo
    FragPos = vec3(model * localPosition);

    // Transformar normales considerando deformación
    Normal = mat3(transpose(inverse(model * boneTransform))) * aNormal;

    // Pasar coordenadas de textura sin modificar
    TexCoords = aTexCoords;

    // Calcular posición final en clip space
    gl_Position = projection * view * model * localPosition;
}
```

#### Explicación del Skinning

```glsl
// Ejemplo con 2 bones
aBoneIDs = [5, 12, -1, -1]
aWeights = [0.7, 0.3, 0.0, 0.0]

// Iteración 1: i=0
boneTransform += finalBonesMatrices[5] * 0.7

// Iteración 2: i=1
boneTransform += finalBonesMatrices[12] * 0.3

// Iteraciones 3,4: i=2,3 (BoneID = -1, se saltan)

// Resultado final:
// boneTransform = 70% del bone 5 + 30% del bone 12

// Aplicar a vértice:
localPosition = boneTransform * vec4(aPos, 1.0)
```

### skeletal_animation.frag

**Propósito:** Calcular color final con iluminación Phong.

#### Inputs

```glsl
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;
```

#### Uniforms

```glsl
uniform sampler2D texture_diffuse1;   // Textura difusa
uniform vec3 lightPos;                // Posición de la luz
uniform vec3 lightColor;              // Color de la luz
uniform vec3 viewPos;                 // Posición de la cámara
```

#### Outputs

```glsl
out vec4 FragColor;   // Color final del fragmento
```

#### Código Completo

```glsl
#version 460 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D texture_diffuse1;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    // Obtener color de la textura
    vec4 texColor = texture(texture_diffuse1, TexCoords);

    // Fallback si no hay textura
    vec3 objectColor = texColor.rgb;
    if (length(objectColor) < 0.01) {
        objectColor = vec3(0.8, 0.7, 0.6);
    }

    // 1. AMBIENT (Iluminación ambiente)
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    // 2. DIFFUSE (Iluminación difusa)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. SPECULAR (Reflexión especular - efecto metálico)
    float specularStrength = 0.6;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);
    vec3 specular = specularStrength * spec * lightColor;

    // Combinar componentes de iluminación
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}
```

#### Modelo de Iluminación Phong

**1. Ambient (Luz ambiente):**
```glsl
ambient = ambientStrength * lightColor
        = 0.35 * (1.3, 1.3, 1.3)
        = (0.455, 0.455, 0.455)
```
Proporciona iluminación base constante.

**2. Diffuse (Luz difusa):**
```glsl
diff = max(dot(normal, lightDirection), 0.0)
diffuse = diff * lightColor

// Si superficie mira hacia luz (dot = 1.0):
diffuse = 1.0 * (1.3, 1.3, 1.3) = (1.3, 1.3, 1.3)

// Si perpendicular (dot = 0.0):
diffuse = (0, 0, 0)
```
Iluminación basada en ángulo superficie-luz.

**3. Specular (Reflexión especular):**
```glsl
spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess)
specular = specularStrength * spec * lightColor

// Shininess = 64 → reflejos nítidos
// specularStrength = 0.6 → moderado brillo metálico
```
Crea puntos brillantes (efecto metálico).

**Combinación final:**
```glsl
result = (ambient + diffuse + specular) * objectColor
       = luz * color_de_textura
```

## 2. Static Shader

### static.vert

**Diferencia clave:** No procesa bones (más eficiente para modelos estáticos).

```glsl
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
// NO tiene aBoneIDs ni aWeights

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    // Transformación directa sin skinning
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

### static.frag

**Idéntico a skeletal_animation.frag**
- Mismo modelo de iluminación Phong
- Mismos uniforms y outputs
- Código compartible

## 3. Axis Shader

### axis.vert

**Propósito:** Renderizar ejes de coordenadas con colores.

```glsl
#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 Color;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    Color = aColor;
    gl_Position = projection * view * vec4(aPos, 1.0);
    // Sin transformación model (siempre en origen)
}
```

### axis.frag

```glsl
#version 460 core

in vec3 Color;
out vec4 FragColor;

void main()
{
    FragColor = vec4(Color, 1.0);
    // Sin iluminación, solo color puro
}
```

## Uniforms Globales

### Matrices de Transformación

**Model Matrix:**
```cpp
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, position);
model = glm::rotate(model, angle, axis);
model = glm::scale(model, scale);
```

**View Matrix:**
```cpp
glm::mat4 view = glm::lookAt(cameraPos, target, up);
```

**Projection Matrix:**
```cpp
glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),    // FOV
    aspectRatio,
    0.1f,                   // Near plane
    100.0f                  // Far plane
);
```

### Parámetros de Iluminación

**Configuración actual:**
```cpp
lightPos = (0, 5, 20)          // Desde eje Z
lightColor = (1.3, 1.3, 1.3)   // Blanco moderado
viewPos = camera.getPosition()  // Posición de cámara
```

## Optimización de Shaders

### Constantes vs Uniforms

**Constantes (compile-time):**
```glsl
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 4;
```
Más eficiente que uniforms para valores que no cambian.

### Precalcular en CPU

```cpp
// En lugar de calcular en shader:
glm::mat4 normalMatrix = glm::transpose(glm::inverse(model));
shader->setMat4("normalMatrix", normalMatrix);

// Shader:
Normal = mat3(normalMatrix) * aNormal;
```

### Limitar Operaciones Costosas

```glsl
// pow() es costoso
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 64);

// Alternativa para shininess bajo:
float spec = max(dot(viewDir, reflectDir), 0.0);
spec = spec * spec * spec * spec;  // shininess = 4
```

## Debugging de Shaders

### Verificar Compilación

```cpp
Shader shader("vertex.vert", "fragment.frag");
// Errores se imprimen automáticamente en consola
```

### Visualizar Normales

```glsl
// En fragment shader
void main() {
    // Mostrar normales como colores
    FragColor = vec4(normalize(Normal) * 0.5 + 0.5, 1.0);
}
```

### Visualizar UVs

```glsl
void main() {
    // Mostrar coordenadas de textura
    FragColor = vec4(TexCoords, 0.0, 1.0);
}
```

### Visualizar Weights de Bones

```glsl
// En vertex shader
out vec4 debugWeights;

void main() {
    debugWeights = aWeights;
    // ...
}

// En fragment shader
void main() {
    FragColor = vec4(debugWeights.xyz, 1.0);
}
```

## Modificaciones Comunes

### Cambiar Modelo de Iluminación

**Añadir Textura Normal (Normal Mapping):**
```glsl
// Vertex shader - añadir output
out mat3 TBN;  // Tangent-Bitangent-Normal matrix

// Fragment shader
uniform sampler2D texture_normal1;

vec3 normal = texture(texture_normal1, TexCoords).rgb;
normal = normalize(normal * 2.0 - 1.0);  // [-1, 1]
normal = normalize(TBN * normal);
```

### Añadir Múltiples Luces

```glsl
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

vec3 totalDiffuse = vec3(0.0);
vec3 totalSpecular = vec3(0.0);

for (int i = 0; i < 4; i++) {
    // Calcular diffuse y specular para cada luz
    // ...
    totalDiffuse += diffuse[i];
    totalSpecular += specular[i];
}

result = (ambient + totalDiffuse + totalSpecular) * objectColor;
```

### Añadir Fog

```glsl
uniform vec3 fogColor;
uniform float fogDensity;

void main() {
    // Calcular iluminación normal
    vec3 result = ...;

    // Calcular factor de niebla
    float dist = length(viewPos - FragPos);
    float fogFactor = 1.0 - exp(-fogDensity * dist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    // Mezclar con niebla
    result = mix(result, fogColor, fogFactor);
    FragColor = vec4(result, 1.0);
}
```

## Versiones de GLSL

**GLSL 4.60 (usado en este proyecto):**
- Compatible con OpenGL 4.6
- Soporta todas las características modernas
- Retrocompatible con versiones anteriores

**Características usadas:**
- `layout (location = N)` - Especificar ubicación de attributes
- `texture()` - Función de muestreo de texturas
- Arrays de uniforms - `uniform mat4 array[200]`
- Integer attributes - `ivec4`

---

## 4. HUD Shader

### hud.vert

**Propósito:** Posicionar rectángulos 2D para la interfaz de usuario.

```glsl
#version 460 core

layout (location = 0) in vec2 aPos;  // Posición 2D en NDC (-1 a 1)

void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);  // Directamente a clip space
}
```

**Notas:**
- No usa matrices de transformación (proyección/vista)
- Las posiciones se dan directamente en Normalized Device Coordinates
- z = 0.0 significa "en el plano de la pantalla"

### hud.frag

**Propósito:** Colorear las barras del HUD con colores sólidos.

```glsl
#version 460 core

out vec4 FragColor;

uniform vec3 color;  // Color de la barra (RGB)

void main() {
    FragColor = vec4(color, 1.0);  // Color sólido, opaco
}
```

**Uso en Application.h:**

```cpp
void renderHUD() {
    glDisable(GL_DEPTH_TEST);  // HUD siempre visible
    
    m_hudShader->use();
    
    // Barra verde para modo TRANSLATE
    glm::vec3 translateColor = (m_selectionState.mode == TransformMode::TRANSLATE) 
        ? glm::vec3(0.2f, 0.8f, 0.2f)   // Verde brillante (activo)
        : glm::vec3(0.3f, 0.3f, 0.3f);  // Gris (inactivo)
    
    drawHUDBar(-0.95f, 0.90f, 0.08f, 0.04f, translateColor);
    
    glEnable(GL_DEPTH_TEST);
}
```

**Sistema de colores del HUD:**

| Elemento | Color Activo | Color Inactivo |
|----------|--------------|----------------|
| TRANSLATE | Verde (0.2, 0.8, 0.2) | Gris (0.3, 0.3, 0.3) |
| ROTATE | Azul (0.2, 0.2, 0.8) | Gris (0.3, 0.3, 0.3) |
| SCALE | Amarillo (0.8, 0.8, 0.2) | Gris (0.3, 0.3, 0.3) |
| Eje X | Rojo (0.8, 0.2, 0.2) | Gris (0.5, 0.5, 0.5) |
| Eje Y | Verde (0.2, 0.8, 0.2) | Gris (0.5, 0.5, 0.5) |
| Eje Z | Azul (0.2, 0.2, 0.8) | Gris (0.5, 0.5, 0.5) |

---

**Siguiente:** [Configuración y Compilación](CONFIGURACION.md)
