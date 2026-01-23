# Guía para Principiantes

Esta guía está diseñada para personas que están iniciando en programación o no tienen experiencia con gráficos 3D. Aquí explicaremos todos los conceptos de forma simple y clara.

## ¿Qué es este proyecto?

Imagina que este proyecto es como un videojuego simple que muestra:
1. **Un cuadrúpedo animado** (un animal de 4 patas que se mueve)
2. **Una escena 3D** (el entorno/fondo donde está el animal)

Todo esto se renderiza (dibuja) en una ventana en tu computadora, y puedes mover la cámara para ver desde diferentes ángulos.

## Conceptos Básicos Explicados

### ¿Qué es C++?

**C++** es un lenguaje de programación (como inglés o español, pero para hablar con computadoras). Es muy usado para:
- Videojuegos
- Aplicaciones gráficas
- Software que necesita ser muy rápido

**Analogía:** Si HTML es como escribir una carta, C++ es como construir un motor de coche. Es más complejo pero mucho más potente.

### ¿Qué es OpenGL?

**OpenGL** es una herramienta (biblioteca) que permite dibujar cosas en 3D en la pantalla de tu computadora.

**¿Cómo funciona?**
1. Le dices a OpenGL las coordenadas de puntos en el espacio (X, Y, Z)
2. OpenGL conecta esos puntos formando triángulos
3. Los triángulos forman objetos 3D
4. OpenGL pinta esos objetos con colores y texturas

**Analogía:** OpenGL es como un pintor profesional. Tú le das instrucciones ("dibuja un cubo aquí, una esfera allá") y él hace todo el trabajo de pintar en la pantalla.

### ¿Qué es un Modelo 3D?

Un **modelo 3D** es un objeto tridimensional (como un personaje, un árbol, un edificio) hecho en programas como Blender.

**Estructura de un modelo:**
```
Modelo 3D
├── Geometría (forma)
│   ├── Vértices (puntos en el espacio)
│   ├── Aristas (líneas que conectan puntos)
│   └── Caras (triángulos formados por las líneas)
├── Texturas (imágenes que se pegan al modelo)
└── Animaciones (cómo se mueve el modelo)
```

**Archivo GLB:** Es como un archivo ZIP que contiene todo lo anterior en un solo archivo.

### ¿Qué es una Animación Esquelética?

Es cómo los videojuegos hacen que los personajes se muevan de forma realista.

**¿Cómo funciona?**
1. **Huesos (Bones):** El modelo tiene un esqueleto virtual invisible
2. **Vértices (puntos del modelo):** Cada punto está "pegado" a uno o más huesos
3. **Animación:** Cuando mueves los huesos, los puntos se mueven con ellos

**Analogía:** Es como un títere de marioneta:
- Los hilos son los huesos
- La tela del títere son los vértices
- Cuando mueves los hilos, la tela se mueve automáticamente

### ¿Qué es un Shader?

Un **shader** es un mini-programa que decide cómo se ve cada pixel en la pantalla.

**Tipos de shaders en este proyecto:**

1. **Vertex Shader:** Decide dónde va cada punto del modelo
2. **Fragment Shader:** Decide de qué color pintar cada pixel

**Analogía:**
- El Vertex Shader es el arquitecto que decide dónde va cada ladrillo
- El Fragment Shader es el pintor que decide de qué color pintar cada ladrillo

### ¿Qué es una Cámara Virtual?

La **cámara** determina desde dónde ves la escena 3D.

**Propiedades de la cámara:**
- **Posición:** Dónde está ubicada en el espacio
- **Objetivo:** Hacia dónde está mirando
- **Distancia:** Qué tan cerca o lejos está del objetivo
- **Ángulo:** Desde qué altura/dirección mira

**Analogía:** Es como una cámara de video real. Puedes:
- Acercarte (zoom in)
- Alejarte (zoom out)
- Girar alrededor del objeto
- Mover el punto donde estás mirando

### ¿Qué es Iluminación (Lighting)?

La **iluminación** hace que los objetos se vean realistas, con partes claras y oscuras.

**Componentes de la luz:**

1. **Luz Ambiental (Ambient):** Luz general que ilumina todo un poco
   - Como la luz del día en un cuarto

2. **Luz Difusa (Diffuse):** Luz que viene de una fuente específica
   - Como una lámpara o el sol

3. **Luz Especular (Specular):** Brillo en superficies brillantes/metálicas
   - Como el reflejo del sol en un espejo

