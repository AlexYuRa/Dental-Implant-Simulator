# Controles del Usuario

> 📝 **Nota:** Estos controles aplican tanto a la versión original como a la versión refactorizada POO.
> El procesamiento de inputs se encuentra en:
> - **Versión original:** `Application.h` (función `processInput()`)
> - **Versión POO:** `src/managers/InputManager.h`

## Referencia Rápida

### Controles de Cámara

| Acción | Control | Descripción |
|--------|---------|-------------|
| Rotar cámara | Clic derecho + Arrastrar | Orbita alrededor del modelo |
| Pan (desplazar) | Clic medio + Arrastrar | Mover vista lateralmente |
| Zoom in/out | Rueda del mouse | Acercar/alejar cámara |
| Mover objetivo | WASD | Mover en plano horizontal |
| Mover rápido | SHIFT + WASD | Mover 3x más rápido |
| Subir objetivo | E | Elevar punto de vista |
| Bajar objetivo | Q | Descender punto de vista |
| Resetear | R | Volver a posición inicial |

### Controles de Selección y Arrastre

| Acción | Control | Descripción |
|--------|---------|-------------|
| Seleccionar objeto | Clic izquierdo | Selecciona objeto arrastrable |
| Arrastrar objeto | Clic izquierdo + Mover | Mueve el objeto seleccionado |
| Deseleccionar | ESC | Deselecciona el objeto actual |

### Controles de Transformación (con objeto seleccionado)

| Acción | Control | Descripción |
|--------|---------|-------------|
| Modo Trasladar | 1 | Activa modo de traslación (HUD verde) |
| Modo Rotar | 2 | Activa modo de rotación (HUD azul) |
| Modo Escalar | 3 | Activa modo de escala (HUD amarillo) |
| Eje X | X | Restringe transformación a eje X |
| Eje Y | Y | Restringe transformación a eje Y |
| Eje Z | Z | Restringe transformación a eje Z |
| Todos los ejes | X/Y/Z (repetir) | Vuelve a modo sin restricción |
| Transformar + | Flecha Arriba/Derecha | Incrementa transformación |
| Transformar - | Flecha Abajo/Izquierda | Decrementa transformación |
| Subir/Bajar | PgUp/PgDn | Solo en modo traslación |
| Escalar + | + o = | Solo en modo escala |
| Escalar - | - | Solo en modo escala |
| Transformar rápido | SHIFT + Flechas | Transforma 3x más rápido |

### Controles de Aplicación

| Acción | Control | Descripción |
|--------|---------|-------------|
| Pausar/Reanudar animación | ESPACIO | Toggle de pausa en la animación |
| Salir | ESC (sin selección) | Cerrar aplicación |

## HUD (Heads-Up Display)

### Indicadores de Modo de Transformación
En la esquina superior izquierda aparecen barras de colores:

**Fila Superior - Modo Activo:**
- 🟩 **Verde brillante**: Modo TRASLADAR activo
- 🟦 **Azul brillante**: Modo ROTAR activo
- 🟨 **Amarillo brillante**: Modo ESCALAR activo
- ⬛ **Gris oscuro**: Modo inactivo

**Fila Inferior - Eje Restringido:**
- 🟥 **Rojo**: Eje X seleccionado
- 🟩 **Verde**: Eje Y seleccionado
- 🟦 **Azul**: Eje Z seleccionado
- ⬜ **Gris claro**: Todos los ejes (sin restricción)

## Controles de Cámara Detallados

### Rotación Orbital

**Control:** Clic derecho + Arrastrar mouse

**Funcionamiento:**
```
1. Mantener presionado clic derecho
2. Mover mouse horizontalmente → Rota alrededor del eje Y (yaw)
3. Mover mouse verticalmente → Rota alrededor del eje X (pitch)
4. Soltar clic derecho para detener
```

**Rango:**
- Yaw: 360° ilimitado
- Pitch: -89° a +89° (evita voltear cámara)

**Sensibilidad:** 0.1 (configurable en Camera.h)

**Ejemplo de uso:**
```
Mouse hacia la derecha → Vista rota a la izquierda
Mouse hacia arriba → Vista desde arriba
Mouse hacia abajo → Vista desde abajo
```

### Zoom

**Control:** Rueda del mouse (scroll)

**Funcionamiento:**
```
Scroll hacia arriba → Acercar (reduce distancia)
Scroll hacia abajo → Alejar (aumenta distancia)
```

**Rango:** 1.0 a 200.0 unidades

**Sensibilidad:** 0.5 unidades por click

**Usos típicos:**
- Ver detalles del modelo: Zoom in (distancia ~2-5)
- Vista general de escena: Zoom out (distancia ~20-50)
- Vista panorámica: Máximo zoom (distancia ~100-200)

### Movimiento del Target

#### WASD - Plano Horizontal

