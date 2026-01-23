# Instalación en una Nueva Computadora

Esta guía te ayudará a instalar y ejecutar este proyecto en una computadora nueva, desde cero. Está diseñada para ser seguida paso a paso, incluso si es tu primera vez haciendo esto.

## Tabla de Contenidos

1. [Requisitos Previos](#requisitos-previos)
2. [Instalación de Herramientas](#instalación-de-herramientas)
3. [Obtener el Proyecto](#obtener-el-proyecto)
4. [Compilar el Proyecto](#compilar-el-proyecto)
5. [Ejecutar el Programa](#ejecutar-el-programa)
6. [Solución de Problemas](#solución-de-problemas)
7. [Verificación Completa](#verificación-completa)

---

## Requisitos Previos

### Hardware Necesario

**Mínimo:**
- Procesador: Intel Core i3 o equivalente
- RAM: 4 GB
- GPU: Tarjeta gráfica con soporte OpenGL 4.6
- Espacio en disco: 2 GB libres

**Recomendado:**
- Procesador: Intel Core i5 o superior
- RAM: 8 GB o más
- GPU: NVIDIA GTX 1050 / AMD RX 560 o superior
- Espacio en disco: 5 GB libres

### Cómo Verificar Tu GPU (Windows)

1. Presiona `Windows + R`
2. Escribe `dxdiag` y presiona Enter
3. Ve a la pestaña "Display" o "Pantalla"
4. Verifica el nombre de tu tarjeta gráfica

**Tarjetas gráficas compatibles:**
- NVIDIA: Serie GTX 900 o superior (2014+)
- AMD: Serie RX 400 o superior (2016+)
- Intel: HD Graphics 500 o superior (2016+)

**Si tu GPU no es compatible:** Necesitarás modificar el proyecto para usar OpenGL 3.3 (consulta sección de troubleshooting).

---

## Instalación de Herramientas

Necesitas instalar 3 programas principales:
1. **Visual Studio** (compilador de C++)
2. **CMake** (herramienta de compilación)
3. **Git** (para descargar el proyecto - opcional)

### Paso 1: Instalar Visual Studio 2022

Visual Studio es el compilador que convierte el código C++ en un programa ejecutable.

**Instrucciones detalladas:**

1. **Descargar Visual Studio:**
   - Ve a: https://visualstudio.microsoft.com/es/downloads/
   - Descarga **Visual Studio 2022 Community** (es gratis)
   - El archivo pesa aproximadamente 3-4 MB (es un instalador pequeño)

2. **Ejecutar el Instalador:**
   - Ejecuta el archivo descargado (`vs_community.exe`)
   - El instalador descargará los componentes necesarios (esto puede tardar)

3. **Seleccionar Componentes:**

   En la ventana del instalador, selecciona:

   ✅ **Desktop development with C++** (Desarrollo de escritorio con C++)

   Esto instalará:
   - Compilador MSVC
   - Windows SDK
   - CMake (ya viene incluido)
   - Herramientas de debug

   **Espacio requerido:** Aproximadamente 7-10 GB

4. **Componentes Opcionales:**

   En el panel derecho (detalles de instalación), asegúrate que estén marcados:
   - ✅ MSVC v143 - VS 2022 C++ x64/x86 build tools
   - ✅ Windows 10/11 SDK
   - ✅ C++ CMake tools for Windows

5. **Instalar:**
   - Click en "Install" o "Instalar"
   - Espera 30-60 minutos (depende de tu conexión a internet)

6. **Verificar Instalación:**

   ```cmd
   # Abre "Command Prompt" o "Símbolo del sistema"
   # Escribe:
   cl
   ```

   Deberías ver información sobre el compilador Microsoft C/C++.
   Si dice "no se reconoce como comando", reinicia tu computadora.

### Paso 2: Instalar CMake (Si No Vino con Visual Studio)

CMake ya debería estar instalado con Visual Studio, pero verifica:

1. **Verificar si CMake está instalado:**

   ```cmd
   cmake --version
   ```

   Si muestra una versión (ej: `cmake version 3.25.1`), ya lo tienes instalado. **Salta al Paso 3**.

2. **Si NO está instalado:**

   - Ve a: https://cmake.org/download/
   - Descarga "Windows x64 Installer" (archivo .msi)
   - Ejecuta el instalador
   - **IMPORTANTE:** En la pantalla de instalación, selecciona:
     - ✅ "Add CMake to the system PATH for all users"

3. **Verificar instalación:**

   Cierra y vuelve a abrir el Command Prompt, luego:

   ```cmd
   cmake --version
   ```

### Paso 3: Instalar Git (Opcional pero Recomendado)

Git te permite descargar proyectos fácilmente. Si tienes el proyecto en un USB o descargado como ZIP, puedes saltar este paso.

1. **Descargar Git:**
   - Ve a: https://git-scm.com/download/win
   - Descarga la versión de 64-bit

2. **Instalar Git:**
   - Ejecuta el instalador
   - Deja todas las opciones por defecto (solo presiona "Next")
   - Instala

3. **Verificar:**
   ```cmd
   git --version
   ```

---

## Obtener el Proyecto

Hay 3 formas de obtener el proyecto. Elige la que prefieras:

### Opción A: Copiar desde USB o Disco Duro

1. Copia la carpeta completa `Cuadrupedo_IIIU` a tu computadora
2. Ubícala en un lugar fácil de acceder, por ejemplo:
   ```
   C:\Users\TuNombre\Documents\Cuadrupedo_IIIU
   ```

**IMPORTANTE:** No pongas el proyecto en rutas con espacios o caracteres especiales:
- ❌ MAL: `C:\Mis Documentos\Proyecto C++\Cuadrupedo_IIIU`
- ✅ BIEN: `C:\Proyectos\Cuadrupedo_IIIU`
- ✅ BIEN: `C:\Users\Juan\Cuadrupedo_IIIU`

### Opción B: Descargar como ZIP

1. Si tienes el proyecto en GitHub, GitLab, etc.:
   - Ve a la página del proyecto
   - Click en "Code" o "Código"
   - Click en "Download ZIP"

2. Extrae el ZIP:
   - Click derecho en el archivo descargado
   - "Extraer todo" o "Extract all"
   - Elige un destino (ej: `C:\Proyectos\`)

### Opción C: Clonar con Git

```cmd
# Abre Command Prompt
# Navega a donde quieres el proyecto
cd C:\Proyectos

# Clona el repositorio (reemplaza URL con la real)
git clone https://github.com/usuario/Cuadrupedo_IIIU.git

# Entra a la carpeta
cd Cuadrupedo_IIIU
```

### Verificar Estructura del Proyecto

Después de obtener el proyecto, verifica que tenga esta estructura:

```
Cuadrupedo_IIIU/
├── CMakeLists.txt          ← Debe existir
├── src/                    ← Carpeta con código
├── shaders/                ← Carpeta con shaders
├── assets/                 ← Carpeta con modelos
│   └── models/
│       ├── quadruped.glb   ← Modelo del animal
│       └── scena.glb       ← Modelo de la escena
├── external/               ← Bibliotecas externas
│   ├── glfw/               ← Debe existir y tener contenido
│   ├── GLAD/               ← Debe existir y tener contenido
│   ├── glm/                ← Debe existir y tener contenido
│   ├── Assimp/             ← Debe existir y tener contenido
│   │   ├── bin/
│   │   │   └── x64/
│   │   │       └── assimp-vc143-mt.dll  ← DLL importante
│   │   ├── include/
│   │   └── lib/
│   └── stb/
└── docs/                   ← Documentación
```

**SI FALTA LA CARPETA `external/` O ESTÁ VACÍA:**
- El proyecto no compilará
- Necesitas obtener las bibliotecas o usar una copia completa del proyecto

---

## Compilar el Proyecto

Ahora compilaremos el proyecto. Hay 2 métodos: **línea de comandos** (más confiable) o **Visual Studio** (interfaz gráfica).

### Método 1: Compilar con Línea de Comandos (Recomendado)

Este método es más confiable y muestra mejor los errores.

#### Paso 1: Abrir Command Prompt

1. Presiona `Windows + R`
2. Escribe `cmd` y presiona Enter

#### Paso 2: Navegar al Proyecto

```cmd
# Cambia a la ruta donde está tu proyecto
# Ejemplo:
cd C:\Proyectos\Cuadrupedo_IIIU

# Verifica que estás en el lugar correcto
dir
```

Deberías ver los archivos `CMakeLists.txt`, carpetas `src`, `shaders`, etc.

#### Paso 3: Configurar con CMake

```cmd
cmake -B build -G "Visual Studio 17 2022"
```

**¿Qué hace este comando?**
- `-B build`: Crea una carpeta llamada `build` con los archivos de compilación
- `-G "Visual Studio 17 2022"`: Usa Visual Studio 2022 como compilador

**Tiempo estimado:** 10-30 segundos

**Salida esperada:**
```
-- The C compiler identification is MSVC 19.34.31937.0
-- The CXX compiler identification is MSVC 19.34.31937.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
...
-- Configuring done
-- Generating done
-- Build files have been written to: C:/Proyectos/Cuadrupedo_IIIU/build
```

Si ves `-- Generating done` al final, ¡todo está bien! 

#### Paso 4: Compilar

```cmd
cmake --build build --config Release
```

**¿Qué hace este comando?**
- `--build build`: Compila usando la carpeta `build`
- `--config Release`: Compila en modo optimizado (rápido)

**Tiempo estimado:** 2-10 minutos (dependiendo de tu PC)

**Salida esperada:**
```
Microsoft (R) Build Engine version ...
Building Custom Rule ...
Compiling...
  main.cpp
  Model.cpp
  Mesh.cpp
  stb_image.cpp
  glad.c
Linking...
   Creating library ...
Cuadrupedo_IIIU.vcxproj -> C:\...\build\Release\Cuadrupedo_IIIU.exe

Build succeeded.
    0 Warning(s)
    0 Error(s)
```

Si ves `Build succeeded`, ¡compiló correctamente!

#### Paso 5: Verificar Archivos Generados

```cmd
# Listar archivos en Release
dir build\Release
```

Deberías ver:
- ✅ `Cuadrupedo_IIIU.exe` (el programa)
- ✅ `assimp-vc143-mt.dll` (biblioteca necesaria)
- ✅ Carpeta `assets/` (modelos y recursos)

---

### Método 2: Compilar con Visual Studio (Interfaz Gráfica)

#### Paso 1: Generar Solución

```cmd
# En Command Prompt, en la carpeta del proyecto
cmake -B build -G "Visual Studio 17 2022"
```

#### Paso 2: Abrir en Visual Studio

1. Navega a `Cuadrupedo_IIIU\build\`
2. Doble click en `Cuadrupedo_IIIU.sln`
3. Visual Studio se abrirá

#### Paso 3: Configurar y Compilar

1. En la barra superior, cambia de "Debug" a **"Release"**
2. Asegúrate que diga **"x64"** (no x86)
3. Menú: `Build` → `Build Solution` (o presiona `Ctrl+Shift+B`)

#### Paso 4: Esperar

Verás el progreso en la parte inferior. Cuando termine dirá:
```
========== Build: 1 succeeded, 0 failed, 0 up-to-date, 0 skipped ==========
```

---

## Ejecutar el Programa

### Desde la Línea de Comandos

```cmd
# Navega al directorio Release
cd build\Release

# Ejecuta el programa
Cuadrupedo_IIIU.exe
```

### Desde el Explorador de Windows

1. Abre el explorador de archivos
2. Navega a `Cuadrupedo_IIIU\build\Release\`
3. Doble click en `Cuadrupedo_IIIU.exe`

### Desde Visual Studio

1. En Visual Studio, presiona `F5` o click en el botón verde ▶️ "Local Windows Debugger"

---

### ¿Qué Deberías Ver?

Si todo funciona correctamente, deberías ver:

1. **Una ventana de 1280x720 pixels**
2. **Fondo color celeste** (azul cielo)
3. **Un cuadrúpedo animado** moviéndose
4. **Una escena 3D rotando** (100x su tamaño original)
5. **Ejes de coordenadas** (X=rojo, Y=verde, Z=azul)

### Controles Básicos

- **Click derecho + Arrastrar:** Rotar cámara
- **Scroll del mouse:** Acercar/Alejar zoom
- **W/A/S/D:** Mover el objetivo de la cámara
- **Q/E:** Mover objetivo arriba/abajo
- **R:** Resetear cámara a posición inicial
- **ESC:** Cerrar programa

---

## Solución de Problemas

### Problema 1: "cmake no se reconoce como comando"

**Causa:** CMake no está instalado o no está en el PATH.

**Solución:**

1. Reinstala CMake y marca "Add to PATH" durante instalación
2. O agrega manualmente a PATH:
   - Busca donde se instaló CMake (ej: `C:\Program Files\CMake\bin`)
   - Windows + R → `sysdm.cpl` → Pestaña "Advanced" → "Environment Variables"
   - Edita "Path" → Agregar nueva entrada → Pega la ruta de CMake
   - Reinicia Command Prompt

### Problema 2: "No se puede abrir archivo de entrada 'glfw3.lib'"

**Causa:** No se encuentra la biblioteca GLFW.

**Solución:**

1. Verifica que exista: `external\glfw\lib-vc2022\glfw3.lib`
2. Si no existe:
   - La carpeta `external/` está incompleta
   - Necesitas una copia completa del proyecto
   - O descarga GLFW precompilado de: https://www.glfw.org/download.html

### Problema 3: "assimp-vc143-mt.dll no encontrado"

**Causa:** Falta la DLL de ASSIMP.

**Solución:**

1. Verifica que exista: `external\Assimp\bin\x64\assimp-vc143-mt.dll`
2. Copia manualmente el DLL a `build\Release\`:
   ```cmd
   copy external\Assimp\bin\x64\assimp-vc143-mt.dll build\Release\
   ```

### Problema 4: El programa abre y cierra inmediatamente

**Causa:** No encuentra los assets (modelos, shaders).

**Solución:**

1. Ejecuta DESDE la carpeta `build\Release\`:
   ```cmd
   cd build\Release
   Cuadrupedo_IIIU.exe
   ```

2. Verifica que existan:
   ```
   build\Release\assets\models\quadruped.glb
   build\Release\assets\models\scena.glb
   ```

3. Si faltan, cópialos manualmente:
   ```cmd
   # Desde la raíz del proyecto
   xcopy /E /I assets build\Release\assets
   ```

### Problema 5: Pantalla negra o gris

**Causa 1:** No encuentra los shaders.

**Solución:**
```cmd
# Copiar shaders manualmente
xcopy /E /I shaders build\Release\shaders
```

**Causa 2:** Tu GPU no soporta OpenGL 4.6.

**Verificar versión de OpenGL:**
1. Descarga: https://www.realtech-vr.com/home/glview
2. Ejecuta y revisa "OpenGL version"

**Si es menor a 4.6:** Necesitas modificar el código (contacta para ayuda o busca en documentación cómo cambiar a OpenGL 3.3).

### Problema 6: Error LNK2019 o símbolos no resueltos

**Causa:** Problema con el linkeo de bibliotecas.

**Solución:**

1. Limpia y recompila:
   ```cmd
   # Eliminar build
   rmdir /S /Q build

   # Reconfigurar
   cmake -B build -G "Visual Studio 17 2022"

   # Recompilar
   cmake --build build --config Release
   ```

2. Verifica que CMakeLists.txt tenga las bibliotecas correctas.

### Problema 7: Error "Failed to create OpenGL context"

**Causa:** Drivers de GPU desactualizados.

**Solución:**

**NVIDIA:**
1. Ve a: https://www.nvidia.com/Download/index.aspx
2. Descarga el driver más reciente para tu GPU
3. Instala

**AMD:**
1. Ve a: https://www.amd.com/support
2. Descarga AMD Software: Adrenalin Edition
3. Instala

**Intel:**
1. Ve a: https://www.intel.com/content/www/us/en/download-center/home.html
2. Descarga Intel Graphics Driver
3. Instala

### Problema 8: Compilación muy lenta

**Solución:** Compila usando todos los núcleos de CPU:

```cmd
cmake --build build --config Release --parallel
```

O especifica número de threads:
```cmd
cmake --build build --config Release -j 8
```

### Problema 9: Warning LNK4098 (MSVCRT conflicts)

**Causa:** Conflicto entre bibliotecas Debug y Release (es normal con ASSIMP).

**Solución:** El CMakeLists.txt ya lo suprime con `/ignore:4098`. Puedes ignorar este warning, no afecta funcionalidad.

---

## Verificación Completa

Usa esta checklist para verificar que todo está correcto:

### Pre-Compilación

- [ ] Visual Studio 2022 instalado
- [ ] CMake instalado (versión 3.15+)
- [ ] Carpeta `Cuadrupedo_IIIU` en ubicación sin espacios
- [ ] Existe `CMakeLists.txt` en la raíz
- [ ] Carpeta `external/` existe y tiene contenido:
  - [ ] `external/glfw/`
  - [ ] `external/GLAD/`
  - [ ] `external/glm/`
  - [ ] `external/Assimp/`
  - [ ] `external/stb/`
- [ ] Carpeta `assets/models/` contiene:
  - [ ] `quadruped.glb`
  - [ ] `scena.glb`
- [ ] Carpeta `shaders/` contiene archivos .vert y .frag

### Compilación

- [ ] `cmake -B build` ejecutó sin errores
- [ ] `cmake --build build --config Release` completó con "Build succeeded"
- [ ] Se creó carpeta `build/Release/`

### Post-Compilación

- [ ] Existe `build/Release/Cuadrupedo_IIIU.exe`
- [ ] Existe `build/Release/assimp-vc143-mt.dll`
- [ ] Existe `build/Release/assets/models/quadruped.glb`
- [ ] Existe `build/Release/assets/models/scena.glb`

### Ejecución

- [ ] El programa abre una ventana (no se cierra inmediatamente)
- [ ] Se ve un fondo celeste
- [ ] Se ve el cuadrúpedo animado
- [ ] Se ve la escena rotando
- [ ] La cámara responde a controles (click derecho + arrastrar)
- [ ] El zoom funciona (scroll del mouse)
- [ ] Presionar ESC cierra el programa

---

## Transferir el Proyecto a Otra Computadora

Si quieres llevar el proyecto compilado a otra computadora:

### Opción A: Solo Ejecutable (Para Uso)

Copia estos archivos/carpetas:
```
Cuadrupedo_IIIU_Portable/
├── Cuadrupedo_IIIU.exe
├── assimp-vc143-mt.dll
└── assets/
    └── models/
        ├── quadruped.glb
        └── scena.glb
```

**Requisitos en la otra PC:**
- Windows 10/11
- Visual C++ Redistributable 2022: https://aka.ms/vs/17/release/vc_redist.x64.exe
- Drivers de GPU actualizados

### Opción B: Proyecto Completo (Para Desarrollo)

Copia toda la carpeta `Cuadrupedo_IIIU`:
```
Cuadrupedo_IIIU/
├── src/
├── shaders/
├── assets/
├── external/           ← IMPORTANTE: Incluir todo
├── docs/
└── CMakeLists.txt
```

**NO copies la carpeta `build/`** (se regenera al compilar).

**En la nueva PC:**
1. Instala Visual Studio 2022
2. Instala CMake
3. Sigue los pasos de compilación desde el inicio

---

## Consejos y Mejores Prácticas

### Organización

1. **Mantén proyectos en una carpeta dedicada:**
   ```
   C:\Proyectos\
   ├── Cuadrupedo_IIIU\
   ├── OtroProyecto\
   └── ...
   ```

2. **Evita rutas con espacios:**
   - ❌ `C:\Mis Proyectos\` (tiene espacio)
   - ✅ `C:\Proyectos\` (sin espacios)

3. **No modifiques `external/`:**
   - Esta carpeta contiene bibliotecas externas
   - No edites nada ahí a menos que sepas exactamente qué haces

### Compilación

1. **Usa Release para ejecutar, Debug para desarrollo:**
   ```cmd
   # Release: Rápido, optimizado
   cmake --build build --config Release

   # Debug: Más lento, útil para encontrar errores
   cmake --build build --config Debug
   ```

2. **Limpia y recompila si algo falla:**
   ```cmd
   rmdir /S /Q build
   cmake -B build
   cmake --build build --config Release
   ```

3. **Compila en paralelo para mayor velocidad:**
   ```cmd
   cmake --build build --config Release --parallel
   ```

### Seguridad

1. **Actualiza drivers regularmente:**
   - Drivers de GPU desactualizados causan muchos problemas

2. **Descarga software oficial:**
   - Visual Studio: https://visualstudio.microsoft.com/
   - CMake: https://cmake.org/
   - Git: https://git-scm.com/

3. **Haz respaldos:**
   - Antes de modificar código importante
   - Usa Git para control de versiones

---

## Recursos Adicionales

### Documentación del Proyecto

- [README.md](README.md) - Índice general
- [GUIA_PRINCIPIANTES.md](GUIA_PRINCIPIANTES.md) - Conceptos básicos explicados
- [CONFIGURACION.md](CONFIGURACION.md) - Detalles técnicos de compilación
- [ARQUITECTURA.md](ARQUITECTURA.md) - Cómo funciona el proyecto

### Herramientas Útiles

- **Visual Studio Code:** Editor de código ligero (alternativa a Visual Studio completo)
- **CMake GUI:** Versión con interfaz gráfica de CMake
- **Git Bash:** Terminal de Git (mejor que Command Prompt)

### Comunidades de Ayuda

- **Stack Overflow (Español):** https://es.stackoverflow.com/
- **Reddit r/opengl:** https://www.reddit.com/r/opengl/
- **OpenGL Discord:** Comunidad activa de desarrolladores

---

## Resumen Rápido

Para alguien que ya sabe lo básico:

```cmd
# 1. Instalar: Visual Studio 2022, CMake
# 2. Obtener proyecto
cd C:\Proyectos
git clone [repo-url] Cuadrupedo_IIIU
cd Cuadrupedo_IIIU

# 3. Configurar
cmake -B build -G "Visual Studio 17 2022"

# 4. Compilar
cmake --build build --config Release --parallel

# 5. Ejecutar
cd build\Release
Cuadrupedo_IIIU.exe
```

**Tiempo total estimado:** 1-2 horas (dependiendo de velocidad de internet y PC).

---

**¿Problemas no listados aquí?** Revisa [CONFIGURACION.md](CONFIGURACION.md) para troubleshooting más técnico o consulta la documentación completa.

---

**Última actualización:** Enero 2026
