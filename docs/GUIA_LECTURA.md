# Guía de Lectura: ¿Por Dónde Empiezo?

## 🎯 Esta guía es para ti si...

- Te sientes perdido leyendo el código
- No sabes por dónde empezar
- Quieres entender paso a paso cómo funciona
- Prefieres aprender HACIENDO en lugar de solo leyendo

---

## 📚 Plan de Aprendizaje (5 Días, 1-2 horas/día)

### DÍA 1: El Inicio - ¿Qué pasa cuando ejecuto el programa?

**📖 Lee SOLO estos archivos (en orden):**

1. **main.cpp** (15 líneas) - El punto de entrada
   ```cpp
   // Lee líneas 1-15
   // Pregunta: ¿Qué hace el try-catch?
   ```

2. **Application.h - Constructor** (líneas 85-110)
   ```cpp
   // Busca: Application()
   // Pregunta: ¿Qué valores se inicializan?
   ```

3. **Application.h - run()** (líneas 121-130)
   ```cpp
   // Busca: void run()
   // Pregunta: ¿Qué 3 cosas hace run()?
   ```

**✅ Ejercicio práctico:**

Agrega un mensaje en cada paso:

```cpp
// En main.cpp
int main() {
    std::cout << "=== INICIO DEL PROGRAMA ===" << std::endl;
    try {
        Application app;
        std::cout << "=== APPLICATION CREADO ===" << std::endl;
        app.run();
    }
    // ...
}

// En Application.h - run()
void run() {
    std::cout << "1. Iniciando initialize()..." << std::endl;
    if (!initialize()) return;

    std::cout << "2. Iniciando mainLoop()..." << std::endl;
    mainLoop();

    std::cout << "3. Iniciando cleanup()..." << std::endl;
    cleanup();
}
```

**Compila y ejecuta.** ¿Ves los mensajes en orden?

**🎯 Al final del día debes poder responder:**
- ¿Cuál es el PRIMER código que se ejecuta?
- ¿En qué orden se llaman initialize, mainLoop, cleanup?
- ¿Qué pasa si initialize() falla?

---

### DÍA 2: La Inicialización - ¿Cómo se carga todo?

**📖 Lee SOLO:**

1. **Application.h - initialize()** (líneas 112-120)
   ```cpp
   // ¿Cuántas funciones llama?
   // ¿Qué pasa si alguna falla?
   ```

2. **Application.h - initWindow()** (líneas 133-155)
   ```cpp
   // ¿Qué es GLFW?
   // ¿Qué hace glfwCreateWindow?
   ```

3. **Application.h - loadResources()** (líneas 167-196)
   ```cpp
   // ¿Qué recursos se cargan?
   // ¿En qué orden?
   ```

**✅ Experimento 1: Cambiar el título de la ventana**

```cpp
// En initWindow(), línea ~145
m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
    "MI PRIMER CAMBIO!", nullptr, nullptr);  // <-- Cambia esto
```

Compila, ejecuta. ¿Cambió el título?

**✅ Experimento 2: Cambiar el color de fondo**

```cpp
// En render(), línea ~501
glClearColor(1.0f, 0.0f, 0.0f, 1.0f);  // Rojo en lugar de celeste
```

Compila, ejecuta. ¿La pantalla es roja ahora?

**✅ Experimento 3: Comentar la carga del cuadrúpedo**

```cpp
// En loadResources(), línea ~175
// m_model = std::make_unique<Model>("assets/models/quadruped.glb");
```

Compila, ejecuta. ¿Qué error da? ¿En qué parte crashea?

**Descomenta antes de continuar.**

**🎯 Al final del día debes poder responder:**
- ¿Qué se carga primero: shaders, modelos o animación?
- ¿Qué pasa si un shader falla al cargar?
- ¿Para qué sirve GLFW?

---

### DÍA 3: El Loop Principal - ¿Qué pasa 60 veces por segundo?