**W (Forward):**
```cpp
Mueve el target en dirección de la vista (plano XZ)
Velocidad: 2.5 unidades/segundo
```

**S (Backward):**
```cpp
Mueve el target opuesto a dirección de vista
```

**A (Left):**
```cpp
Mueve el target a la izquierda (perpendicular a vista)
```

**D (Right):**
```cpp
Mueve el target a la derecha
```

#### Q/E - Eje Vertical

**E (Up):**
```cpp
Eleva el target en eje Y (hacia arriba)
Útil para observar modelos desde diferentes alturas
```

**Q (Down):**
```cpp
Baja el target en eje Y (hacia abajo)
```

**Ejemplo de uso:**
```
1. W + D → Diagonal forward-right
2. E → Elevar vista
3. Zoom in → Ver modelo desde arriba
```

### Reset (R)

**Control:** Tecla R

**Restaura:**
- Target: (0, 0, 0)
- Yaw: -90° (mirando hacia -Z)
- Pitch: 0° (horizontal)
- Distancia: 5.0 unidades
- Ciclos de animación: 0

**Cuándo usar:**
- Perdiste de vista el modelo
- Quieres volver a la vista inicial
- Resetear la animación

## Controles de Animación

### Pausar/Reanudar Animación

**Control:** Tecla ESPACIO

**Funcionamiento:**
```
Presionar ESPACIO → Toggle entre pausado/reanudado
```

**Estados:**
- **Animación activa**: El cuadrúpedo se anima normalmente
- **Animación pausada**: El cuadrúpedo se congela en el frame actual

**Mensajes en consola:**
```
"Animacion PAUSADA"    → Al pausar
"Animacion REANUDADA"  → Al reanudar
```

**Uso típico:**
```
1. Ejecutar programa (animación activa)
2. Presionar ESPACIO → Pausar en un frame interesante
3. Rotar cámara para observar la pose desde diferentes ángulos
4. Presionar ESPACIO → Reanudar animación
```

**Notas:**
- La pausa NO afecta la escena estática (sigue rotando)
- La cámara sigue funcionando mientras está pausado
- El frame se mantiene exactamente donde se pausó

### Ajustar Distancia por Ciclo

**Aumentar (+/=):**
```cpp
Incrementa distancia que el modelo recorre por ciclo
Paso: 0.05 unidades
Sin límite superior
```

**Reducir (-/_):**
```cpp
Reduce distancia por ciclo
Paso: 0.05 unidades
Límite inferior: 0.0 (modelo no se mueve)
```

**Uso:**
```
Distancia = 0.0  → Modelo en su lugar (solo anima)
Distancia = 2.0  → Movimiento normal
Distancia = 10.0 → Movimiento rápido
```

## Combinaciones de Teclas

### Navegación Eficiente

**Vista Frontal:**
```
1. R (reset)
2. Scroll para ajustar zoom
```

**Vista Lateral:**
```
1. R (reset)
2. Clic derecho + arrastrar horizontalmente 90°
```

**Vista Superior:**
```
1. R (reset)
2. Clic derecho + arrastrar hacia arriba (pitch ~80°)
3. Zoom out para ver todo
```

**Seguir Animación:**
```
1. W (mantener presionado) → Cámara sigue al modelo
2. O usar + para aumentar velocidad de movimiento
```

## Configuración de Controles

### Modificar en Código

#### Sensibilidad de Mouse

```cpp
// Camera.h
static constexpr float SENSITIVITY = 0.1f;  // Rotación
static constexpr float ZOOM_SENSITIVITY = 0.5f;  // Zoom
```

**Valores recomendados:**
- Baja (0.05): Control preciso, ideal para detalles
- Media (0.1-0.2): Uso general
- Alta (0.3-0.5): Navegación rápida

#### Velocidad de Movimiento

```cpp
// Camera.h
static constexpr float MOVE_SPEED = 2.5f;
```

**Valores recomendados:**
- Lenta (1.0-2.0): Control fino
- Normal (2.5-5.0): Uso general
- Rápida (5.0-10.0): Navegación de escenas grandes

#### Rango de Zoom

```cpp
// Camera.h
static constexpr float MIN_DISTANCE = 1.0f;
static constexpr float MAX_DISTANCE = 200.0f;
```

#### Paso de Ajuste de Distancia

```cpp
// Application.h
static constexpr float DISTANCE_ADJUSTMENT_STEP = 0.05f;
```

### Remapear Teclas

Para cambiar teclas de movimiento, modificar en `Application.h`:

```cpp
void Application::processInput() {
    // Ejemplo: Cambiar WASD a teclas de flecha
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        // mover forward
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        // mover backward
    // etc.
}
```

## Atajos de Teclado GLFW

### Códigos de Teclas Útiles

