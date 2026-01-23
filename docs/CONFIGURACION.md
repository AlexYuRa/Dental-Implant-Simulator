# Configuración y Compilación

> 📝 **Nota:** La configuración de compilación es igual para ambas versiones.
> Para usar la versión POO refactorizada, compila `main_refactored.cpp` en lugar de `main.cpp`.

## Requisitos del Sistema

### Hardware

**Mínimo:**
- GPU con soporte OpenGL 4.6
- 4 GB RAM
- 500 MB espacio en disco

**Recomendado:**
- GPU dedicada (NVIDIA/AMD) con drivers actualizados
- 8 GB RAM
- 1 GB espacio en disco

### Software

**Esencial:**
- CMake 3.15 o superior
- Compilador C++17:
  - Windows: MSVC 2019+ (Visual Studio)
  - Linux: GCC 9+ o Clang 10+
  - macOS: Clang (Xcode Command Line Tools)

**Opcional:**
- Git (para clonar repositorio)
- Visual Studio Code (IDE recomendado)
- Blender 3.0+ (para crear/modificar modelos)

### Verificar OpenGL

**Windows:**
```cmd
# Descargar y ejecutar OpenGL Extensions Viewer
# O ejecutar el programa y verificar en consola
```

**Linux:**
```bash
glxinfo | grep "OpenGL version"
```

**macOS:**
```bash
# OpenGL 4.1 es máximo en macOS
# Proyecto requiere modificación para compatibilidad
```

## Dependencias

### Incluidas en el Proyecto (external/)

- **GLFW 3** - Gestión de ventanas
- **GLAD** - Cargador de OpenGL
- **GLM** - Matemáticas 3D
- **ASSIMP** - Importación de modelos
- **STB Image** - Carga de imágenes

**Nota:** No requieren instalación adicional, están en `external/`.

### Sistema

**Windows:**
- Visual Studio Build Tools o Visual Studio completo
- Windows SDK

**Linux:**
```bash
sudo apt-get install build-essential cmake
sudo apt-get install libx11-dev libxrandr-dev libxinerama-dev \
                     libxcursor-dev libxi-dev libgl1-mesa-dev
```

**macOS:**
```bash
xcode-select --install
brew install cmake
```

## Compilación Paso a Paso

### 1. Obtener el Proyecto

```bash
# Si está en Git
git clone [repository-url]
cd Cuadrupedo_IIIU

# Si es un ZIP
unzip Cuadrupedo_IIIU.zip
cd Cuadrupedo_IIIU
```

### 2. Verificar Estructura

```
Cuadrupedo_IIIU/
├── CMakeLists.txt
├── src/
├── shaders/
├── assets/
├── external/
└── docs/
```

### 3. Configurar con CMake

```bash
# Crear directorio de build
cmake -B build

# O especificar generador (Windows)
cmake -B build -G "Visual Studio 17 2022"

# Linux/macOS con Makefile
cmake -B build -G "Unix Makefiles"
```

**Opciones de CMake:**
```bash
# Compilación Debug
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Compilación Release (recomendado para uso)
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Especificar compilador
cmake -B build -DCMAKE_CXX_COMPILER=g++-11
```

### 4. Compilar

**Windows (Visual Studio):**
```bash
cmake --build build --config Release

# O abrir build/Cuadrupedo_IIIU.sln en Visual Studio
```

**Linux/macOS:**
```bash
cmake --build build --config Release

# O con make directamente
cd build
make -j4
```

**Compilación paralela:**
```bash
# Usar todos los cores
cmake --build build --config Release --parallel

# Especificar número de jobs
cmake --build build --config Release -j 8
```

### 5. Ejecutar

```bash
# Windows
cd build/Release
Cuadrupedo_IIIU.exe

# Linux/macOS
cd build
./Cuadrupedo_IIIU
```

## Estructura del Build

```
build/
├── Release/                    # Ejecutable Release
│   ├── Cuadrupedo_IIIU.exe
│   └── assets/                # Copiado automáticamente
│       ├── models/
│       │   ├── quadruped.glb
│       │   └── scena.glb
│       └── shaders/           # (si se copian)
├── Debug/                      # Ejecutable Debug
│   └── Cuadrupedo_IIIU.exe
└── CMakeFiles/                # Archivos internos de CMake
```

## CMakeLists.txt Explicado

### Configuración Básica

```cmake
cmake_minimum_required(VERSION 3.15)
project(Cuadrupedo_IIIU)

# Estándar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
```

### Archivos Fuente

```cmake
# Código fuente
file(GLOB SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Crear ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})
```

### Bibliotecas Externas

```cmake
# GLFW
add_subdirectory(external/glfw)
target_link_libraries(${PROJECT_NAME} glfw)

# GLAD
add_library(glad external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)
target_link_libraries(${PROJECT_NAME} glad)

# GLM (header-only)
target_include_directories(${PROJECT_NAME} PUBLIC external/glm)

# ASSIMP
add_subdirectory(external/assimp)
target_link_libraries(${PROJECT_NAME} assimp)

# STB (header-only)
target_include_directories(${PROJECT_NAME} PUBLIC external/stb)
```

