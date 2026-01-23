#version 460 core

// Atributos de entrada del vértice
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in ivec4 aBoneIDs;       // IDs de huesos (enteros)
layout (location = 4) in vec4 aWeights;        // Pesos de influencia

// Matrices de transformación
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// Matrices de bones para skeletal animation
const int MAX_BONES = 200;
uniform mat4 finalBonesMatrices[MAX_BONES];

// Salidas al fragment shader
out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    // Calcular transformación combinada de bones (skinning)
    mat4 boneTransform = mat4(0.0);

    for (int i = 0; i < 4; i++)
    {
        if (aBoneIDs[i] >= 0)  // Validar que el bone ID es válido
        {
            boneTransform += finalBonesMatrices[aBoneIDs[i]] * aWeights[i];
        }
    }

    // Si no hay bones válidos, usar matriz identidad
    if (boneTransform == mat4(0.0))
    {
        boneTransform = mat4(1.0);
    }

    // Aplicar deformación del esqueleto a la posición
    vec4 localPosition = boneTransform * vec4(aPos, 1.0);

    // Calcular posición final en espacio del mundo
    FragPos = vec3(model * localPosition);

    // Transformar normales considerando la deformación del esqueleto
    Normal = mat3(transpose(inverse(model * boneTransform))) * aNormal;

    // Pasar coordenadas de textura sin modificar
    TexCoords = aTexCoords;

    // Calcular posición final en clip space
    gl_Position = projection * view * model * localPosition;
}
