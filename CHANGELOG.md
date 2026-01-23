# Changelog

Todos los cambios notables de este proyecto serán documentados en este archivo.

El formato está basado en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/),
y este proyecto adhiere a [Semantic Versioning](https://semver.org/lang/es/).

## [Unreleased]

### Por hacer
- Refactorizar Application.h en módulos más pequeños
- Agregar sistema de configuración externa (JSON)
- Implementar tests unitarios
- Optimizar rendimiento con frustum culling

---

## [1.0.0] - 2025-01-23

### Añadido
- Sistema de cámara orbital 3D con vistas médicas predefinidas
- Carga de modelos GLB/GLTF con ASSIMP
- Animación esquelética (hasta 200 huesos)
- Animación por keyframes para objetos interactivos
- Sistema de drag & drop para instrumental
- Renderizado PBR con materiales físicos
- Sistema de transformación de objetos (trasladar/rotar)
- HUD con indicadores de modo activo
- Múltiples modelos de consultorio dental:
  - Consultorio base
  - Instrumental quirúrgico (taladros, bisturís, pinzas)
  - Implantes dentales con cabezales
  - Mobiliario y decoración

### Características técnicas
- OpenGL 4.6 Core Profile
- C++17
- CMake build system
- Compatible con Visual Studio 2022

---

## Leyenda

- **Añadido** para nuevas funcionalidades
- **Cambiado** para cambios en funcionalidades existentes
- **Obsoleto** para funcionalidades que serán removidas próximamente
- **Eliminado** para funcionalidades eliminadas
- **Corregido** para corrección de bugs
- **Seguridad** para vulnerabilidades
