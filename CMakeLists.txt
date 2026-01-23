cmake_minimum_required(VERSION 3.15)
project(Cuadrupedo_IIIU)

# Configurar C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Agregar directorio de includes
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/GLAD/include
    ${CMAKE_SOURCE_DIR}/external/Assimp/include
    ${CMAKE_SOURCE_DIR}/external/glm
)

# Agregar directorios de librerías
link_directories(
    ${CMAKE_SOURCE_DIR}/external/glfw/lib-vc2022
    ${CMAKE_SOURCE_DIR}/external/Assimp/lib/x64
)

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/stb_image.cpp
    src/InteractionSystem.cpp
    external/GLAD/src/glad.c
)

# Crear ejecutable
add_executable(${PROJECT_NAME} ${SOURCES})

# Linkear librerías
target_link_libraries(${PROJECT_NAME}
    glfw3
    assimp-vc143-mt
    opengl32
)

# Suprimir warning LNK4098 (es inofensivo, no afecta funcionalidad)
if(MSVC)
    target_link_options(${PROJECT_NAME} PRIVATE /ignore:4098)
endif()

# Copiar DLL y recursos al directorio de salida
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/Assimp/bin/x64/assimp-vc143-mt.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