**📖 Lee SOLO:**

1. **Application.h - mainLoop()** (líneas 427-441)
   ```cpp
   // ¿Cuántas veces se ejecuta este while?
   // ¿Cuándo termina?
   ```

2. **Application.h - processInput()** (líneas 454-484)
   ```cpp
   // ¿Qué teclas detecta?
   // ¿Qué hace cada tecla?
   ```

3. **Application.h - update()** (líneas 485-499)
   ```cpp
   // ¿Qué se actualiza aquí?
   // ¿Qué pasa si la animación está pausada?
   ```

**✅ Experimento 4: Contar los frames**

```cpp
// En mainLoop(), ANTES del while
int frameCount = 0;

// DENTRO del while, al final
frameCount++;
if (frameCount % 60 == 0) {  // Cada 60 frames (1 segundo a 60 FPS)
    std::cout << "Han pasado " << (frameCount / 60) << " segundos" << std::endl;
}
```

Ejecuta. ¿Cada cuánto sale el mensaje?

**✅ Experimento 5: Ver el delta time**

```cpp
// En mainLoop(), después de updateDeltaTime()
if (frameCount % 60 == 0) {  // Cada segundo
    std::cout << "Delta time: " << m_deltaTime << " segundos" << std::endl;
    std::cout << "FPS estimados: " << (1.0f / m_deltaTime) << std::endl;
}
```

Ejecuta. ¿Cuántos FPS tiene tu PC?

**✅ Experimento 6: Pausar la animación**

Ejecuta el programa normalmente.
- Presiona ESPACIO → ¿Qué mensaje sale?
- ¿El cuadrúpedo se detiene?
- ¿La cámara sigue funcionando?
- Presiona ESPACIO de nuevo → ¿Se reanuda?

**🎯 Al final del día debes poder responder:**
- ¿Cuántas veces por segundo se ejecuta el loop?
- ¿Qué es delta time y para qué sirve?
- ¿En qué orden se ejecutan processInput, update, render?

---

### DÍA 4: El Renderizado - ¿Cómo se dibuja en pantalla?

**📖 Lee SOLO:**

1. **Application.h - render()** (líneas 500-512)
   ```cpp
   // ¿Qué se renderiza primero?
   // ¿Por qué se llama glClear?
   ```

2. **Application.h - renderModel()** (líneas 521-548)
   ```cpp
   // ¿Qué son las matrices projection, view, model?
   // ¿Qué hace m_model->Draw()?
   ```

3. **Shader.h** (skim, solo mira los nombres de funciones)
   ```cpp
   // ¿Qué hace use()?
   // ¿Qué hace setMat4()?
   ```

**✅ Experimento 7: Mover el cuadrúpedo**

```cpp
// En renderModel(), línea ~537
glm::mat4 modelMat = glm::mat4(1.0f);
// Cambia la posición:
modelMat = glm::translate(modelMat, glm::vec3(3.0f, 0.0f, 0.0f));  // Mover a la derecha
```

Ejecuta. ¿El cuadrúpedo se movió?

**✅ Experimento 8: Agrandar el cuadrúpedo**

```cpp
// Después del translate:
modelMat = glm::scale(modelMat, glm::vec3(2.0f, 2.0f, 2.0f));  // 2x más grande
```

Ejecuta. ¿Se ve más grande?

**✅ Experimento 9: Cambiar el color de la luz**

```cpp
// En renderModel(), línea ~543
m_shader->setVec3("lightColor", glm::vec3(0.0f, 1.0f, 0.0f));  // Verde
```

Ejecuta. ¿El cuadrúpedo se ve verde?

**Revierte todos los cambios antes de continuar.**

**🎯 Al final del día debes poder responder:**
- ¿Qué hace glClearColor?
- ¿Qué son las matrices Model, View, Projection?
- ¿Para qué sirve un shader?

---

