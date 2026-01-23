# Arquitectura POO Refactorizada

> ⚠️ **NOTA:** Estos archivos han sido movidos a `CopySecurity/refactored_poo/` 
> porque actualmente el proyecto usa la versión original (`Application.h`).
> 
> Si deseas usar la versión refactorizada:
> 1. Copia los archivos de vuelta a `src/`
> 2. Cambia `main.cpp` por `main_refactored.cpp` en CMakeLists.txt

Este documento describe la arquitectura orientada a objetos que fue implementada.

## 📁 Ubicación Actual de Archivos

```
CopySecurity/refactored_poo/
├── interfaces/              # Interfaces abstractas
│   ├── IRenderable.h       # Contrato para objetos renderizables
│   ├── ITransformable.h    # Contrato para objetos transformables
│   └── IUpdatable.h        # Contrato para objetos actualizables
│
├── core/                    # Clases núcleo
│   ├── TransformTypes.h    # Enums (Mode, Axis, Space)
│   ├── Keyframe.h          # Estructuras de keyframes
│   ├── InputState.h        # Estado centralizado de input
│   ├── SceneObject.h       # Clase base para objetos de escena
│   ├── SelectionManager.h  # Gestión de selección
│   └── TransformController.h # Control de transformaciones
│
├── managers/                # Managers de sistemas
│   ├── SceneManager.h      # Gestión de escena completa
│   ├── RenderManager.h     # Sistema de renderizado
│   └── InputManager.h      # Gestión de input
│
├── ApplicationRefactored.h  # Application usando POO
├── main_refactored.cpp      # Punto de entrada alternativo
└── NodeAnimation_Example.h  # Ejemplo de uso (documentación)
```

## 🎯 Principios POO Aplicados

### 1. **Principio de Responsabilidad Única (SRP)**
Cada clase tiene una sola razón para cambiar:
- `SceneManager`: Solo gestiona objetos de escena
- `RenderManager`: Solo renderiza
- `InputManager`: Solo procesa input
- `SelectionManager`: Solo maneja selección

### 2. **Principio Abierto/Cerrado (OCP)**
Las clases están abiertas a extensión pero cerradas a modificación:
```cpp
// Crear nuevos tipos de objetos sin modificar existentes
class MyCustomObject : public SceneObject {
    // Nueva funcionalidad
};
```

### 3. **Principio de Sustitución de Liskov (LSP)**
Los subtipos pueden sustituir a sus tipos base:
```cpp
SceneObject* obj = new KeyframeSceneObject("model.glb");
obj->Draw(shader);  // Funciona polimórficamente
```

### 4. **Principio de Segregación de Interfaces (ISP)**
Interfaces pequeñas y específicas:
- `IRenderable`: Solo método `Draw()`
- `ITransformable`: Solo transformaciones
- `IUpdatable`: Solo `Update()`

### 5. **Principio de Inversión de Dependencias (DIP)**
Dependencias hacia abstracciones:
```cpp
void RenderAll(std::vector<IRenderable*>& objects) {
    for (auto* obj : objects) {
        obj->Draw(shader);  // No depende de implementación
    }
}
```

## 🏗️ Diagrama de Clases

```
                    ┌─────────────────┐
                    │  IRenderable    │
                    │  (interface)    │
                    └────────┬────────┘
                             │
                    ┌────────┴────────┐
                    │  ITransformable │
                    │  (interface)    │
                    └────────┬────────┘
                             │
                    ┌────────┴────────┐
                    │  IUpdatable     │
                    │  (interface)    │
                    └────────┬────────┘
                             │
              ┌──────────────┼──────────────┐
              ▼              ▼              ▼
    ┌─────────────┐ ┌───────────────┐ ┌──────────────┐
    │ SceneObject │ │AnimatedScene  │ │ KeyframeScene│
    │   (base)    │ │   Object      │ │    Object    │
    └─────────────┘ └───────────────┘ └──────────────┘
```

## 📋 Uso de los Nuevos Componentes

### SceneManager
```cpp
SceneManager scene;

// Agregar objetos
scene.AddStaticObject("model.glb", "MiObjeto");
scene.AddKeyframeObject("animated.glb", "Animado");

// Buscar
SceneObject* obj = scene.FindByName("MiObjeto");

// Actualizar y renderizar
scene.Update(deltaTime);
scene.DrawStatic(shader);
```

### SelectionManager
```cpp
SelectionManager selection;

// Seleccionar
selection.Select(objeto);

// Cambiar modo
selection.SetTransformMode(Transform::Mode::TRANSLATE);
selection.ToggleTransformAxis(Transform::Axis::X);

// Verificar
if (selection.HasSelection()) {
    auto* obj = selection.GetSelectedObject();
}
```

### TransformController
```cpp
SelectionManager selection;
TransformController transform(&selection);

// En el loop de input
transform.ProcessModeInput(inputState);
transform.ProcessInput(inputState, deltaTime);
```

### InputState
```cpp
InputState input;

// Actualizar cada frame
input.Update(window);

// Verificar estados
if (input.IsKeyPressed(GLFW_KEY_SPACE)) {
    // Acaba de presionarse (no mantenido)
}

if (input.IsKeyHeld(GLFW_KEY_W)) {
    // Está siendo mantenido
}
```

## 🔄 Migración desde Application Original

La versión original (`Application.h`) sigue funcionando. Para usar la versión refactorizada:

1. En `main.cpp`, cambiar:
```cpp
// Antes
#include "Application.h"
Application app;

// Después
#include "ApplicationRefactored.h"
ApplicationRefactored app;
```

2. O usar el nuevo `main_refactored.cpp`

## ✅ Beneficios de la Refactorización

| Aspecto | Antes | Después |
|---------|-------|---------|
| Líneas en Application | ~2000 | ~400 |
| Responsabilidades | Múltiples | Una por clase |
| Testabilidad | Difícil | Fácil (mocks) |
| Extensibilidad | Modificar Application | Crear subclases |
| Reutilización | Mínima | Alta |
| Mantenibilidad | Compleja | Simple |
