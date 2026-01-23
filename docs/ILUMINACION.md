# Sistema de Iluminación

> 📝 **Nota:** La configuración de iluminación es igual en ambas versiones del proyecto.
> Los shaders de iluminación (`static.frag`, `skeletal_animation.frag`) son compartidos.

## Introducción

El proyecto utiliza el modelo de iluminación **Phong** con una **luz puntual (Point Light)**. Este modelo es eficiente y produce resultados visuales realistas para aplicaciones en tiempo real.

## Configuración Actual

### Luz Puntual

```cpp
// En Application.h - renderAllStaticModels()
m_staticShader->setVec3("lightPos", glm::vec3(0.0f, 5.0f, 20.0f));
m_staticShader->setVec3("lightColor", glm::vec3(1.3f, 1.3f, 1.3f));
m_staticShader->setVec3("viewPos", m_camera.getPosition());
```

| Parámetro | Valor | Descripción |
|-----------|-------|-------------|
| **Posición** | (0, 5, 20) | Arriba y adelante de la escena |
| **Color** | (1.3, 1.3, 1.3) | Blanca, ligeramente sobreexpuesta |
| **viewPos** | Posición de cámara | Para cálculos especulares |

## Modelo Phong

El modelo Phong calcula la iluminación combinando tres componentes:

### 1. Ambient (Luz Ambiente)

La luz que ilumina todo uniformemente, simulando la luz rebotada del entorno.

```glsl
float ambientStrength = 0.35;  // 35% de intensidad base
vec3 ambient = ambientStrength * lightColor;
```

**Efecto:** Sin luz ambient, las áreas no iluminadas directamente serían completamente negras.

### 2. Diffuse (Luz Difusa)

La luz que viene directamente de la fuente y se dispersa uniformemente en la superficie.

```glsl
vec3 norm = normalize(Normal);
vec3 lightDir = normalize(lightPos - FragPos);
float diff = max(dot(norm, lightDir), 0.0);
vec3 diffuse = diff * lightColor;
```

**Cálculo:** Usa el producto punto entre la normal de la superficie y la dirección hacia la luz.
- Si la superficie mira hacia la luz: máxima iluminación
- Si la superficie está perpendicular: sin iluminación
- Si mira opuesto: sin iluminación (clampeado a 0)

### 3. Specular (Brillo Especular)

El brillo que aparece en superficies pulidas/metálicas cuando reflejan la luz hacia la cámara.

```glsl
// Ajustado por propiedades del material PBR
float specularStrength = mix(0.3, 0.9, material.metallic);
float shininess = mix(16.0, 128.0, 1.0 - material.roughness);

vec3 viewDir = normalize(viewPos - FragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
vec3 specular = specularStrength * spec * lightColor;

// Para metales, el especular toma el color del objeto
if (material.metallic > 0.5) {
    specular *= objectColor;
}
```

**Parámetros PBR:**
- `metallic` → Controla intensidad del especular (0.3 a 0.9)
- `roughness` → Controla nitidez del brillo (shininess 16 a 128)

## Combinación Final

```glsl
vec3 result = (ambient + diffuse + specular) * objectColor;
FragColor = vec4(result, alpha);
```

## Diagrama Visual

```
                    Luz (lightPos)
                         ●
                        /|\
                       / | \
                      /  |  \
                     /   |   \
    Diffuse -----> /    |    \ <----- Specular
                  /     |     \        (reflexión)
                 /      |      \
                /       |       \
    ───────────●────────●────────●───────────
              Superficie del modelo
                        |
                        |
                        ▼
                    Cámara (viewPos)
```

## Características y Limitaciones

### ✅ Características Actuales
- Una luz puntual
- Materiales PBR (metallic, roughness)
- Especular ajustado por tipo de material
- Color de luz configurable

### ❌ Limitaciones
- **Sin sombras** - Los objetos no proyectan sombras
- **Sin atenuación** - La luz no se debilita con la distancia
- **Una sola luz** - No hay múltiples fuentes de luz
- **Sin luz direccional** - No hay "sol" infinito

## Posibles Mejoras Futuras

### Atenuación por Distancia

```glsl
float distance = length(lightPos - FragPos);
float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
diffuse *= attenuation;
specular *= attenuation;
```

### Múltiples Luces

```glsl
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

vec3 totalLight = ambient;
for (int i = 0; i < 4; i++) {
    totalLight += calculateLight(lightPositions[i], lightColors[i]);
}
```

### Shadow Mapping

Para agregar sombras se necesitaría:
1. Renderizar la escena desde la perspectiva de la luz (depth map)
2. Comparar profundidades en el fragment shader
3. Oscurecer pixels que están en sombra

---

**Siguiente:** [Sistema de Cámara](CAMERA.md)
