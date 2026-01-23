# Sistema de Interactividad (Drag & Drop 3D + Transformaciones)

## Introducción: ¿Qué es el Sistema de Interactividad?

> 📝 **Nota sobre versiones:**
> - **Versión Original**: El sistema está integrado en `Application.h`
> - **Versión POO**: Está separado en `InputManager.h`, `SelectionManager.h` y `TransformController.h`
> 
> Esta documentación cubre los conceptos aplicables a ambas versiones.

**Para entender el concepto general:**

Este sistema permite "tocar", seleccionar y transformar objetos 3D con el mouse y teclado. Incluye:
1. **Selección de objetos** - Click para seleccionar instrumental dental
2. **Arrastre (Drag & Drop)** - Mover objetos con el mouse
3. **Sistema de Transformación** - Trasladar, rotar y escalar objetos seleccionados
4. **HUD Visual** - Feedback visual del modo de transformación activo

**El desafío principal:**
- Tu pantalla es 2D (X, Y) - pixeles en ancho y alto
- El mundo 3D es 3D (X, Y, Z) - posiciones en espacio
- ¿Cómo convertir el click del mouse (2D) en una posición 3D?

**La solución: Ray Casting**

Imagina que disparas un "rayo invisible" desde la cámara, atravesando el punto donde hiciste click en la pantalla, hacia el mundo 3D. Si ese rayo toca un objeto, ¡lo seleccionamos!

---

## Sistema de Transformación

### Modos de Transformación

```cpp
enum class TransformMode {
    TRANSLATE,  // Mover objeto en el espacio
    ROTATE,     // Rotar objeto sobre su centro
    SCALE       // Cambiar tamaño del objeto
};
```

### Restricción por Eje

```cpp
enum class TransformAxis {
    ALL,  // Transformar en todos los ejes
    X,    // Solo eje X (rojo)
    Y,    // Solo eje Y (verde)
    Z     // Solo eje Z (azul)
};
```

### Estado de Selección

```cpp
struct SelectionState {
    bool hasSelection = false;           // ¿Hay objeto seleccionado?
    int selectedIndex = -1;              // Índice del objeto seleccionado
    bool isStatic = true;                // ¿Es modelo estático o animado?
    TransformMode mode = TransformMode::TRANSLATE;
    TransformAxis axis = TransformAxis::ALL;
};
```

### Rotación Alrededor del Centro del Modelo

Un aspecto importante: los objetos rotan **alrededor de su propio centro**, no del origen del mundo. Esto requiere una secuencia especial de transformaciones:

```cpp
// En renderAllStaticModels() y handleMouseClick()
glm::mat4 modelMat = glm::mat4(1.0f);

// Obtener centro del modelo desde transformación raíz de Blender
glm::mat4 rootTransform = modelData.model->GetRootTransform();
glm::vec3 modelCenter = glm::vec3(rootTransform[3]);

// 1. Mover a posición final
modelMat = glm::translate(modelMat, modelData.position);

// 2. Mover al centro del modelo
modelMat = glm::translate(modelMat, modelCenter);

// 3. Aplicar rotación (ahora rota sobre el centro)
modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.x), glm::vec3(1, 0, 0));
modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.y), glm::vec3(0, 1, 0));
modelMat = glm::rotate(modelMat, glm::radians(modelData.rotation.z), glm::vec3(0, 0, 1));

// 4. Mover de vuelta
modelMat = glm::translate(modelMat, -modelCenter);

// 5. Aplicar escala
modelMat = glm::scale(modelMat, modelData.scale);

// 6. Aplicar transformación raíz del modelo
modelMat = modelMat * rootTransform;
```

**¿Por qué es importante?**
Sin esta secuencia, al rotar un objeto este "orbitaría" alrededor del origen del mundo en lugar de girar sobre sí mismo.

---

## HUD (Heads-Up Display)

### Renderizado del HUD

El HUD muestra barras de colores indicando el modo y eje activo:

```cpp
void renderHUD() {
    // Desactivar depth test para que siempre esté visible
    glDisable(GL_DEPTH_TEST);
    
    m_hudShader->use();
    
    // Barras de modo (fila superior)
    drawHUDBar(-0.95f, 0.90f, 0.08f, 0.04f, 
               m_selectionState.mode == TransformMode::TRANSLATE 
                   ? glm::vec3(0.2f, 0.8f, 0.2f)   // Verde brillante
                   : glm::vec3(0.3f, 0.3f, 0.3f)); // Gris
    
    // ... más barras para ROTATE y SCALE
    
    // Barras de eje (fila inferior)
    // Rojo=X, Verde=Y, Azul=Z, Gris=ALL
    
    glEnable(GL_DEPTH_TEST);
}
```

---

## Conceptos Fundamentales

### ¿Qué es Ray Casting?

Ray casting es una técnica para determinar qué objeto 3D está bajo el cursor del mouse. Consiste en:
1. Convertir las coordenadas del mouse (2D) a un rayo 3D
2. Probar si ese rayo intersecta con objetos en la escena
3. Seleccionar el objeto más cercano que fue tocado

**Ventajas:**
- Selección precisa en 3D
- Funciona independiente de la orientación de la cámara
- Usado en videojuegos, CAD, modelado 3D

**¿Cómo se diferencia de otras técnicas?**
- **2D Bounding Box**: Solo funciona con objetos alineados a la pantalla (no rota con cámara)
- **Ray Casting**: Funciona en verdadero 3D, considera la perspectiva

### ¿Qué es un AABB (Axis-Aligned Bounding Box)?

Un AABB es una "caja invisible" que rodea completamente un objeto 3D. Es la forma más simple de representar el área que ocupa un objeto.

**Características:**
- Siempre alineada con los ejes X, Y, Z (no rota)
- Definida por dos puntos: `min` (esquina inferior) y `max` (esquina superior)
- Rápida para calcular intersecciones
- Se recalcula con la transformación completa del objeto

**Importante:** El AABB se calcula usando la **misma transformación** que el renderizado, incluyendo rotación y escala. Esto asegura que la detección de clicks coincida con lo que el usuario ve en pantalla.

---

## Estructura de Datos

### 1. AABB (Bounding Box)

```cpp
struct AABB {
    glm::vec3 min;  // Esquina con los valores mínimos de X, Y, Z
    glm::vec3 max;  // Esquina con los valores máximos de X, Y, Z

    AABB() : min(FLT_MAX), max(-FLT_MAX) {}

    // Expande el AABB para incluir un nuevo punto
    void expand(const glm::vec3& point) {
        min = glm::min(min, point);
        max = glm::max(max, point);
    }

    // Calcula el centro del AABB
    glm::vec3 center() const {
        return (min + max) * 0.5f;
    }
};
```

**¿Cómo funciona `expand()`?**

Imagina que estás construyendo el AABB del cuadrúpedo:
1. Empiezas con min=(infinito, infinito, infinito) y max=(-infinito, -infinito, -infinito)
2. Por cada vértice del modelo (ej: pata, cabeza, cola):
   - Si el vértice está más a la izquierda que `min.x`, actualiza `min.x`
   - Si el vértice está más arriba que `max.y`, actualiza `max.y`
   - Etc. para todos los ejes
3. Al final, tienes la caja mínima que contiene todos los vértices

**Ejemplo numérico:**
```cpp
AABB box;
box.expand(glm::vec3(1, 2, 3));   // min=(1,2,3), max=(1,2,3)
box.expand(glm::vec3(-1, 5, 2));  // min=(-1,2,2), max=(1,5,3)
box.expand(glm::vec3(2, 1, 4));   // min=(-1,1,2), max=(2,5,4)
// Resultado: Caja que va de (-1,1,2) a (2,5,4)
```

### 2. DraggableObject (Tipo de Objeto)

```cpp
enum class DraggableObject {
    NONE,       // Ningún objeto seleccionado
    QUADRUPED,  // Cuadrúpedo seleccionado
    SCENE       // Escena seleccionada
};
```

**¿Por qué un enum?**

En lugar de usar números (0, 1, 2) que no dicen nada, usamos nombres descriptivos. Es más legible y evita errores.

### 3. Variables de Estado

