/**
 * @file Mesh.h
 * @brief Representación de una malla 3D con soporte para animación esquelética
 * 
 * Define las estructuras de datos para almacenar geometría 3D:
 * - Vertex: posición, normal, UV, IDs de huesos y pesos
 * - Texture: texturas diffuse, specular, normal, etc.
 * - Material: propiedades PBR (metallic, roughness)
 * - Mesh: conjunto de vértices, índices y texturas
 * 
 * La clase Mesh configura los VAO/VBO de OpenGL y dibuja
 * la geometría usando el shader proporcionado.
 * 
 * Usado por: Model.h para almacenar las mallas de un modelo
 */

#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include <string>
#include <vector>

// Estructura de vértice con soporte para skeletal animation
constexpr int MAX_BONE_INFLUENCE = 4;  // ← Revertido a 4 (ivec4/vec4 soportan 4)
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;

    // Datos de bones para skeletal animation
    glm::ivec4 BoneIDs;    // IDs de hasta 4 huesos que influyen en este vértice
    glm::vec4 Weights;     // Pesos de influencia de cada hueso (suman 1.0)
};

// Estructura de textura
struct Texture {
    unsigned int id;
    std::string type;    // diffuse, specular, normal, etc.
    std::string path;    // Para evitar cargar texturas duplicadas
};

// Estructura de material (colores del GLB)
struct Material {
    glm::vec3 baseColor;      // Color base del material
    float metallic;           // Factor metálico (0-1)
    float roughness;          // Rugosidad (0-1)
    float opacity;            // Opacidad/transparencia (0-1)
    
    Material() : baseColor(0.8f), metallic(0.0f), roughness(0.5f), opacity(1.0f) {}
};

class Mesh {
public:
    // Constructor con material
    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, 
            std::vector<Texture> textures, Material material = Material())
        : m_vertices(vertices), m_indices(indices), m_textures(textures), 
            m_material(material), m_VAO(0), m_VBO(0), m_EBO(0)
    {
        // Ahora que tenemos todos los datos necesarios, configurar los buffers
        setupMesh();
    }

    // Getters para acceso controlado
    const std::vector<Vertex>& getVertices() const { return m_vertices; }
    const std::vector<unsigned int>& getIndices() const { return m_indices; }
    const std::vector<Texture>& getTextures() const { return m_textures; }
    const Material& getMaterial() const { return m_material; }
    unsigned int getVAO() const { return m_VAO; }

    // Renderizar la malla
    void Draw(Shader &shader)
    {
        // Indicar al shader si este mesh tiene texturas
        bool hasTextures = !m_textures.empty();
        shader.setBool("hasTexture", hasTextures);
        
        // Enviar propiedades del material al shader
        shader.setVec3("material.baseColor", m_material.baseColor);
        shader.setFloat("material.metallic", m_material.metallic);
        shader.setFloat("material.roughness", m_material.roughness);
        shader.setFloat("material.opacity", m_material.opacity);
        
        // Vincular texturas apropiadas
        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr = 1;
        unsigned int heightNr = 1;

        for(unsigned int i = 0; i < m_textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i); // Activar la unidad de textura apropiada

            // Obtener el número de textura (difusa, especular, normal, height)
            std::string number;
            std::string name = m_textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);
            else if(name == "texture_normal")
                number = std::to_string(normalNr++);
            else if(name == "texture_height")
                number = std::to_string(heightNr++);

            // Setear el sampler al índice de textura correcto
            shader.setInt((name + number).c_str(), i);
            // Vincular la textura
            glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
        }

        // Dibujar la malla
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Desvincular texturas para evitar que otros modelos las hereden
        for(unsigned int i = 0; i < m_textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // Datos de la malla
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Texture> m_textures;
    Material m_material;

    // Datos de renderizado
    unsigned int m_VAO, m_VBO, m_EBO;

    // Inicializar todos los buffers y configurar atributos de vértices
    void setupMesh()
    {
        // Crear buffers/arrays
        glGenVertexArrays(1, &m_VAO);
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);

        glBindVertexArray(m_VAO);

        // Cargar datos en buffers
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

        // Configurar atributos de vértices

        // Posiciones de vértices
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

        // Normales de vértices
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

        // Coordenadas de textura de vértices
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        // IDs de bones (enteros)
        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));

        // Pesos de bones
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));

        glBindVertexArray(0);
    }
};

#endif
