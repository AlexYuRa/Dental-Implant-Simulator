/**
 * @file main.cpp
 * @brief Punto de entrada del programa (versión original)
 * 
 * Crea una instancia de Application y ejecuta el bucle principal.
 * Para usar la versión refactorizada POO, ver main_refactored.cpp.
 */

#include "Application.h"

int main() {
    Application app;

    if (!app.initialize()) {
        return -1;
    }

    app.run();

    return 0;
}