### DÍA 5: La Animación - ¿Cómo se mueven los huesos?

**📖 Lee SOLO:**

1. **Animator.h** (líneas 1-50)
   ```cpp
   // ¿Qué hace UpdateAnimation()?
   // ¿Qué devuelve GetFinalBoneMatrices()?
   ```

2. **Animation.h** (líneas 1-50)
   ```cpp
   // ¿Qué información tiene una animación?
   // ¿Qué es m_Duration?
   ```

3. **Application.h - renderModel()** (líneas 523-528)
   ```cpp
   // ¿Cómo se envían las matrices de huesos al shader?
   // ¿Cuántas matrices hay?
   ```

**✅ Experimento 10: Congelar en un frame específico**

```cpp
// En update(), línea ~486
if (!m_animationPaused) {
    // Congela en el frame 50
    if (m_animator->GetCurrentTime() > 50.0f) {
        m_animationPaused = true;
        std::cout << "CONGELADO EN FRAME 50" << std::endl;
    }
    m_animator->UpdateAnimation(m_deltaTime);
}
```

Ejecuta. ¿Se congela solo?

**✅ Experimento 11: Animación al doble de velocidad**

```cpp
// En update(), línea ~488
m_animator->UpdateAnimation(m_deltaTime * 2.0f);  // 2x velocidad
```

Ejecuta. ¿Se mueve más rápido?

**✅ Experimento 12: Ver información de la animación**

```cpp
// En update(), después de UpdateAnimation
static int frameCounter = 0;
frameCounter++;
if (frameCounter % 120 == 0) {  // Cada 2 segundos
    std::cout << "Tiempo actual: " << m_animator->GetCurrentTime() << std::endl;
    std::cout << "Ciclos completos: " << m_totalCycles << std::endl;
}
```

Ejecuta. ¿Cuántos ciclos completa en 10 segundos?

**🎯 Al final del día debes poder responder:**
- ¿Qué es un "hueso" (bone)?
- ¿Cómo se mueve un modelo con huesos?
- ¿Qué pasa cuando la animación termina?

---

## 🎓 Evaluación Final: ¿Lo entendiste?

### Desafío 1: Cambiar el comportamiento

**Tarea:** Haz que el cuadrúpedo NO se mueva en el eje Z (que se quede en su lugar).

<details>
<summary>💡 Pista</summary>

Busca donde se calcula `autoMoveDistance` y coméntalo.
</details>

<details>
<summary>✅ Solución</summary>

```cpp
// En render(), línea ~506
// float autoMoveDistance = m_totalCycles * m_distancePerCycle + currentCycleDistance;
float autoMoveDistance = 0.0f;  // No mover
```
</details>

### Desafío 2: Agregar un nuevo control

**Tarea:** Haz que la tecla P también pause la animación (además de ESPACIO).

<details>
<summary>💡 Pista</summary>

Busca donde se maneja GLFW_KEY_SPACE y agrega GLFW_KEY_P.
</details>

<details>
<summary>✅ Solución</summary>

```cpp
// En processInput(), línea ~458
if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS ||
    glfwGetKey(m_window, GLFW_KEY_P) == GLFW_PRESS) {
    if (!m_spaceKeyPressed) {
        // ...
    }
}
```
</details>

### Desafío 3: Debugging

**Tarea:** El cuadrúpedo no aparece en pantalla. ¿Qué puede estar mal? (Lista 3 posibles causas)

<details>
<summary>✅ Respuestas posibles</summary>

1. El modelo no se cargó (error en loadResources)
2. La cámara está muy lejos o mirando en otra dirección
3. El shader no se compiló correctamente
4. El modelo está en una posición fuera de la vista
5. glClearColor del mismo color que el modelo
</details>

---

## 📊 Mapa Mental del Código

