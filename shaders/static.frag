#version 460 core

// Salida del fragment shader
out vec4 FragColor;

// Entradas desde el vertex shader
in vec2 TexCoords;
in vec3 Normal;
in vec3 FragPos;

// Textura difusa del modelo
uniform sampler2D texture_diffuse1;

// Indica si el modelo tiene textura
uniform bool hasTexture;

// Propiedades del material (del GLB)
struct MaterialProperties {
    vec3 baseColor;
    float metallic;
    float roughness;
    float opacity;
};
uniform MaterialProperties material;

// Parámetros de iluminación
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

void main()
{
    vec3 objectColor;
    float alpha = material.opacity;
    
    // Usar textura si existe, sino usar color del material
    if (hasTexture) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        objectColor = texColor.rgb * material.baseColor;  // Multiplicar por color base
        alpha *= texColor.a;
    } else {
        // Usar color base del material
        objectColor = material.baseColor;
    }

    // 1. Componente Ambient (iluminación ambiente)
    float ambientStrength = 0.35;
    vec3 ambient = ambientStrength * lightColor;

    // 2. Componente Diffuse (iluminación difusa)
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // 3. Componente Especular (ajustado por metallic y roughness)
    float specularStrength = mix(0.3, 0.9, material.metallic);  // Más metálico = más brillo
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float shininess = mix(16.0, 128.0, 1.0 - material.roughness);  // Menos rugoso = más nítido
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // Para materiales metálicos, el especular toma el color del objeto
    if (material.metallic > 0.5) {
        specular *= objectColor;
    }

    // Combinar los tres componentes de iluminación
    vec3 result = (ambient + diffuse + specular) * objectColor;
    
    // Aplicar transparencia
    FragColor = vec4(result, alpha);
}