```cpp
// En Application.h
DraggableObject m_selectedObject;  // ¿Qué objeto está seleccionado?
bool m_isDragging;                  // ¿Está el usuario arrastrando?
glm::vec3 m_quadrupedPosition;     // Posición arrastrable del cuadrúpedo
glm::vec3 m_scenePosition;         // Posición arrastrable de la escena
glm::vec3 m_dragOffset;            // Offset entre click y centro del objeto
float m_dragPlaneDistance;         // Distancia del plano de arrastre
```

**¿Por qué necesitamos `m_dragOffset`?**

Imagina que haces click en la oreja del cuadrúpedo (no en su centro). Sin offset, al arrastrar, el centro del cuadrúpedo "saltaría" a tu cursor. Con offset, mantenemos la posición relativa del click.

**Analogía:**
Es como arrastrar una ventana en Windows: no importa si haces click en la esquina o el centro de la barra de título, la ventana mantiene su forma y se mueve naturalmente.

---

## Ray Casting: De 2D a 3D

### Conversión de Coordenadas de Pantalla a Rayo

```cpp
glm::vec3 screenToWorldRay(double mouseX, double mouseY) {
    // 1. Normalizar coordenadas de pantalla a rango [-1, 1]
    float x = (2.0f * mouseX) / SCR_WIDTH - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / SCR_HEIGHT;  // Invertir Y

    // 2. Crear vector en clip space
    glm::vec4 rayClip(x, y, -1.0f, 1.0f);

    // 3. Convertir a eye space (deshacer proyección)
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
                                           (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                           0.1f, 10000.0f);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    // 4. Convertir a world space (deshacer vista)
    glm::vec3 rayWorld = glm::vec3(glm::inverse(m_camera.getViewMatrix()) * rayEye);
    return glm::normalize(rayWorld);
}
```

**Desglose paso a paso:**

**Paso 1: Normalización (Pantalla → NDC)**
- Entrada: mouseX=640, mouseY=360 (centro de pantalla 1280x720)
- Proceso:
  ```cpp
  x = (2.0 * 640) / 1280 - 1.0 = 1280/1280 - 1.0 = 0.0
  y = 1.0 - (2.0 * 360) / 720 = 1.0 - 1.0 = 0.0
  ```
- Resultado: (0, 0) = centro en NDC (Normalized Device Coordinates)

**Paso 2: Clip Space**
- Crea un vector 4D que representa un punto en la pantalla
- z=-1.0 significa "en el plano cercano de la cámara"
- w=1.0 es necesario para las transformaciones homogéneas

**Paso 3: Eye Space (Espacio de Vista)**
- Deshace la proyección perspectiva
- Ahora el rayo está en "espacio de la cámara" (como si la cámara estuviera en el origen)

**Paso 4: World Space (Espacio del Mundo)**
- Deshace la transformación de vista
- Ahora el rayo está en coordenadas del mundo real
- Normalizamos para obtener solo la dirección (vector unitario)

**Resultado Final:**
Un vector 3D que apunta desde la cámara hacia donde hiciste click en el mundo 3D.

---

## Detección de Colisión: Ray-AABB Intersection

### Algoritmo de Intersección

```cpp
bool rayAABBIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir,
                         const AABB& aabb, float& distance) {
    // 1. Calcular inversa de la dirección del rayo
    glm::vec3 invDir = 1.0f / rayDir;

    // 2. Calcular distancias a los planos de la caja
    glm::vec3 t0s = (aabb.min - rayOrigin) * invDir;
    glm::vec3 t1s = (aabb.max - rayOrigin) * invDir;

    // 3. Asegurar que t0 < t1 (pueden intercambiarse si invDir es negativo)
    glm::vec3 tsmaller = glm::min(t0s, t1s);
    glm::vec3 tbigger = glm::max(t0s, t1s);

    // 4. Encontrar el punto de entrada y salida
    float tmin = glm::max(glm::max(tsmaller.x, tsmaller.y), tsmaller.z);
    float tmax = glm::min(glm::min(tbigger.x, tbigger.y), tbigger.z);

    // 5. Verificar intersección
    if (tmin > tmax || tmax < 0.0f) return false;

    // 6. Calcular distancia
    distance = tmin > 0.0f ? tmin : tmax;
    return true;
}
```