```
main()
  ↓
Application()  [Constructor - Inicializa variables]
  ↓
run()
  ↓
  ├─→ initialize()
  │     ├─→ initWindow()      [Crea ventana GLFW]
  │     ├─→ initOpenGL()      [Configura OpenGL]
  │     └─→ loadResources()   [Carga shaders, modelos, animación]
  │
  ├─→ mainLoop()  [Loop infinito, 60 FPS]
  │     ↓
  │     while (!cerrar) {
  │       ├─→ updateDeltaTime()    [Calcular tiempo entre frames]
  │       ├─→ processInput()       [Detectar teclas/mouse]
  │       ├─→ update()             [Actualizar animación]
  │       ├─→ render()             [Dibujar todo]
  │       │     ├─→ glClear()            [Limpiar pantalla]
  │       │     ├─→ renderModel()        [Dibujar cuadrúpedo]
  │       │     │     ├─→ Enviar matrices de huesos
  │       │     │     ├─→ Enviar matrices MVP
  │       │     │     └─→ model->Draw()  [GPU dibuja]
  │       │     └─→ renderAxis()         [Dibujar ejes XYZ]
  │       └─→ glfwSwapBuffers()    [Mostrar frame]
  │     }
  │
  └─→ cleanup()  [Liberar recursos]
```

---

## 🔑 Conceptos Clave (Lo Mínimo que Debes Saber)

### 1. El Loop de Juego

```
INICIALIZAR → [PROCESAR INPUT → ACTUALIZAR → RENDERIZAR] × 60/seg → LIMPIAR
```

### 2. Las 3 Matrices (MVP)

```
Model      → Dónde está el objeto en el mundo
View       → Dónde está la cámara
Projection → Cómo se proyecta 3D → 2D

Vértice final = Projection × View × Model × Vértice original
```

### 3. Skeletal Animation

```
Keyframes → Interpolación → Matrices de huesos → Shader → Vértices deformados
```

### 4. Delta Time

```
Si FPS varía, el movimiento debe ser independiente:
position += speed * deltaTime  ✅
position += speed              ❌ (varía con FPS)
```

---

## 📝 Checklist de Comprensión

Marca lo que YA entiendes:

**Básico:**
- [ ] Sé dónde empieza el programa (main.cpp)
- [ ] Entiendo qué hace el constructor de Application
- [ ] Sé qué es el loop principal
- [ ] Puedo cambiar el título de la ventana
- [ ] Puedo cambiar el color de fondo

**Intermedio:**
- [ ] Entiendo qué es delta time
- [ ] Sé cómo se procesa el input del teclado
- [ ] Puedo pausar la animación
- [ ] Entiendo qué hace render()
- [ ] Puedo cambiar la posición del modelo

**Avanzado:**
- [ ] Entiendo las matrices Model, View, Projection
- [ ] Sé cómo funcionan los shaders
- [ ] Entiendo cómo funciona la animación esquelética
- [ ] Puedo agregar un nuevo control de teclado
- [ ] Entiendo el sistema de ray casting

---

## ❓ Preguntas Frecuentes

**P: ¿Tengo que leer TODO el código?**
R: NO. Empieza con los ejercicios de esta guía. Lee solo lo necesario.

**P: No entiendo las matrices y matemáticas 3D**
R: No pasa nada. Puedes usar el código sin entender toda la matemática. Con el tiempo lo entenderás.

**P: ¿Cuánto tiempo me tomará entender todo?**
R: Con 1-2 horas al día, en 1 semana entenderás lo esencial. Para dominar todo, 2-3 semanas.

**P: Me da error al compilar después de hacer cambios**
R: Usa Git para revertir: `git checkout Application.h`

**P: ¿Qué hago si me quedo atascado?**
R:
1. Revierte tus cambios
2. Vuelve a leer la sección
3. Prueba el experimento de nuevo
4. Compara con el código original

---

## 🎯 Próximos Pasos

**Después de completar esta guía:**