**Analogía de cocina:**
- Ambient = luz del techo (ilumina todo)
- Diffuse = luz enfocada sobre la estufa
- Specular = brillo en una olla metálica limpia

## Cómo Funciona Este Proyecto (Simplificado)

### Paso a Paso de lo que Sucede

```
1. INICIO DEL PROGRAMA
   ↓
2. CREAR VENTANA
   - Se abre una ventana vacía de 1280x720 pixels
   ↓
3. CARGAR MODELOS 3D
   - Lee quadruped.glb (animal animado)
   - Lee scena.glb (escena estática)
   ↓
4. COMPILAR SHADERS
   - Carga los programas que dibujarán los objetos
   ↓
5. BUCLE PRINCIPAL (se repite 60 veces por segundo)
   │
   ├─→ 5a. LIMPIAR PANTALLA
   │     (Pinta todo de color celeste)
   │
   ├─→ 5b. ACTUALIZAR ANIMACIÓN
   │     (Mueve los huesos del cuadrúpedo)
   │
   ├─→ 5c. PROCESAR INPUTS
   │     (¿El usuario movió la cámara? ¿Presionó teclas?)
   │
   ├─→ 5d. DIBUJAR ESCENA
   │     (Dibuja el modelo estático con rotación)
   │
   ├─→ 5e. DIBUJAR CUADRÚPEDO
   │     (Dibuja el animal animado)
   │
   └─→ 5f. MOSTRAR EN PANTALLA
         (Lo que dibujamos se hace visible)
   │
   ↓ (Volver al paso 5)

6. CERRAR PROGRAMA (cuando presionas ESC)
   - Libera memoria
   - Cierra ventana
```

### Arquitectura del Proyecto

El proyecto tiene **dos versiones** de la arquitectura:

#### Versión Original (Application.h)
Versión más simple, todo en un solo archivo:

```
┌─────────────────────────────────────────────┐
│           APLICACIÓN PRINCIPAL              │
│            (Application.h)                  │
└──────────┬──────────────────────┬───────────┘
           │                      │
    ┌──────▼──────┐        ┌─────▼──────┐
    │   VENTANA   │        │   CÁMARA   │
    │   (GLFW)    │        │  (Camera)  │
    └──────┬──────┘        └─────┬──────┘
           │                     │
           ▼                     ▼
    ┌─────────────────────────────────┐
    │       RENDERIZADOR              │
    │        (OpenGL)                 │
    └──┬──────────────────┬───────────┘
       │                  │
┌──────▼──────┐    ┌──────▼──────────┐
│   SHADERS   │    │    MODELOS      │
│  (.vert/.frag) │    │  (.glb files)   │
└─────────────┘    └─────────────────┘
```

#### Versión Refactorizada POO (ApplicationRefactored.h)
Versión profesional con buenas prácticas de programación orientada a objetos:

```
┌─────────────────────────────────────────────────────────┐
│              APLICACIÓN REFACTORIZADA                    │
│            (ApplicationRefactored.h)                     │
│   "El director que coordina a todos los managers"       │
└────────┬───────────────┬───────────────┬────────────────┘
         │               │               │
   ┌─────▼─────┐   ┌─────▼─────┐   ┌─────▼─────┐
   │  SCENE    │   │  RENDER   │   │  INPUT    │
   │  MANAGER  │   │  MANAGER  │   │  MANAGER  │
   │"Organiza  │   │"Dibuja    │   │"Escucha   │
   │ objetos"  │   │ todo"     │   │ teclado"  │
   └─────┬─────┘   └─────┬─────┘   └─────┬─────┘
         │               │               │
         ▼               ▼               ▼
   ┌───────────────────────────────────────┐
   │           OBJETOS DE ESCENA            │
   │   (Implementan interfaces comunes)     │
   │                                        │
   │  ┌────────────┐  ┌─────────────────┐  │
   │  │ Animated   │  │   Keyframe      │  │
   │  │ SceneObject│  │   SceneObject   │  │
   │  │ (animado)  │  │ (transf. manual)│  │
   │  └────────────┘  └─────────────────┘  │
   └───────────────────────────────────────┘
```

**¿Por qué dos versiones?**
- **Original:** Más fácil de leer si estás aprendiendo (todo junto)
- **Refactorizada:** Mejor para proyectos grandes y profesionales (código organizado)

**Conceptos POO usados (explicación simple):**