**¿Cómo funciona este algoritmo?**

**Concepto clave:** Un AABB está formado por 6 planos (caras). El rayo debe entrar por una cara y salir por otra.

**Paso 1-2: Calcular intersecciones con planos**
- Para cada eje (X, Y, Z), calculamos dónde el rayo cruza los planos `min` y `max`
- `t` representa "cuántas veces la dirección del rayo hay que avanzar desde el origen"

**Ejemplo visual (1D):**
```
                rayOrigin=0, rayDir=1 (hacia la derecha)
                ↓
Rayo: ---●===========================→
         t=0    t=2      t=5
                ↑        ↑
                min      max

t0 = (min - rayOrigin) / rayDir = (2 - 0) / 1 = 2
t1 = (max - rayOrigin) / rayDir = (5 - 0) / 1 = 5
```

**Paso 3: Manejar direcciones negativas**
Si el rayo va hacia la izquierda (rayDir=-1), `t0` y `t1` se intercambian. `glm::min` y `glm::max` los reordenan.

**Paso 4: Encontrar intervalo de intersección**
- `tmin` = el punto más lejano de entrada (máximo de los mínimos)
- `tmax` = el punto más cercano de salida (mínimo de los máximos)

**Paso 5: Verificar si hay intersección**
- `tmin > tmax` → El rayo no cruza los tres pares de planos al mismo tiempo → NO HAY INTERSECCIÓN
- `tmax < 0` → La caja está detrás del rayo → NO HAY INTERSECCIÓN

**Paso 6: Calcular distancia**
- Si `tmin > 0`: El rayo empieza fuera de la caja, distancia = tmin (punto de entrada)
- Si `tmin < 0`: El rayo empieza dentro de la caja, distancia = tmax (punto de salida)

---

## Cálculo de AABB Transformado

### ¿Por qué necesitamos transformar el AABB?

El modelo tiene un AABB en su "espacio local" (como si estuviera en el origen 0,0,0). Pero cuando lo movemos, rotamos o escalamos, el AABB también debe transformarse.

```cpp
AABB calculateTransformedAABB(Model* model, const glm::mat4& transform) {
    AABB aabb;

    // Iterar sobre todos los vértices del modelo
    for (const auto& mesh : model->getMeshes()) {
        for (const auto& vertex : mesh.getVertices()) {
            // Transformar cada vértice
            glm::vec4 transformedPos = transform * glm::vec4(vertex.Position, 1.0f);
            // Expandir AABB para incluirlo
            aabb.expand(glm::vec3(transformedPos));
        }
    }

    return aabb;
}
```

**¿Por qué iterar todos los vértices?**

Cuando rotas o escalas un objeto, su AABB puede cambiar de tamaño. No podemos solo transformar el AABB original.

**Ejemplo:**
```
Cubo original (sin rotar):          Cubo rotado 45°:
    +---+                              +
    |   |                             /|\
    |   |                            / | \
    +---+                           +  +  +
    AABB pequeño                    \  |  /
                                     \ | /
                                      \|/
                                       +
                                    AABB más grande
```

**Optimización posible (no implementada):**
En lugar de transformar todos los vértices, se podría transformar los 8 vértices del AABB original. Es más rápido pero solo funciona correctamente para ciertas transformaciones.

---

## Sistema de Arrastre (Drag & Drop)

### Posición Actual del Cuadrúpedo

```cpp
glm::vec3 getCurrentQuadrupedPosition() const {
    float currentAnimTime = m_animator->GetCurrentTime();
    float currentCycleDistance = 0.0f;

    // Calcular distancia del ciclo actual
    if (currentAnimTime <= m_pauseStartTick) {
        float activeProgress = currentAnimTime / m_pauseStartTick;
        currentCycleDistance = activeProgress * m_distancePerCycle;
    } else {
        currentCycleDistance = m_distancePerCycle;
    }

    float autoMoveDistance = m_totalCycles * m_distancePerCycle + currentCycleDistance;
    return m_quadrupedPosition + glm::vec3(0.0f, 0.0f, autoMoveDistance);
}
```

**¿Por qué esta función es necesaria?**