1. **Lee la documentación técnica:** Ahora que entiendes el flujo básico, las docs serán más claras
2. **Experimenta:** Agrega tus propias features
3. **Refactoriza (opcional):** Si quieres practicar POO, extrae el sistema de interactividad

**Ideas de proyectos para practicar:**
- Agregar una segunda animación
- Cambiar el modelo 3D
- Agregar más luces
- Implementar sombras
- Agregar una UI con texto

---

## 🆕 Bonus: Explorando la Versión Refactorizada (POO)

Si ya completaste los 5 días y quieres ver **código profesional**, el proyecto tiene una **versión refactorizada** con buenas prácticas de Programación Orientada a Objetos.

### Diferencia Rápida

| Aspecto | Versión Original | Versión Refactorizada |
|---------|------------------|----------------------|
| Archivos | 1 archivo grande (Application.h) | Múltiples archivos organizados |
| Líneas | ~2044 líneas | ~400 líneas + managers |
| Variables | 20+ booleanos de teclas | InputState centralizado |
| Objetos | Código específico | Clases genéricas reutilizables |

### Guía de Lectura POO (3 días adicionales)

#### DÍA 6: Las Interfaces

**📖 Lee estos archivos:**

1. **src/interfaces/IRenderable.h**
   ```cpp
   // ¿Qué método deben implementar los objetos dibujables?
   ```

2. **src/interfaces/ITransformable.h**
   ```cpp
   // ¿Qué propiedades tienen los objetos transformables?
   ```

3. **src/interfaces/IUpdatable.h**
   ```cpp
   // ¿Cuándo se llama a update()?
   ```

**🎯 Concepto clave:** Una interfaz es un "contrato" - dice QUÉ puede hacer algo, pero no CÓMO.

#### DÍA 7: Los Managers

**📖 Lee estos archivos:**

1. **src/managers/SceneManager.h**
   ```cpp
   // ¿Cómo se agregan objetos a la escena?
   // ¿Qué hace updateAll()?
   ```

2. **src/managers/RenderManager.h**
   ```cpp
   // ¿Qué objetos dibuja?
   // ¿Cómo sabe qué shader usar?
   ```

3. **src/managers/InputManager.h**
   ```cpp
   // ¿Cómo maneja el teclado?
   // ¿Qué es un callback?
   ```

**🎯 Concepto clave:** Cada manager tiene UNA responsabilidad (Principio de Responsabilidad Única).

#### DÍA 8: Los Objetos de Escena

**📖 Lee:**

1. **src/core/SceneObject.h**
   ```cpp
   // ¿Qué propiedades tiene todo objeto de escena?
   // ¿Qué diferencia hay entre AnimatedSceneObject y KeyframeSceneObject?
   ```

**🎯 Concepto clave:** Herencia - AnimatedSceneObject "es un" SceneObject con animación.

### Comparación Visual

```
VERSIÓN ORIGINAL:
┌──────────────────────────────────────────┐
│              Application.h               │
│   (Todo mezclado: input + render + etc)  │
└──────────────────────────────────────────┘

VERSIÓN REFACTORIZADA:
┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ InputManager │  │ SceneManager │  │RenderManager │
│  (solo input)│  │(solo escena) │  │ (solo dibujo)│
└──────────────┘  └──────────────┘  └──────────────┘
         ↓               ↓               ↓
         └───────────────┼───────────────┘
                         ↓
              ┌─────────────────────┐
              │ApplicationRefactored│
              │   (coordina todo)   │
              └─────────────────────┘
```

### Para más detalles

Lee la documentación completa en **[POO_REFACTORIZACION.md](POO_REFACTORIZACION.md)**

---

**¿Sigues perdido?** Empieza por el DÍA 1 y hazlo TODO, incluyendo los experimentos. No sigas al DÍA 2 hasta que puedas responder las preguntas del 🎯.

**Recuerda:** Aprender a programar es 20% leer y 80% HACER. ¡Haz los experimentos!
