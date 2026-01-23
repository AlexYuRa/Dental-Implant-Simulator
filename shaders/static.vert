#version 460 core

// Atributos de entrada del vértice
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

// Matrices de transformación
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Salidas al fragment shader
out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    // Calcular posición en espacio del mundo
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transformar normales
    Normal = mat3(transpose(inverse(model))) * aNormal;

    // Pasar coordenadas de textura
    TexCoords = aTexCoords;

    // Calcular posición final
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
