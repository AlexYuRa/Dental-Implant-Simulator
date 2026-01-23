# Guía de Contribución

¡Gracias por tu interés en contribuir a Dental Implant Simulator! 🦷

## 📋 Tabla de Contenidos

- [Código de Conducta](#código-de-conducta)
- [¿Cómo puedo contribuir?](#cómo-puedo-contribuir)
- [Configuración del entorno](#configuración-del-entorno)
- [Guía de estilo](#guía-de-estilo)
- [Proceso de Pull Request](#proceso-de-pull-request)

## 📜 Código de Conducta

Este proyecto se adhiere a un código de conducta. Al participar, se espera que respetes este código. Por favor, reporta comportamiento inaceptable.

## 🤝 ¿Cómo puedo contribuir?

### Reportando Bugs

Antes de crear un reporte de bug:
1. Verifica que el bug no haya sido reportado anteriormente
2. Incluye información detallada:
   - Sistema operativo y versión
   - Versión de GPU y drivers
   - Pasos para reproducir el problema
   - Comportamiento esperado vs actual
   - Screenshots si es posible

### Sugiriendo Mejoras

Las mejoras pueden ser:
- Nuevas características
- Mejoras de rendimiento
- Mejoras de UX/UI
- Documentación

### Pull Requests

1. Fork el repositorio
2. Crea tu rama de feature (`git checkout -b feature/AmazingFeature`)
3. Haz commit de tus cambios (`git commit -m 'Add AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abre un Pull Request

## 🛠️ Configuración del entorno

### Requisitos

- Windows 10/11
- Visual Studio 2022 con workload "Desktop development with C++"
- CMake 3.15+
- Git

### Compilar el proyecto

```powershell
git clone https://github.com/tu-usuario/dental-implant-simulator.git
cd dental-implant-simulator
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

## 📏 Guía de estilo

### C++

```cpp
// Clases: PascalCase
class MyClass {
public:
    // Métodos: camelCase
    void doSomething();
    
private:
    // Miembros privados: prefijo m_
    int m_memberVariable;
    
    // Constantes: UPPER_SNAKE_CASE
    static constexpr int MAX_VALUE = 100;
};

// Funciones libres: camelCase
void myFunction();

// Variables locales: camelCase
int localVariable = 42;
```

### Comentarios

```cpp
/**
 * @brief Descripción breve de la función
 * 
 * Descripción detallada si es necesario.
 * 
 * @param param1 Descripción del parámetro
 * @return Descripción del valor de retorno
 */
int myFunction(int param1);

// Comentarios de una línea para explicaciones breves
```

### Shaders (GLSL)

```glsl
#version 460 core

// Atributos con layout explícito
layout (location = 0) in vec3 aPosition;

// Uniforms agrupados y documentados
uniform mat4 model;      // Matriz de modelo
uniform mat4 view;       // Matriz de vista
uniform mat4 projection; // Matriz de proyección
```

## 🔀 Proceso de Pull Request

1. **Asegúrate de que compila** sin warnings
2. **Prueba la funcionalidad** en modo Debug y Release
3. **Actualiza la documentación** si es necesario
4. **Describe los cambios** claramente en el PR

### Checklist del PR

- [ ] El código compila sin errores ni warnings
- [ ] He probado los cambios localmente
- [ ] He actualizado la documentación si es necesario
- [ ] Mi código sigue la guía de estilo del proyecto
- [ ] He agregado comentarios donde el código no es obvio

## 🏷️ Versionado

Usamos [SemVer](https://semver.org/) para versionado:
- **MAJOR**: Cambios incompatibles con versiones anteriores
- **MINOR**: Nueva funcionalidad compatible hacia atrás
- **PATCH**: Correcciones de bugs compatibles

## 📞 Contacto

Si tienes preguntas, abre un Issue en GitHub.

¡Gracias por contribuir! 🎉