| Concepto | Qué significa | Ejemplo en el proyecto |
|----------|---------------|------------------------|
| **Interfaz** | "Contrato" que dice qué puede hacer algo | IRenderable = "puedo ser dibujado" |
| **Herencia** | Un objeto hereda características de otro | AnimatedSceneObject hereda de SceneObject |
| **Polimorfismo** | Tratar diferentes objetos de la misma forma | Todos los objetos usan render() igual |
| **Encapsulamiento** | Esconder detalles internos | InputManager esconde cómo procesa teclas |

## Glosario de Términos Técnicos

### A-F

- **API (Application Programming Interface):** Conjunto de herramientas para programar algo específico
- **ASSIMP:** Biblioteca que lee archivos 3D (GLB, OBJ, FBX, etc.)
- **Asset:** Recurso del proyecto (modelos, texturas, sonidos)
- **Bone (Hueso):** Parte del esqueleto virtual de un modelo animado
- **Build (Compilación):** Convertir código C++ a un programa ejecutable (.exe)
- **CMake:** Herramienta que configura cómo compilar el proyecto
- **Core Profile:** Versión moderna de OpenGL (sin funciones antiguas)
- **Debug:** Modo de compilación para encontrar errores (más lento)
- **Fragment:** Un pixel que se va a pintar en la pantalla

### G-M

- **GLAD:** Biblioteca que carga funciones de OpenGL
- **GLFW:** Biblioteca para crear ventanas y manejar inputs
- **GLM:** Biblioteca de matemáticas 3D (vectores, matrices)
- **GLB:** Formato de archivo 3D (versión binaria de GLTF)
- **GPU (Graphics Processing Unit):** Tarjeta gráfica de la computadora
- **Header File (.h):** Archivo que declara funciones y clases
- **Implementation File (.cpp):** Archivo que define cómo funcionan las funciones
- **Keyframe:** Posición de una animación en un momento específico
- **Linking:** Proceso de conectar todas las bibliotecas al compilar
- **Matrix (Matriz):** Tabla de números usada para transformaciones 3D
- **Mesh (Malla):** Conjunto de triángulos que forman un objeto 3D

### N-Z

- **Near/Far Plane:** Límites de qué tan cerca/lejos puede ver la cámara
- **OpenGL Context:** Conexión entre tu programa y la tarjeta gráfica
- **Pipeline:** Secuencia de pasos para dibujar algo en pantalla
- **Projection:** Cómo se convierte 3D a 2D para mostrarlo en pantalla
- **Release:** Modo de compilación optimizado (rápido, para distribución)
- **Render:** Dibujar/pintar objetos 3D en la pantalla
- **Shader Program:** Programa que corre en la GPU para dibujar
- **Skinning:** Proceso de mover vértices según los huesos
- **Source File:** Archivo de código (.cpp, .h)
- **STB Image:** Biblioteca para cargar imágenes (PNG, JPG)
- **Texture:** Imagen que se aplica sobre un modelo 3D
- **Transform:** Mover, rotar o escalar un objeto
- **Uniform:** Variable que pasas del programa C++ al shader
- **Vertex:** Un punto en el espacio 3D (X, Y, Z)
- **Viewport:** Área de la ventana donde se dibuja

## Estructura de Archivos Explicada

