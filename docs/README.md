# Simulador de Implantes Dentales - Documentación del Proyecto

## Descripción General

Sistema de simulación 3D interactivo para procedimientos de implantes dentales, desarrollado con OpenGL 4.6 Core Profile. Permite visualizar, manipular y transformar instrumental quirúrgico dental en un entorno 3D realista.

**Características principales:**
- Animación esqueletal con soporte para hasta 200 huesos
- Carga de 72+ modelos GLB de instrumental dental con texturas embebidas
- Sistema de materiales PBR (baseColor, metallic, roughness, opacity)
- Sistema de cámara orbital con zoom, rotación y pan
- Iluminación Phong con soporte para materiales metálicos
- Sistema de transformación completo (Trasladar/Rotar/Escalar)
- HUD visual para feedback de modo de transformación activo
- Sistema de selección y arrastre de objetos (Drag & Drop 3D)
- Detección de colisiones con AABB transformados
- Sistema de ejes de coordenadas para orientación

## Tecnologías Utilizadas

- **OpenGL 4.6 Core Profile** - Renderizado gráfico moderno
- **GLFW 3** - Gestión de ventanas y entrada de usuario
- **GLAD** - Cargador de funciones OpenGL
- **GLM** - Matemáticas 3D (vectores, matrices, transformaciones)
- **ASSIMP** - Importación de modelos 3D (GLB/GLTF)
- **STB Image** - Carga de texturas (embebidas y externas)
- **C++17** - Lenguaje de programación

## Estructura del Proyecto

```
Cuadrupedo_IIIU/
├── src/                    # Código fuente C++
│   ├── main.cpp           # Punto de entrada (versión original)
│   ├── main_refactored.cpp # Punto de entrada (versión POO)
│   ├── Application.h      # Clase principal original
│   ├── ApplicationRefactored.h # Clase principal POO ⭐ NUEVO
│   │
│   ├── interfaces/        # Interfaces abstractas ⭐ NUEVO
│   │   ├── IRenderable.h      # Contrato para objetos renderizables
│   │   ├── ITransformable.h   # Contrato para objetos transformables
│   │   └── IUpdatable.h       # Contrato para objetos actualizables
│   │
│   ├── core/              # Clases núcleo ⭐ NUEVO
│   │   ├── TransformTypes.h   # Enums (Mode, Axis, Space)
│   │   ├── Keyframe.h         # Estructuras de keyframes
│   │   ├── InputState.h       # Estado centralizado de input
│   │   ├── SceneObject.h      # Clase base para objetos de escena
│   │   ├── SelectionManager.h # Gestión de selección
│   │   └── TransformController.h # Control de transformaciones
│   │
│   ├── managers/          # Managers de sistemas ⭐ NUEVO
│   │   ├── SceneManager.h     # Gestión de escena completa
│   │   ├── RenderManager.h    # Sistema de renderizado
│   │   └── InputManager.h     # Gestión de input
│   │
│   ├── Camera.h           # Sistema de cámara orbital
│   ├── Shader.h           # Gestión de shaders
│   ├── Model.h            # Carga de modelos 3D y materiales PBR
│   ├── Mesh.h             # Representación de mallas con materiales
│   ├── Animation.h        # Sistema de animación
│   ├── Animator.h         # Controlador de animaciones
│   ├── Bone.h             # Estructura de huesos
│   ├── InteractionSystem.h # Ray casting y selección 3D
│   └── stb_image.h        # Carga de texturas
│
├── shaders/               # Shaders GLSL
│   ├── skeletal_animation.vert  # Vertex shader con skinning (200 bones)
│   ├── skeletal_animation.frag  # Fragment shader con materiales PBR
│   ├── static.vert       # Vertex shader para modelos estáticos
│   ├── static.frag       # Fragment shader con materiales PBR
│   ├── axis.vert         # Shader para ejes de coordenadas
│   ├── axis.frag         # Shader para ejes de coordenadas
│   ├── hud.vert          # Shader para HUD de transformación
│   └── hud.frag          # Shader para HUD de transformación
├── assets/               # Recursos del proyecto
│   └── models/          # 72+ modelos GLB de instrumental dental
│       ├── CABEZAL-*.glb       # Cabezales de implante
│       ├── CUERPO.glb          # Modelo del paciente
│       ├── DENTADURA*.glb      # Dentaduras
│       ├── tijera*.glb         # Tijeras quirúrgicas
│       ├── forcet*.glb         # Fórceps
│       ├── alicate*.glb        # Alicates
│       ├── aguja*.glb          # Agujas
│       └── ...                 # Más instrumental
├── external/            # Bibliotecas externas
├── docs/               # Documentación
└── CMakeLists.txt      # Configuración de compilación
```

## ¿Por Dónde Empezar?

### Si Eres Principiante o Estás Iniciando en Programación

1. **[Guía para Principiantes](GUIA_PRINCIPIANTES.md)** ⭐ EMPIEZA AQUÍ
   - Conceptos básicos explicados de forma simple
   - Glosario de términos técnicos
   - Cómo funciona el proyecto (sin tecnicismos)
   - Recursos para aprender más

2. **[Instalación en Nueva PC](INSTALACION_NUEVA_PC.md)** ⭐ IMPORTANTE
   - Guía paso a paso para instalar en otra computadora
   - Instalación de todas las herramientas necesarias
   - Solución de problemas comunes
   - Checklist de verificación completa

3. **[Controles](CONTROLES.md)**
   - Cómo usar el programa
   - Guía de controles de teclado y mouse

### Si Ya Tienes Experiencia en Programación