El cuadrúpedo tiene dos componentes de posición:
1. **Posición arrastrable** (`m_quadrupedPosition`) - Dónde el usuario lo movió
2. **Movimiento automático** (`autoMoveDistance`) - Desplazamiento por animación cíclica

La posición visual final es la suma de ambas. Esta función calcula esa posición completa.

**Problema resuelto:**
Sin esta función, el AABB se calculaba solo con `m_quadrupedPosition`, pero el modelo se renderizaba en `m_quadrupedPosition + autoMoveDistance`. ¡El rayo buscaba en el lugar equivocado!

### Manejo de Click (Selección)

```cpp
void handleMouseClick(double mouseX, double mouseY) {
    glm::vec3 rayOrigin = m_camera.getPosition();
    glm::vec3 rayDir = screenToWorldRay(mouseX, mouseY);

    float quadrupedDist = FLT_MAX;

    // Test contra cuadrúpedo (usando posición real con autoMoveDistance)
    glm::vec3 currentQuadrupedPos = getCurrentQuadrupedPosition();
    glm::mat4 quadrupedTransform = glm::translate(glm::mat4(1.0f), currentQuadrupedPos);
    AABB quadrupedAABB = calculateTransformedAABB(m_model.get(), quadrupedTransform);
    bool hitQuadruped = rayAABBIntersection(rayOrigin, rayDir, quadrupedAABB, quadrupedDist);

    // DEBUG: Mostrar información detallada
    std::cout << "\n=== CLICK DEBUG ===" << std::endl;
    std::cout << "Cuadrupedo: " << (hitQuadruped ? "HIT" : "NO HIT");
    if (hitQuadruped) std::cout << " (dist: " << quadrupedDist << ")";
    std::cout << std::endl;

    // Seleccionar solo el cuadrúpedo
    if (hitQuadruped) {
        m_selectedObject = DraggableObject::QUADRUPED;
        m_dragPlaneDistance = quadrupedDist;
        glm::vec3 hitPoint = projectToViewPlane(rayDir, quadrupedDist);
        m_dragOffset = m_quadrupedPosition - hitPoint;
        std::cout << ">>> CUADRUPEDO SELECCIONADO <<<" << std::endl;
        m_isDragging = true;
    } else {
        std::cout << ">>> NADA SELECCIONADO <<<" << std::endl;
    }
    std::cout << "===================\n" << std::endl;
}
```

**Flujo de selección:**

1. **Obtener rayo** → De la cámara hacia el punto del click
2. **Calcular posición real** → Incluir movimiento automático
3. **Crear AABB transformado** → Con la posición actual
4. **Test de intersección** → ¿El rayo toca el AABB?
5. **Si hay HIT:**
   - Marcar objeto como seleccionado
   - Guardar distancia del plano de arrastre
   - Calcular offset entre click y posición base
   - Activar modo arrastre

**¿Qué es `projectToViewPlane()`?**

Proyecta el punto de intersección 3D a un plano paralelo a la vista de la cámara. Esto permite que el arrastre sea intuitivo (el objeto se mueve "en pantalla" en lugar de alejarse/acercarse).

```cpp
glm::vec3 projectToViewPlane(const glm::vec3& rayDir, float distance) {
    glm::vec3 cameraPos = m_camera.getPosition();
    return cameraPos + rayDir * distance;
}
```

**Cálculo del offset:**
```cpp
m_dragOffset = m_quadrupedPosition - hitPoint;
```

Si haces click en la oreja (hitPoint), el offset guarda "cuánto está la oreja del centro". Al arrastrar, sumamos este offset para mantener la posición relativa.

### Actualización de Arrastre

```cpp
void handleMouseDrag(double mouseX, double mouseY) {
    if (!m_isDragging || m_selectedObject == DraggableObject::NONE) return;

    glm::vec3 rayDir = screenToWorldRay(mouseX, mouseY);
    glm::vec3 newPosition = projectToViewPlane(rayDir, m_dragPlaneDistance) + m_dragOffset;

    if (m_selectedObject == DraggableObject::QUADRUPED) {
        m_quadrupedPosition = newPosition;
    } else if (m_selectedObject == DraggableObject::SCENE) {
        m_scenePosition = newPosition;
    }
}
```