```
Cuadrupedo_IIIU/
│
├── src/                          # CÓDIGO FUENTE
│   ├── main.cpp                 # Donde empieza el programa (versión original)
│   ├── main_refactored.cpp      # Punto de entrada (versión POO)
│   ├── Application.h            # Lógica principal original
│   ├── ApplicationRefactored.h  # Lógica principal POO
│   ├── Camera.h                 # Sistema de cámara
│   ├── Shader.h                 # Carga y maneja shaders
│   ├── Model.h/cpp              # Carga modelos 3D
│   ├── Mesh.h/cpp               # Representa una malla 3D
│   ├── Animation.h              # Datos de animación
│   ├── Animator.h               # Controla animaciones
│   ├── Bone.h                   # Estructura de huesos
│   │
│   ├── interfaces/              # CONTRATOS POO (qué pueden hacer los objetos)
│   │   ├── IRenderable.h        # "Puedo ser dibujado"
│   │   ├── ITransformable.h     # "Puedo moverme/rotarme"
│   │   └── IUpdatable.h         # "Puedo actualizarme cada frame"
│   │
│   ├── core/                    # CLASES BASE (bloques fundamentales)
│   │   ├── SceneObject.h        # Clase base de objetos en escena
│   │   ├── TransformTypes.h     # Tipos de transformación
│   │   ├── Keyframe.h           # Animación por keyframes
│   │   ├── InputState.h         # Estado del teclado/mouse
│   │   ├── SelectionManager.h   # Gestión de selección
│   │   └── TransformController.h # Aplicar transformaciones
│   │
│   └── managers/                # MANAGERS (organizadores)
│       ├── SceneManager.h       # Organiza objetos de la escena
│       ├── RenderManager.h      # Maneja el dibujado
│       └── InputManager.h       # Procesa teclado y mouse
│
├── shaders/                      # PROGRAMAS PARA LA GPU
│   ├── skeletal_animation.vert  # Shader de vértices animados
│   ├── skeletal_animation.frag  # Shader de colores animados
│   ├── static.vert              # Shader de vértices estáticos
│   ├── static.frag              # Shader de colores estáticos
│   ├── axis.vert                # Shader para ejes X,Y,Z
│   └── axis.frag                # Shader de colores de ejes
│
├── assets/                       # RECURSOS
│   └── models/                  # Modelos 3D
│       ├── quadruped.glb        # Animal animado
│       └── scena.glb            # Escena estática
│
├── external/                     # BIBLIOTECAS EXTERNAS
│   ├── glfw/                    # Ventanas e input
│   ├── GLAD/                    # Cargador de OpenGL
│   ├── glm/                     # Matemáticas 3D
│   ├── Assimp/                  # Importador de modelos
│   └── stb/                     # Cargador de imágenes
│
├── docs/                         # DOCUMENTACIÓN
│   ├── README.md                # Índice de documentación
│   ├── GUIA_PRINCIPIANTES.md    # Este archivo
│   ├── POO_REFACTORIZACION.md   # ⭐ Guía de la nueva arquitectura POO
│   └── ...                      # Otros documentos
│
├── build/                        # ARCHIVOS DE COMPILACIÓN
│   ├── Release/                 # Ejecutable optimizado
│   │   ├── Cuadrupedo_IIIU.exe  # Programa final
│   │   └── assets/              # Copia de recursos
│   └── Debug/                   # Ejecutable para debugging
│
└── CMakeLists.txt               # CONFIGURACIÓN DE COMPILACIÓN
```

## ¿Qué Hace Cada Biblioteca Externa?

### GLFW - "El Constructor de Ventanas"
- **Qué hace:** Crea la ventana donde se muestra todo
- **Funciones principales:**
  - Abrir/cerrar ventana
  - Detectar teclas presionadas
  - Detectar movimiento del mouse
  - Detectar scroll del mouse

### GLAD - "El Traductor"
- **Qué hace:** Permite que tu programa use funciones de OpenGL
- **Por qué es necesario:** Cada tarjeta gráfica tiene diferentes funciones de OpenGL. GLAD encuentra las correctas para tu GPU.

### GLM - "La Calculadora 3D"
- **Qué hace:** Proporciona matemáticas para 3D
- **Incluye:**
  - Vectores (X, Y, Z)
  - Matrices (para transformaciones)
  - Funciones (rotar, escalar, trasladar)

### ASSIMP - "El Importador Universal"
- **Qué hace:** Lee archivos 3D de muchos formatos
- **Soporta:** GLB, GLTF, OBJ, FBX, Collada, 3DS, y 40+ más
- **Lee:**
  - Geometría (vértices, caras)
  - Texturas
  - Animaciones
  - Esqueletos (bones)

### STB Image - "El Lector de Imágenes"
- **Qué hace:** Carga imágenes (PNG, JPG, BMP, etc.)
- **Uso en proyecto:** Cargar texturas para modelos

## Cómo se Compila el Proyecto

### ¿Qué es Compilar?

**Compilar** es convertir código que los humanos entienden (C++) a código que la computadora entiende (lenguaje máquina).

**Proceso simplificado:**

```
CÓDIGO C++ (.cpp)
      ↓
  COMPILADOR (cl.exe en Windows, g++ en Linux)
      ↓
CÓDIGO OBJETO (.obj)
      ↓
   LINKER (conecta todo)
      ↓
EJECUTABLE (.exe)
```

### Pasos de Compilación con CMake

**CMake** es como un "asistente de compilación" que hace todo automáticamente.