```cpp
// Letras
GLFW_KEY_A, GLFW_KEY_B, ..., GLFW_KEY_Z

// Números
GLFW_KEY_0, GLFW_KEY_1, ..., GLFW_KEY_9

// Funcionales
GLFW_KEY_ESCAPE
GLFW_KEY_ENTER
GLFW_KEY_SPACE
GLFW_KEY_TAB

// Flechas
GLFW_KEY_UP
GLFW_KEY_DOWN
GLFW_KEY_LEFT
GLFW_KEY_RIGHT

// Modificadores
GLFW_KEY_LEFT_SHIFT
GLFW_KEY_LEFT_CONTROL
GLFW_KEY_LEFT_ALT

// Símbolos
GLFW_KEY_EQUAL       // +
GLFW_KEY_MINUS       // -
GLFW_KEY_KP_ADD      // + (teclado numérico)
GLFW_KEY_KP_SUBTRACT // - (teclado numérico)
```

## Tips y Trucos

### Navegación Eficiente

1. **Usar reset frecuentemente:**
   - R es tu amigo cuando te pierdes

2. **Combinar zoom y rotación:**
   - Zoom out → Rotar → Zoom in al área de interés

3. **Movimiento suave:**
   - Mantener teclas presionadas en lugar de pulsar repetidamente

4. **Exploración de modelo:**
   ```
   - Reset (R)
   - Zoom in moderado
   - Rotar lentamente 360° para ver todos los ángulos
   ```

### Captura de Vistas Específicas

**Vista de Referencia:**
```
1. R (reset)
2. Ajustar con rotación/zoom
3. Recordar valores (mostrados en consola si se implementa)
```

**Vista Cinematográfica:**
```
1. Zoom out bastante (distancia ~50-100)
2. Pitch ligeramente negativo (~-10 a -20°)
3. Rotar lentamente con clic derecho
```

## Accesibilidad

### Para Usuarios con Limitaciones

**Solo Mouse:**
- Clic derecho + arrastrar: Navegación completa posible
- Scroll: Zoom

**Solo Teclado:**
- WASD, Q, E: Movimiento completo del target
- R: Reset
- +/-: Ajuste de velocidad
- (Rotación requiere implementación adicional con teclas)

## Solución de Problemas

### La cámara no responde

**Síntomas:** Mouse no hace nada al arrastrar
**Causa:** No está presionado el botón derecho
**Solución:** Asegurarse de mantener clic derecho presionado

### Movimiento muy lento/rápido

**Síntomas:** Teclas WASD mueven muy poco/mucho
**Causa:** MOVE_SPEED inadecuado
**Solución:** Ajustar MOVE_SPEED en Camera.h

### Zoom insuficiente

**Síntomas:** No puedes alejar suficiente la cámara
**Causa:** MAX_DISTANCE muy pequeño
**Solución:** Ya configurado a 200.0f (suficiente para escenas grandes)

### Cámara se voltea

**Síntomas:** Vista se invierte al mirar muy arriba/abajo
**Causa:** Pitch fuera de rango
**Solución:** Ya limitado a ±89° (no debería ocurrir)

## Implementación de Nuevos Controles

### ✅ Pausa de Animación (Ya Implementado)

**Control:** Tecla ESPACIO

La pausa de animación ya está implementada en el proyecto. El código se encuentra en [Application.h](../src/Application.h):

```cpp
// Variables de estado (líneas 48-49)
bool m_animationPaused;        // Estado de pausa de la animación
bool m_spaceKeyPressed;        // Para evitar múltiples toggles

// En processInput() (líneas 232-241)
if (glfwGetKey(m_window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    if (!m_spaceKeyPressed) {
        m_animationPaused = !m_animationPaused;
        m_spaceKeyPressed = true;
        std::cout << (m_animationPaused ? "Animacion PAUSADA" : "Animacion REANUDADA") << std::endl;
    }
} else {
    m_spaceKeyPressed = false;
}

// En update() (líneas 258-261)
if (!m_animationPaused) {
    m_animator->UpdateAnimation(m_deltaTime);
}
```

**Características de la implementación:**
- Toggle suave: Evita múltiples activaciones con `m_spaceKeyPressed`
- Feedback en consola: Muestra mensaje cuando se pausa/reanuda
- No afecta otros sistemas: La escena y cámara siguen funcionando
- Mantiene el frame exacto donde se pausó

### Ejemplo: Cambiar Velocidad de Animación

```cpp
// En Application.h
private:
    float m_animationSpeed = 1.0f;

void processInput() {
    // Aumentar velocidad
    if (glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS)
        m_animationSpeed += 0.1f;

    // Reducir velocidad
    if (glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS)
        m_animationSpeed = std::max(0.1f, m_animationSpeed - 0.1f);
}

void update() {
    m_animator->UpdateAnimation(m_deltaTime * m_animationSpeed);
}
```

---

**Próximo:** [Referencia de Shaders](SHADERS.md)