**¿Cómo funciona el arrastre continuo?**

1. **Cada frame** que el mouse se mueve (con botón presionado)
2. Calcular nuevo rayo desde la cámara
3. Proyectar a la misma distancia que el click inicial (`m_dragPlaneDistance`)
4. Sumar el offset para mantener la posición relativa del click
5. Actualizar la posición del objeto

**¿Por qué usar la misma `m_dragPlaneDistance`?**

Mantiene el objeto a la misma "profundidad" visual. Si usáramos una nueva distancia cada frame, el objeto podría alejarse/acercarse erráticamente.

### Liberación del Objeto

```cpp
void handleMouseRelease() {
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
```

**Limpieza del estado:**
- Resetear flags de arrastre
- Deseleccionar objeto
- Mostrar posición final (feedback al usuario)

---

## Integración con Mouse Callbacks

### Mouse Movement Callback

```cpp
static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_instance) {
        // Si está arrastrando, actualizar posición del objeto
        if (s_instance->m_isDragging) {
            s_instance->handleMouseDrag(xpos, ypos);
        } else {
            // Mostrar hover info cuando no está arrastrando
            s_instance->handleMouseHover(xpos, ypos);
        }
        // La cámara solo se mueve con click derecho
        s_instance->m_camera.processMouseMovement(xpos, ypos);
    }
}
```

**Lógica de prioridad:**
1. Si está arrastrando → Actualizar arrastre
2. Si no → Mostrar hover info (qué objeto está bajo el cursor)
3. Siempre → Permitir movimiento de cámara (manejado internamente con click derecho)

### Mouse Button Callback

```cpp
static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (s_instance) {
        // Click izquierdo para drag & drop
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);
                s_instance->handleMouseClick(xpos, ypos);
            } else if (action == GLFW_RELEASE) {
                s_instance->handleMouseRelease();
            }
        }
        // Click derecho para cámara (comportamiento normal)
        s_instance->m_camera.processMouseButton(button, action);
    }
}
```

**Separación de botones:**
- **Botón izquierdo**: Drag & drop de objetos
- **Botón derecho**: Control de cámara orbital

Esto permite mover objetos y rotar la cámara independientemente.

---

## Sistema de Hover Feedback

```cpp
void handleMouseHover(double mouseX, double mouseY) {
    glm::vec3 rayOrigin = m_camera.getPosition();
    glm::vec3 rayDir = screenToWorldRay(mouseX, mouseY);

    float quadrupedDist = FLT_MAX;

    // Test contra cuadrúpedo (usando posición real con autoMoveDistance)
    glm::vec3 currentQuadrupedPos = getCurrentQuadrupedPosition();
    glm::mat4 quadrupedTransform = glm::translate(glm::mat4(1.0f), currentQuadrupedPos);
    AABB quadrupedAABB = calculateTransformedAABB(m_model.get(), quadrupedTransform);
    bool hitQuadruped = rayAABBIntersection(rayOrigin, rayDir, quadrupedAABB, quadrupedDist);

    // Mostrar información de hover
    static double lastHoverTime = 0.0;
    double currentTime = glfwGetTime();

    // Actualizar cada 0.5 segundos para no saturar la consola
    if (currentTime - lastHoverTime > 0.5) {
        if (hitQuadruped) {
            std::cout << "\n=== HOVER INFO ===" << std::endl;
            std::cout << "  Cuadrupedo: HIT (distancia: " << quadrupedDist << ")" << std::endl;
            std::cout << "  >> SE SELECCIONARIA: CUADRUPEDO <<" << std::endl;
            std::cout << "==================\n" << std::endl;
        }
        lastHoverTime = currentTime;
    }
}
```

**¿Por qué limitar las actualizaciones a 0.5 segundos?**

El callback de mouse se llama cientos de veces por segundo. Sin límite, la consola se saturaría de mensajes y sería ilegible.

**Técnica de rate limiting:**
```cpp
static double lastHoverTime = 0.0;  // Persiste entre llamadas (static)
double currentTime = glfwGetTime();

if (currentTime - lastHoverTime > 0.5) {
    // Han pasado más de 0.5 segundos desde el último mensaje
    // ... imprimir info ...
    lastHoverTime = currentTime;  // Actualizar timestamp
}
```