1. **[Configuración](CONFIGURACION.md)** - Compilación y setup
2. **[Arquitectura](ARQUITECTURA.md)** - Diseño del sistema
3. Explora los documentos técnicos según lo que necesites

---

## Documentación Detallada

### Para Principiantes
- **[Guía para Principiantes](GUIA_PRINCIPIANTES.md)** - Conceptos básicos explicados
- **[Instalación en Nueva PC](INSTALACION_NUEVA_PC.md)** - Setup completo paso a paso
- **[Lógica Global](LOGICA_GLOBAL.md)** - Flujo completo del programa explicado paso a paso
- [Controles](CONTROLES.md) - Guía de controles del usuario

### Arquitectura y Sistemas
- [Arquitectura del Proyecto](ARQUITECTURA.md) - Diseño general y patrones
- **[Refactorización POO](POO_REFACTORIZACION.md)** - Nueva arquitectura orientada a objetos ⭐ NUEVO
- [Sistema de Renderizado](RENDERING.md) - Pipeline de renderizado y shaders
- [Sistema de Animación](ANIMATION.md) - Animación esquelética
- [Sistema de Interactividad](INTERACTIVIDAD.md) - Ray casting y drag & drop 3D
- [Sistema de Iluminación](ILUMINACION.md) - Modelo Phong y configuración

### Componentes
- [Modelos y Texturas](MODELS.md) - Carga de GLB y gestión de texturas
- [Cámara](CAMERA.md) - Sistema de cámara orbital

### Referencia Técnica
- [Shaders](SHADERS.md) - Referencia de shaders GLSL
- [Configuración](CONFIGURACION.md) - Compilación y configuración

## Inicio Rápido

### Requisitos Previos
- CMake 3.15+
- Compilador C++17 (MSVC, GCC, Clang)
- OpenGL 4.6 compatible GPU

### Compilación

```bash
# Configurar proyecto
cmake -B build

# Compilar (Release)
cmake --build build --config Release

# Ejecutar
cd build/Release
./Cuadrupedo_IIIU.exe
```

### Controles Básicos

**Cámara:**
- **Clic derecho + Arrastrar**: Rotar cámara (orbital)
- **Clic medio + Arrastrar**: Pan (desplazar vista)
- **Scroll**: Zoom in/out
- **WASD**: Mover objetivo
- **Q/E**: Mover objetivo arriba/abajo
- **SHIFT + WASD**: Movimiento rápido (3x)
- **R**: Resetear cámara

**Selección y Transformación:**
- **Clic izquierdo**: Seleccionar y arrastrar objeto
- **1**: Modo TRASLADAR (barra verde en HUD)
- **2**: Modo ROTAR (barra azul en HUD)
- **3**: Modo ESCALAR (barra amarilla en HUD)
- **X/Y/Z**: Restringir a eje específico
- **Flechas**: Aplicar transformación
- **+/-**: Aumentar/disminuir escala
- **ESC**: Deseleccionar objeto / Salir

## Configuración Actual

### Renderizado
- Resolución: 1280x720
- FOV: 45°
- Rango de renderizado: 0.1 - 10000 unidades
- Color de fondo: Celeste claro (135, 206, 235)

### Iluminación (Phong)
- Tipo: Point Light (Luz puntual)
- Posición: (0, 5, 20)
- Color: Blanco (1.3, 1.3, 1.3)
- Ambient: 0.35
- Specular: Ajustado por material.metallic
- Shininess: Ajustado por material.roughness

### Sistema de Materiales (PBR)
- **baseColor**: Color base del material (RGB)
- **metallic**: Factor metálico (0-1)
- **roughness**: Rugosidad de la superficie (0-1)
- **opacity**: Transparencia (0-1)

### Cámara
- Distancia inicial: 5 unidades
- Rango de zoom: 1.0 - 200.0 unidades
- Sensibilidad de rotación: 0.1
- Velocidad de movimiento: 2.5 unidades/segundo

## Características Avanzadas

### Sistema de Transformación
El sistema permite transformar objetos seleccionados en 3 modos:
1. **TRASLADAR** - Mover objetos en el espacio 3D
2. **ROTAR** - Rotar objetos alrededor de su centro (rotación local)
3. **ESCALAR** - Cambiar el tamaño de objetos

Cada modo puede restringirse a ejes individuales (X, Y, Z) o aplicarse a todos.

### HUD Visual
Indicadores en pantalla que muestran:
- **Barras superiores**: Modo activo (Verde=Trasladar, Azul=Rotar, Amarillo=Escalar)
- **Barras inferiores**: Eje activo (Rojo=X, Verde=Y, Azul=Z, Gris=Todos)

### Dual Rendering
El sistema renderiza simultáneamente:
1. **72+ modelos estáticos** (instrumental dental) - Con materiales PBR
2. **Modelos animados** (opcional) - Con skeletal animation

### Skeletal Animation
- Soporte para hasta 200 huesos
- Máximo 4 influencias por vértice
- Interpolación automática de keyframes
- Control de velocidad y ciclos de animación

### Sistema de Texturas y Materiales
- Texturas embebidas en GLB (formato JPEG/PNG)
- Extracción automática de propiedades PBR de archivos GLTF/GLB
- Materiales sin textura usan color base del material
- Generación automática de mipmaps

### Sistema de Selección (Ray Casting)
- Detección precisa con ray-AABB intersection
- AABB transformados que siguen rotación/escala del objeto
- Soporte para múltiples objetos arrastrables

## Autor y Licencia

Proyecto desarrollado para la Universidad IIIU - Simulador de Implantes Dentales.

---

**Última actualización:** Enero 2026