**Paso 1: Configurar**
```bash
cmake -B build
```
- CMake lee `CMakeLists.txt`
- Encuentra todas las bibliotecas (GLFW, ASSIMP, etc.)
- Crea archivos de configuración en carpeta `build/`

**Paso 2: Compilar**
```bash
cmake --build build --config Release
```
- Compila todos los archivos .cpp
- Linkea todas las bibliotecas
- Copia assets (modelos, shaders) al directorio del ejecutable
- Copia DLLs necesarias

**Resultado:**
- `build/Release/Cuadrupedo_IIIU.exe` (el programa)
- `build/Release/assets/` (modelos y shaders)
- `build/Release/assimp-vc143-mt.dll` (biblioteca necesaria)

## Primeros Pasos Recomendados

### Si Nunca Has Programado

1. **Aprende C++ básico primero:**
   - Variables (int, float, string)
   - Funciones
   - Clases y objetos
   - Punteros (concepto avanzado pero importante)

2. **Recursos recomendados:**
   - [LearnCpp.com](https://www.learncpp.com/) (inglés, muy completo)
   - [C++ para principiantes](https://www.youtube.com/results?search_query=c%2B%2B+tutorial+español) (YouTube en español)

3. **Luego aprende sobre gráficos 3D:**
   - [LearnOpenGL.com](https://learnopengl.com/) (excelente tutorial)
   - Conceptos de vectores y matrices

### Si Ya Sabes Programar (Pero No OpenGL)

1. **Estudia los archivos en este orden:**
   ```
   1. docs/README.md              # Visión general
   2. docs/GUIA_PRINCIPIANTES.md  # Este archivo
   3. docs/ARQUITECTURA.md        # Cómo está estructurado
   4. src/main.cpp                # Punto de entrada simple
   5. src/Application.h           # Lógica principal
   6. docs/SHADERS.md             # Entender shaders
   ```

2. **Experimenta modificando:**
   - Color de fondo (línea 252 de Application.h)
   - Velocidad de animación (Animator.h)
   - Posición de la luz (Application.h línea 329)
   - Escala de modelos (Application.h línea 319)

### Si Ya Sabes OpenGL

Puedes enfocarte directamente en:
- [ANIMATION.md](ANIMATION.md) - Sistema de animación esquelética
- [RENDERING.md](RENDERING.md) - Dual rendering system
- [SHADERS.md](SHADERS.md) - Implementación de shaders

## Errores Comunes y Cómo Entenderlos

### Error al Compilar

**Error:** `CMake no encontrado` o `cmake: command not found`

**¿Qué significa?** Tu computadora no tiene CMake instalado.

**Solución:** Instalar CMake desde [cmake.org](https://cmake.org/download/)

---

**Error:** `No se encuentra cl.exe` o `C++ compiler not found`

**¿Qué significa?** No tienes un compilador de C++ instalado.

**Solución Windows:** Instalar Visual Studio Community (gratis)
**Solución Linux:** `sudo apt-get install build-essential`

---

**Error:** `Cannot find GLFW` o similar

**¿Qué significa?** Falta alguna biblioteca en la carpeta `external/`

**Solución:** Verificar que las carpetas external/glfw, external/Assimp, etc. existan y tengan contenido

### Error al Ejecutar

**Error:** Ventana negra y se cierra inmediatamente

**¿Qué significa?** Probablemente no encuentra los archivos de assets (modelos, shaders)

**Solución:**
1. Ejecutar desde `build/Release/`
2. Verificar que exista `build/Release/assets/`
3. Verificar que exista `build/Release/shaders/`

---

**Error:** `Failed to load model`

**¿Qué significa?** No encuentra el archivo .glb

**Solución:**
1. Verificar que `quadruped.glb` esté en `build/Release/assets/models/`
2. Verificar que `scena.glb` esté en `build/Release/assets/models/`
3. Recompilar para que CMake copie automáticamente

---

**Error:** Pantalla blanca o colores extraños

**¿Qué significa?** Error con los shaders

**Solución:**
1. Verificar que los archivos .vert y .frag estén en `build/Release/shaders/`
2. Revisar la consola para mensajes de error de shaders

---

**Error:** `OpenGL version not supported`

**¿Qué significa?** Tu tarjeta gráfica no soporta OpenGL 4.6

**Solución:**
1. Actualizar drivers de la tarjeta gráfica
2. Si es muy antigua, el proyecto requiere modificación (cambiar a OpenGL 3.3)

## Conceptos de Matemáticas 3D (Simplificados)

### Vectores

Un **vector** es una dirección con magnitud (fuerza).

```cpp
glm::vec3 posicion(2.0f, 5.0f, 3.0f);
//              X     Y     Z
```

**Visualización:**
```
      Y (arriba)
      |
      |
      o------- X (derecha)
     /
    /
   Z (hacia ti)
```

### Matrices

Una **matriz** es una tabla de números que representa una transformación.

**Tipos de transformaciones:**

1. **Traslación (Mover):**
```cpp
glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
// Mueve el objeto 5 unidades a la derecha
```

2. **Rotación (Girar):**
```cpp
glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
// Rota 45 grados alrededor del eje Y
```

3. **Escala (Agrandar/Achicar):**
```cpp
glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));
// Duplica el tamaño del objeto
```

**Combinar transformaciones:**
```cpp
glm::mat4 model = glm::mat4(1.0f);
model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));  // Primero mover
model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));  // Luego rotar
model = glm::scale(model, glm::vec3(100.0f, 100.0f, 100.0f));  // Finalmente escalar
```

### Sistema de Coordenadas

En OpenGL (como en este proyecto), usamos un sistema **mano derecha**:

```
- Eje X (rojo): Derecha es positivo, izquierda es negativo
- Eje Y (verde): Arriba es positivo, abajo es negativo
- Eje Z (azul): Hacia ti es positivo, alejándose es negativo
```

## Recursos para Aprender Más

### Tutoriales de OpenGL (Español)

- [Tutorial OpenGL - YouTube](https://www.youtube.com/results?search_query=opengl+tutorial+español)
- [Curso de OpenGL](https://www.opengl-tutorial.org/es/)

### Tutoriales de OpenGL (Inglés - Muy Recomendados)

- [LearnOpenGL.com](https://learnopengl.com/) - Excelente para principiantes
- [OpenGL Tutorial - YouTube (The Cherno)](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2)

### Matemáticas 3D

- [3Blue1Brown - Álgebra Lineal](https://www.youtube.com/playlist?list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab) - Visualizaciones increíbles
- [Essence of Linear Algebra](https://www.3blue1brown.com/topics/linear-algebra)

### C++ Moderno

- [LearnCpp.com](https://www.learncpp.com/)
- [C++ Reference](https://en.cppreference.com/)

### Creación de Modelos 3D

- [Blender Tutorials](https://www.blender.org/support/tutorials/) - Software gratis para crear modelos
- [Mixamo](https://www.mixamo.com/) - Modelos y animaciones gratis

## Preguntas Frecuentes (FAQ)

### ¿Puedo usar este proyecto en Mac?

Parcialmente. macOS solo soporta OpenGL 4.1 como máximo, este proyecto usa 4.6. Necesitarías modificar el código para usar una versión anterior.

### ¿Cuánto espacio ocupa el proyecto completo?

- Código fuente: ~500 KB
- Bibliotecas externas: ~200 MB
- Modelos 3D: ~50 MB
- Total: ~250-300 MB

### ¿Puedo agregar mis propios modelos 3D?

Sí, solo necesitas:
1. Exportar tu modelo como .glb desde Blender
2. Copiar el .glb a `assets/models/`
3. Recompilar el proyecto

### ¿El proyecto funciona en laptops sin tarjeta gráfica dedicada?

Sí, si tu laptop tiene una GPU integrada (Intel HD Graphics, AMD Radeon Vega) que soporte OpenGL 4.6. La mayoría de laptops modernas (2015+) deberían funcionar.

### ¿Puedo cambiar los controles?

Sí, editando el archivo `Application.h` en la función `processInput()`. Ahí defines qué pasa cuando presionas cada tecla.

### ¿Qué FPS (cuadros por segundo) debería tener?

Idealmente 60 FPS. Si ves menos de 30 FPS:
- Tu GPU puede ser muy antigua
- Los modelos pueden ser muy complejos
- Hay un problema de configuración

### ¿Puedo distribuir el ejecutable a otras personas?

Sí, pero necesitan:
1. El archivo .exe
2. La carpeta `assets/`
3. El archivo `assimp-vc143-mt.dll`
4. Visual C++ Redistributable instalado
5. Drivers de GPU actualizados

---

**¿Más preguntas?** Revisa la documentación completa en [README.md](README.md) o busca en los otros archivos de `docs/`.

---

**Última actualización:** Enero 2026