---

## Flujo Completo de Interacción

### Diagrama de Flujo

```
[Usuario mueve mouse]
        ↓
[mouseCallback()]
        ↓
    ¿Está arrastrando?
        ↙         ↘
      SÍ          NO
        ↓          ↓
[handleMouseDrag()] [handleMouseHover()]
        ↓          ↓
[Actualizar posición] [Mostrar info]
        ↓
[Renderizar objeto en nueva posición]

[Usuario hace click izquierdo]
        ↓
[mouseButtonCallback()]
        ↓
[handleMouseClick()]
        ↓
[screenToWorldRay()] → Convertir mouse a rayo 3D
        ↓
[getCurrentQuadrupedPosition()] → Posición real
        ↓
[calculateTransformedAABB()] → AABB en posición actual
        ↓
[rayAABBIntersection()] → ¿Rayo toca AABB?
        ↓
    ¿HIT?
        ↙     ↘
      SÍ      NO
        ↓      ↓
[Seleccionar objeto] [Nada]
[m_isDragging = true]
        ↓
[Usuario arrastra mouse]
        ↓
[handleMouseDrag()] → Actualizar posición
        ↓
[Usuario suelta botón]
        ↓
[handleMouseRelease()]
        ↓
[m_isDragging = false]
```

---

## Optimizaciones y Consideraciones

### Cálculo de AABB

**Problema:** `calculateTransformedAABB()` itera todos los vértices del modelo (miles) en cada click y hover.

**Optimización posible:**
```cpp
// Cachear AABB en espacio local (calcularlo una sola vez)
AABB m_modelLocalAABB;  // En constructor del modelo

// Solo transformar las 8 esquinas del AABB
AABB calculateTransformedAABB_Fast(const AABB& localAABB, const glm::mat4& transform) {
    AABB result;
    glm::vec3 corners[8] = {
        glm::vec3(localAABB.min.x, localAABB.min.y, localAABB.min.z),
        glm::vec3(localAABB.max.x, localAABB.min.y, localAABB.min.z),
        // ... otras 6 esquinas
    };

    for (int i = 0; i < 8; i++) {
        glm::vec3 transformed = glm::vec3(transform * glm::vec4(corners[i], 1.0f));
        result.expand(transformed);
    }

    return result;
}
```

**Trade-off:** 8 vértices vs miles. Mucho más rápido, pero solo funciona para transformaciones afines (translate, rotate, scale uniform).

### Frecuencia de Hover

**Configuración actual:** 0.5 segundos

**Ajustes posibles:**
- **Más frecuente (0.1s)**: Feedback más inmediato, más mensajes en consola
- **Menos frecuente (1.0s)**: Consola más limpia, feedback más lento

```cpp
// Cambiar en handleMouseHover()
if (currentTime - lastHoverTime > 0.1) {  // Más frecuente
    // ...
}
```

### Múltiples Objetos

**Actualmente:** Solo detecta el cuadrúpedo (escena comentada)

**Para agregar más objetos:**
```cpp
void handleMouseClick(double mouseX, double mouseY) {
    // ... código existente ...

    float sceneDist = FLT_MAX;
    glm::mat4 sceneTransform = glm::translate(glm::mat4(1.0f), m_scenePosition);
    sceneTransform = glm::scale(sceneTransform, glm::vec3(100.0f));
    AABB sceneAABB = calculateTransformedAABB(m_sceneModel.get(), sceneTransform);
    bool hitScene = rayAABBIntersection(rayOrigin, rayDir, sceneAABB, sceneDist);

    // Seleccionar el más cercano
    if (hitQuadruped && hitScene) {
        if (quadrupedDist < sceneDist) {
            // Seleccionar cuadrúpedo
        } else {
            // Seleccionar escena
        }
    } else if (hitQuadruped) {
        // Solo cuadrúpedo
    } else if (hitScene) {
        // Solo escena
    }
}
```

---

## Debugging

### Mensajes de Consola

**Hover Info:**
```
=== HOVER INFO ===
  Cuadrupedo: HIT (distancia: 8.234)
  >> SE SELECCIONARIA: CUADRUPEDO <<
==================
```