### Copiar Assets

```cmake
# Copiar carpeta assets al directorio de salida
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
```

**Explicación:**
- `POST_BUILD`: Ejecuta después de compilar
- `copy_directory`: Copia recursivamente
- `$<TARGET_FILE_DIR:${PROJECT_NAME}>`: Directorio del ejecutable
- Se ejecuta automáticamente cada vez que compilas

## Configuraciones de Compilación

### Debug vs Release

**Debug:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```
- Sin optimizaciones
- Símbolos de debug incluidos
- Más lento pero útil para debugging

**Release:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
- Optimizaciones máximas (-O3)
- Sin símbolos de debug
- Más rápido, para distribución

**RelWithDebInfo:**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
```
- Optimizado pero con símbolos de debug
- Balance entre performance y debugging

### Flags de Compilación

**Agregar warnings:**
```cmake
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()
```

**Optimizaciones específicas:**
```cmake
if(CMAKE_BUILD_TYPE MATCHES Release)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /O2)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -O3 -march=native)
    endif()
endif()
```

## Troubleshooting de Compilación

### Error: CMake no encontrado

**Síntoma:** `cmake: command not found`

**Windows:**
```cmd
# Descargar e instalar desde cmake.org
# O con chocolatey:
choco install cmake
```

**Linux:**
```bash
sudo apt-get install cmake
```

**macOS:**
```bash
brew install cmake
```

### Error: Compilador C++ no encontrado

**Windows:**
- Instalar Visual Studio Community (gratis)
- O Visual Studio Build Tools

**Linux:**
```bash
sudo apt-get install build-essential
```

### Error: OpenGL headers not found

**Linux:**
```bash
sudo apt-get install libgl1-mesa-dev libglu1-mesa-dev
```

### Error: Cannot find GLFW/GLAD/etc.

**Causa:** Subcarpetas `external/` no pobladas

**Solución:**
```bash
# Si usaste git sin --recursive
git submodule update --init --recursive

# O copiar manualmente bibliotecas a external/
```

### Error: ASSIMP linking error

**Solución:**
```cmake
# En CMakeLists.txt, asegurar:
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
```

### Warning: Muchos warnings de ASSIMP/GLFW

**Normal** - Bibliotecas externas pueden tener warnings.

**Para silenciar:**
```cmake
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC external/assimp/include)
```

## Configuración de IDE

### Visual Studio Code

**Extensiones recomendadas:**
- C/C++ (Microsoft)
- CMake Tools
- CMake Language Support

**settings.json:**
```json
{
    "cmake.configureOnOpen": true,
    "cmake.buildDirectory": "${workspaceFolder}/build",
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

**tasks.json:**
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "CMake Build Release",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "build",
                "--config",
                "Release"
            ]
        }
    ]
}
```

### Visual Studio

1. Abrir `build/Cuadrupedo_IIIU.sln`
2. Seleccionar configuración Release
3. Build → Build Solution (Ctrl+Shift+B)
4. Run (F5)

### CLion

1. File → Open → Seleccionar carpeta raíz
2. CLion detecta CMakeLists.txt automáticamente
3. Build → Build Project
4. Run → Run 'Cuadrupedo_IIIU'

## Distribución

### Preparar para Distribución

```bash
# Compilar Release
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

# Copiar ejecutable y assets
mkdir distribution
cp build/Release/Cuadrupedo_IIIU.exe distribution/
cp -r build/Release/assets distribution/
```

### Dependencias Runtime

**Windows:**
- Visual C++ Redistributable (si usa MSVC)
- OpenGL drivers actualizados

**Linux:**
```bash
ldd Cuadrupedo_IIIU
# Verificar dependencias compartidas
```

## Variables de Entorno

### Ubicación de Assets

Por defecto busca `assets/` en directorio del ejecutable.

**Modificar en código:**
```cpp
// En Application.h o config file
const std::string ASSETS_PATH = "./assets/";
// O usar ruta absoluta
const std::string ASSETS_PATH = "C:/Projects/Cuadrupedo_IIIU/assets/";
```

### OpenGL Debug Context

```cpp
// En Application.h, initWindow()
#ifdef _DEBUG
glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
```

## Actualizar Bibliotecas

### Actualizar GLFW

```bash
cd external/glfw
git fetch
git checkout [nueva-versión]
```

### Actualizar ASSIMP

```bash
cd external/assimp
git pull origin master
```

**Luego recompilar:**
```bash
cd ../..
cmake --build build --config Release --clean-first
```

## Limpieza de Build

### Limpiar completamente

```bash
# Eliminar build directory
rm -rf build

# Reconfigurar desde cero
cmake -B build
cmake --build build --config Release
```

### Limpiar solo binarios

```bash
cmake --build build --target clean
```

---

**Documentación completa disponible en:** `docs/README.md`