**Click Debug:**
```
=== CLICK DEBUG ===
Cuadrupedo: HIT (dist: 8.234)
>>> CUADRUPEDO SELECCIONADO <<<
===================
```

**Release Info:**
```
Objeto liberado en posicion: (2.5, 0.0, 15.3)
```

### Verificación Visual

**Para visualizar los AABBs (opcional):**
```cpp
void renderAABB(const AABB& aabb) {
    // Crear líneas para las 12 aristas del AABB
    // Similar a renderAxis() pero con las esquinas del AABB
    // Útil para debugging visual
}
```

---

## Problemas Comunes

### Problema: El objeto no se selecciona

**Síntomas:** Click sobre el modelo pero no sale "HIT" en consola

**Causas posibles:**
1. **AABB en posición incorrecta**: Verificar que `getCurrentQuadrupedPosition()` retorna la posición visual
2. **AABB demasiado pequeño**: El modelo puede estar fuera del AABB calculado
3. **Transformaciones incorrectas**: Verificar que las matrices de transformación coinciden con el renderizado

**Solución:**
```cpp
// Agregar debug en handleMouseClick()
std::cout << "Posición visual: " << currentQuadrupedPos.x << ", "
          << currentQuadrupedPos.y << ", " << currentQuadrupedPos.z << std::endl;
std::cout << "AABB min: " << quadrupedAABB.min.x << ", " << quadrupedAABB.min.y << ", " << quadrupedAABB.min.z << std::endl;
std::cout << "AABB max: " << quadrupedAABB.max.x << ", " << quadrupedAABB.max.y << ", " << quadrupedAABB.max.z << std::endl;
```

### Problema: El objeto "salta" al hacer click

**Síntomas:** Al empezar a arrastrar, el objeto se mueve bruscamente

**Causa:** El offset no se está calculando correctamente

**Solución:**
Verificar que `m_dragOffset` se calcula como:
```cpp
m_dragOffset = m_quadrupedPosition - hitPoint;
```

Y se aplica como:
```cpp
newPosition = projectToViewPlane(...) + m_dragOffset;
```

### Problema: El arrastre no sigue el mouse

**Síntomas:** Mueves el mouse pero el objeto no se mueve proporcionalmente

**Causa:** La distancia del plano de arrastre cambia entre frames

**Solución:**
Asegurar que `m_dragPlaneDistance` se establece una vez al hacer click y NO se modifica durante el arrastre:
```cpp
// EN handleMouseClick():
m_dragPlaneDistance = quadrupedDist;  // ✅ Solo una vez

// EN handleMouseDrag():
// USAR la distancia guardada, NO calcular una nueva
glm::vec3 newPosition = projectToViewPlane(rayDir, m_dragPlaneDistance) + m_dragOffset;
```

---

## Mejoras Futuras

### 1. Selección Más Precisa (Mesh-Level)

En lugar de usar AABB (caja), probar intersección con los triángulos reales del modelo.

**Ventajas:** Selección perfecta
**Desventajas:** Mucho más lento (miles de triángulos)

### 2. Highlight Visual

Cambiar el color del objeto cuando está seleccionado.

```cpp
// En el fragment shader
if (isSelected) {
    FragColor = mix(FragColor, vec4(1.0, 1.0, 0.0, 1.0), 0.3);  // Tinte amarillo
}
```

### 3. Restricción de Movimiento

Limitar el arrastre a ciertos ejes o planos.

```cpp
// Solo permitir movimiento en plano XZ (horizontal)
newPosition.y = originalY;  // Mantener altura constante
```

### 4. Snapping a Grid

Hacer que el objeto se "pegue" a una cuadrícula.

```cpp
// Snapping cada 1.0 unidades
newPosition.x = round(newPosition.x);
newPosition.z = round(newPosition.z);
```

---

**Relacionado:**
- [Sistema de Cámara](CAMERA.md) - Para entender las transformaciones de vista
- [Renderizado](RENDERING.md) - Pipeline de renderizado
- [Lógica Global](LOGICA_GLOBAL.md) - Flujo general del programa

**Próximo:** [Lógica Global del Proyecto](LOGICA_GLOBAL.md)
