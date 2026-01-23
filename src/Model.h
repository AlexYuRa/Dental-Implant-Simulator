/**
 * @file Model.h
 * @brief Cargador de modelos 3D usando ASSIMP
 * 
 * Carga modelos desde archivos GLB/GLTF/FBX/OBJ y los convierte
 * a estructuras utilizables por OpenGL. Soporta:
 * - Múltiples mallas (meshes)
 * - Texturas diffuse, specular, normal
 * - Materiales PBR (metallic, roughness, opacity)
 * - Esqueleto de huesos para animación
 * - Bounding boxes para colisiones
 * 
 * Proceso de carga:
 * 1. ASSIMP lee el archivo 3D
 * 2. Se procesan nodos recursivamente
 * 3. Se extraen mallas, materiales, texturas
 * 4. Se configura información de huesos
 * 
 * Depende de: Mesh.h, Shader.h, ASSIMP, stb_image
 */

#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "stb_image.h"
#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

// Función de utilidad para cargar texturas desde archivo
unsigned int TextureFromFile(const char* path, const std::string &directory, bool gamma = false);

// Estructura para almacenar información de huesos
struct BoneInfo
{
    int id;              // Índice del hueso en el array de bones
    glm::mat4 offset;    // Matriz de offset (inverse bind pose matrix)
};

// Función de conversión de matrices de ASSIMP a GLM
inline glm::mat4 ConvertMatrixToGLMFormat(const aiMatrix4x4& from)
{
    glm::mat4 to;
    // Assimp es row-major, GLM es column-major, así que transponemos
    to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
    to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
    to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
    to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
    return to;
}

class Model
{
public:
    // Constructor: carga el modelo desde un archivo
    Model(std::string const &path)
    {
        loadModel(path);
    }

    // Dibuja el modelo
    void Draw(Shader &shader)
    {
        for (unsigned int i = 0; i < m_meshes.size(); i++)
            m_meshes[i].Draw(shader);
    }

    // Getters para acceso controlado
    const std::vector<Mesh>& getMeshes() const { return m_meshes; }
    const std::string& getDirectory() const { return m_directory; }
    const std::vector<Texture>& getTexturesLoaded() const { return m_textures_loaded; }

    // Acceso a bone mapping (usado por Animation)
    auto& GetBoneInfoMap() { return m_BoneInfoMap; }
    int& GetBoneCount() { return m_BoneCounter; }

    // Acceso a la escena de Assimp (para NodeAnimation)
    const aiScene* GetScene() const { return m_Scene; }

    // Obtener la transformación del nodo raíz (posición/rotación/escala de Blender)
    const glm::mat4& GetRootTransform() const { return m_RootTransform; }

    // Extraer solo la posición del nodo raíz
    glm::vec3 GetRootPosition() const {
        return glm::vec3(m_RootTransform[3][0], m_RootTransform[3][1], m_RootTransform[3][2]);
    }

    // Helper: Busca un nodo por nombre en el árbol
    aiNode* FindNodeByName(aiNode* node, const std::string& name) const {
        if (!node) return nullptr;
        if (std::string(node->mName.C_Str()) == name) return node;
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            aiNode* found = FindNodeByName(node->mChildren[i], name);
            if (found) return found;
        }
        return nullptr;
    }

    // Helper: Obtiene la transformación acumulada desde la raíz hasta un nodo
    glm::mat4 GetNodeGlobalTransform(aiNode* node) const {
        glm::mat4 transform = ConvertMatrixToGLMFormat(node->mTransformation);
        aiNode* parent = node->mParent;
        while (parent) {
            transform = ConvertMatrixToGLMFormat(parent->mTransformation) * transform;
            parent = parent->mParent;
        }
        return transform;
    }

    // Extraer transformaciones desde los keyframes de animación (para objetos sin huesos)
    // Estructura para devolver keyframes con quaternion
    struct KeyframeData {
        glm::vec3 position;
        glm::quat rotation;
        glm::vec3 scale;
    };

    std::vector<KeyframeData> GetAnimationKeyframesQuat() const {
        std::vector<KeyframeData> keyframes;

        if (!m_Scene || m_Scene->mNumAnimations == 0) {
            std::cout << "[Model] No se encontraron animaciones en el modelo" << std::endl;
            return keyframes;
        }

        // Tomar la primera animación
        aiAnimation* animation = m_Scene->mAnimations[0];
        std::cout << "[Model] Animacion encontrada: " << animation->mName.C_Str()
                  << " (duracion: " << animation->mDuration << " ticks, "
                  << animation->mTicksPerSecond << " ticks/sec)" << std::endl;
        std::cout << "[Model] Canales de animacion: " << animation->mNumChannels << std::endl;

        // Buscar el canal de animación del nodo raíz o primer mesh
        for (unsigned int i = 0; i < animation->mNumChannels; i++) {
            aiNodeAnim* nodeAnim = animation->mChannels[i];
            std::string nodeName = nodeAnim->mNodeName.C_Str();
            std::cout << "[Model] Canal " << i << ": " << nodeName
                      << " (Pos keys: " << nodeAnim->mNumPositionKeys
                      << ", Rot keys: " << nodeAnim->mNumRotationKeys
                      << ", Scale keys: " << nodeAnim->mNumScalingKeys << ")" << std::endl;

            // Buscar el nodo para obtener la transformación del padre
            aiNode* animatedNode = FindNodeByName(m_Scene->mRootNode, nodeName);
            glm::mat4 parentTransform = glm::mat4(1.0f);
            if (animatedNode && animatedNode->mParent) {
                parentTransform = GetNodeGlobalTransform(animatedNode->mParent);
                std::cout << "[Model] Nodo padre encontrado, aplicando transformacion global" << std::endl;
            }

            // Extraer keyframes
            if (nodeAnim->mNumPositionKeys >= 2) {
                // Lambda para construir la matriz local del keyframe
                auto buildKeyframeMatrix = [](const aiVector3D& pos, const aiQuaternion& rot, const aiVector3D& scale) {
                    glm::mat4 mat = glm::mat4(1.0f);
                    mat = glm::translate(mat, glm::vec3(pos.x, pos.y, pos.z));
                    mat = mat * glm::mat4_cast(glm::quat(rot.w, rot.x, rot.y, rot.z));
                    mat = glm::scale(mat, glm::vec3(scale.x, scale.y, scale.z));
                    return mat;
                };

                // Lambda para extraer componentes de una matriz
                auto decomposeMatrix = [](const glm::mat4& mat, glm::vec3& pos, glm::quat& rot, glm::vec3& scale) {
                    pos = glm::vec3(mat[3]);
                    
                    scale.x = glm::length(glm::vec3(mat[0]));
                    scale.y = glm::length(glm::vec3(mat[1]));
                    scale.z = glm::length(glm::vec3(mat[2]));
                    
                    glm::mat3 rotMat(
                        glm::vec3(mat[0]) / scale.x,
                        glm::vec3(mat[1]) / scale.y,
                        glm::vec3(mat[2]) / scale.z
                    );
                    rot = glm::quat_cast(rotMat);
                };

                // Extraer TODOS los keyframes
                std::cout << "[Model] Extrayendo TODOS los " << nodeAnim->mNumPositionKeys << " keyframes..." << std::endl;

                for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++) {
                    // Obtener índices correspondientes
                    unsigned int rotIdx = std::min(k, nodeAnim->mNumRotationKeys - 1);
                    unsigned int scaleIdx = std::min(k, nodeAnim->mNumScalingKeys - 1);

                    // Construir matriz local del keyframe
                    glm::mat4 localMat = buildKeyframeMatrix(
                        nodeAnim->mPositionKeys[k].mValue,
                        nodeAnim->mRotationKeys[rotIdx].mValue,
                        nodeAnim->mScalingKeys[scaleIdx].mValue
                    );
                    glm::mat4 globalMat = parentTransform * localMat;

                    // Extraer componentes
                    KeyframeData kf;
                    decomposeMatrix(globalMat, kf.position, kf.rotation, kf.scale);
                    keyframes.push_back(kf);

                    // Mostrar info de este keyframe
                    double timeInTicks = nodeAnim->mPositionKeys[k].mTime;
                    float ticksPerFrame = animation->mTicksPerSecond / 24.0f;
                    int blenderFrame = (int)(timeInTicks / ticksPerFrame) + 1;

                    std::cout << "[Model]   Keyframe " << k << " @ " << timeInTicks
                              << " ticks (≈ frame " << blenderFrame << " en Blender)"
                              << " - Pos: (" << kf.position.x << ", " << kf.position.y << ", " << kf.position.z << ")" << std::endl;
                }

                std::cout << "[Model] Keyframes extraidos: " << keyframes.size() << " (transformacion global)" << std::endl;
            }
        }

        return keyframes;
    }

    std::vector<glm::mat4> GetAnimationKeyframes() const {
        std::vector<glm::mat4> keyframeTransforms;

        if (!m_Scene || m_Scene->mNumAnimations == 0) {
            std::cout << "[Model] No se encontraron animaciones en el modelo" << std::endl;
            return keyframeTransforms;
        }

        // Tomar la primera animación
        aiAnimation* animation = m_Scene->mAnimations[0];
        std::cout << "[Model] Animacion encontrada: " << animation->mName.C_Str()
                  << " (duracion: " << animation->mDuration << " ticks, "
                  << animation->mTicksPerSecond << " ticks/sec)" << std::endl;
        std::cout << "[Model] Canales de animacion: " << animation->mNumChannels << std::endl;

        // Buscar el canal de animación del nodo raíz o primer mesh
        for (unsigned int i = 0; i < animation->mNumChannels; i++) {
            aiNodeAnim* nodeAnim = animation->mChannels[i];
            std::cout << "[Model] Canal " << i << ": " << nodeAnim->mNodeName.C_Str()
                      << " (Pos keys: " << nodeAnim->mNumPositionKeys
                      << ", Rot keys: " << nodeAnim->mNumRotationKeys
                      << ", Scale keys: " << nodeAnim->mNumScalingKeys << ")" << std::endl;

            // Extraer TODOS los keyframes de este canal
            if (nodeAnim->mNumPositionKeys >= 1) {
                std::cout << "[Model] Extrayendo TODOS los " << nodeAnim->mNumPositionKeys << " keyframes..." << std::endl;
                std::cout << "[Model] Duracion animacion: " << animation->mDuration << " ticks" << std::endl;
                std::cout << "[Model] Ticks por segundo: " << animation->mTicksPerSecond << std::endl;

                // Calcular frames de Blender (asumiendo 24 fps por defecto)
                float ticksPerFrame = animation->mTicksPerSecond / 24.0f;
                std::cout << "[Model] Ticks por frame (24fps): " << ticksPerFrame << std::endl;

                // Iterar sobre todos los keyframes de posición
                for (unsigned int k = 0; k < nodeAnim->mNumPositionKeys; k++) {
                    double timeInTicks = nodeAnim->mPositionKeys[k].mTime;
                    int blenderFrame = (int)(timeInTicks / ticksPerFrame) + 1; // Blender frames empiezan en 1

                    // Obtener índices correspondientes en rotación y escala
                    unsigned int rotIdx = std::min(k, nodeAnim->mNumRotationKeys - 1);
                    unsigned int scaleIdx = std::min(k, nodeAnim->mNumScalingKeys - 1);

                    // Extraer posición
                    glm::vec3 pos(nodeAnim->mPositionKeys[k].mValue.x,
                                 nodeAnim->mPositionKeys[k].mValue.y,
                                 nodeAnim->mPositionKeys[k].mValue.z);

                    // Extraer rotación
                    glm::quat rot(nodeAnim->mRotationKeys[rotIdx].mValue.w,
                                 nodeAnim->mRotationKeys[rotIdx].mValue.x,
                                 nodeAnim->mRotationKeys[rotIdx].mValue.y,
                                 nodeAnim->mRotationKeys[rotIdx].mValue.z);

                    // Extraer escala
                    glm::vec3 scale(nodeAnim->mScalingKeys[scaleIdx].mValue.x,
                                   nodeAnim->mScalingKeys[scaleIdx].mValue.y,
                                   nodeAnim->mScalingKeys[scaleIdx].mValue.z);

                    // Construir matriz de transformación
                    glm::mat4 transform = glm::mat4(1.0f);
                    transform = glm::translate(transform, pos);
                    transform = transform * glm::mat4_cast(rot);
                    transform = glm::scale(transform, scale);
                    keyframeTransforms.push_back(transform);

                    std::cout << "[Model]   Keyframe " << k << " @ tiempo " << timeInTicks
                              << " ticks (≈ frame " << blenderFrame << " en Blender)"
                              << " - Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
                }

                std::cout << "[Model] Total keyframes extraidos: " << keyframeTransforms.size() << std::endl;
                std::cout << "[Model] Tiempo final: " << nodeAnim->mPositionKeys[nodeAnim->mNumPositionKeys - 1].mTime
                          << " de " << animation->mDuration << " (duracion total)" << std::endl;
            }
        }

        return keyframeTransforms;
    }

private:
    // Datos del modelo
    std::vector<Mesh> m_meshes;
    std::string m_directory;
    std::vector<Texture> m_textures_loaded;  // Evita cargar texturas duplicadas

    // Gestión de bones para skeletal animation
    std::map<std::string, BoneInfo> m_BoneInfoMap;
    int m_BoneCounter = 0;

    // Almacenar el Importer y escena para acceder a texturas embebidas
    Assimp::Importer m_Importer;
    const aiScene* m_Scene = nullptr;

    // Transformación del nodo raíz (posición/rotación/escala de Blender)
    glm::mat4 m_RootTransform = glm::mat4(1.0f);

    // Carga el modelo usando Assimp
    void loadModel(std::string const &path)
    {
        const aiScene* scene = m_Importer.ReadFile(path,
            aiProcess_Triangulate |
            aiProcess_FlipUVs |
            aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            std::cout << "ERROR::ASSIMP::" << m_Importer.GetErrorString() << std::endl;
            return;
        }

        // Guardar referencia a la escena para texturas embebidas
        m_Scene = scene;

        // Extraer transformación del nodo raíz (posición/rotación/escala de Blender)
        m_RootTransform = ConvertMatrixToGLMFormat(scene->mRootNode->mTransformation);

        // Debug: mostrar si el modelo tiene transformación no trivial
        glm::vec3 rootPos = GetRootPosition();
        if (glm::length(rootPos) > 0.001f) {  // Si no está en origen
            std::cout << "Modelo tiene transformación de raíz: pos("
                     << rootPos.x << ", " << rootPos.y << ", " << rootPos.z << ")" << std::endl;
        }

        m_directory = path.substr(0, path.find_last_of('/'));
        if (m_directory == path)  // Si no hay '/', probar con '\'
            m_directory = path.substr(0, path.find_last_of('\\'));

        processNode(scene->mRootNode, scene);
    }

    // Procesa nodos del árbol jerárquico del modelo recursivamente
    void processNode(aiNode *node, const aiScene *scene)
    {
        // Procesa todas las mallas del nodo
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            m_meshes.push_back(processMesh(mesh, scene));
        }
        // Procesa los hijos del nodo (recursivo)
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }
    }

    // Procesa una malla individual
    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;

        // Vértices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            // Inicializar bone data a valores por defecto
            SetVertexBoneDataToDefault(vertex);

            glm::vec3 vector;

            // Posición
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // Normales
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }

            // Coordenadas de textura
            if (mesh->mTextureCoords[0])
            {
                glm::vec2 tex;
                tex.x = mesh->mTextureCoords[0][i].x;
                tex.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = tex;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }

        // Índices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // Material con valores por defecto
        Material meshMaterial;

        // Materiales y texturas
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

            // Extraer color base del material (GLTF/GLB)
            aiColor4D baseColor;
            if (AI_SUCCESS == material->Get(AI_MATKEY_BASE_COLOR, baseColor)) {
                meshMaterial.baseColor = glm::vec3(baseColor.r, baseColor.g, baseColor.b);
                meshMaterial.opacity = baseColor.a;
            } else if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor)) {
                // Fallback a color difuso tradicional
                meshMaterial.baseColor = glm::vec3(baseColor.r, baseColor.g, baseColor.b);
                meshMaterial.opacity = baseColor.a;
            }

            // Extraer factor metálico
            float metallic = 0.0f;
            if (AI_SUCCESS == material->Get(AI_MATKEY_METALLIC_FACTOR, metallic)) {
                meshMaterial.metallic = metallic;
            }

            // Extraer rugosidad
            float roughness = 0.5f;
            if (AI_SUCCESS == material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness)) {
                meshMaterial.roughness = roughness;
            }

            // Extraer opacidad adicional si existe
            float opacity = 1.0f;
            if (AI_SUCCESS == material->Get(AI_MATKEY_OPACITY, opacity)) {
                meshMaterial.opacity *= opacity;
            }

            std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
            textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
            
            // También buscar texturas base color (GLTF)
            std::vector<Texture> baseColorMaps = loadMaterialTextures(material, aiTextureType_BASE_COLOR, "texture_diffuse");
            textures.insert(textures.end(), baseColorMaps.begin(), baseColorMaps.end());

            std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
            textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        }

        // Extraer bone weights de la malla
        ExtractBoneWeightForVertices(vertices, mesh, scene);

        // DEBUG: Verificar que los vértices tienen bone weights
        int verticesWithBones = 0;
        for (const auto& vertex : vertices) {
            if (vertex.BoneIDs[0] >= 0) verticesWithBones++;
        }
        if (mesh->mNumBones > 0) {
            std::cout << "  Malla con " << mesh->mNumBones << " bones, "
                    << verticesWithBones << "/" << vertices.size()
                    << " vértices tienen bone weights" << std::endl;
        }

        return Mesh(vertices, indices, textures, meshMaterial);
    }

    // Inicializa bone data de un vértice a valores por defecto
    void SetVertexBoneDataToDefault(Vertex& vertex)
    {
        // Inicializar todos los BoneIDs a -1 (no influenciado por ningún hueso)
        vertex.BoneIDs = glm::ivec4(-1);
        // Inicializar todos los pesos a 0.0
        vertex.Weights = glm::vec4(0.0f);
    }

    // Asigna un bone ID y peso a un vértice
    void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
    {
        // Encontrar un slot libre en los 4 slots disponibles
        for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
        {
            if (vertex.BoneIDs[i] < 0)
            {
                vertex.Weights[i] = weight;
                vertex.BoneIDs[i] = boneID;
                break;
            }
        }
    }

    // Extrae información de bones y sus pesos de la malla
    void ExtractBoneWeightForVertices(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene)
    {
        // Iterar sobre todos los bones de la malla
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
        {
            int boneID = -1;
            std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

            // Si el bone no existe en nuestro mapa, agregarlo
            if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end())
            {
                BoneInfo newBoneInfo;
                newBoneInfo.id = m_BoneCounter;
                newBoneInfo.offset = ConvertMatrixToGLMFormat(mesh->mBones[boneIndex]->mOffsetMatrix);
                m_BoneInfoMap[boneName] = newBoneInfo;
                boneID = m_BoneCounter;
                m_BoneCounter++;
            }
            else
            {
                boneID = m_BoneInfoMap[boneName].id;
            }
            assert(boneID != -1);

            // Obtener los pesos de este bone para cada vértice afectado
            auto weights = mesh->mBones[boneIndex]->mWeights;
            int numWeights = mesh->mBones[boneIndex]->mNumWeights;

            for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
            {
                int vertexId = weights[weightIndex].mVertexId;
                float weight = weights[weightIndex].mWeight;
                assert(vertexId <= vertices.size());
                SetVertexBoneData(vertices[vertexId], boneID, weight);
            }
        }
    }

    // Carga las texturas de un material
    std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
    {
        std::vector<Texture> textures;
        std::cout << "Buscando texturas de tipo: " << typeName << " (count: " << mat->GetTextureCount(type) << ")" << std::endl;

        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            std::cout << "  Textura path: " << str.C_Str() << std::endl;

            bool skip = false;

            for (unsigned int j = 0; j < m_textures_loaded.size(); j++)
            {
                if (std::strcmp(m_textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(m_textures_loaded[j]);
                    skip = true;
                    break;
                }
            }

            if (!skip)
            {
                Texture texture;

                // Verificar si es textura embebida (path empieza con '*')
                std::string texPath = str.C_Str();
                if (texPath[0] == '*') {
                    // Es textura embebida
                    int textureIndex = std::atoi(&texPath[1]);
                    std::cout << "  Textura embebida detectada, indice: " << textureIndex << std::endl;
                    texture.id = LoadEmbeddedTexture(m_Scene->mTextures[textureIndex]);
                } else {
                    // Es textura externa
                    texture.id = TextureFromFile(str.C_Str(), m_directory);
                }

                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                m_textures_loaded.push_back(texture);
                std::cout << "  Textura cargada con ID: " << texture.id << std::endl;
            }
        }
        return textures;
    }

    // Cargar textura embebida desde ASSIMP
    unsigned int LoadEmbeddedTexture(const aiTexture* embeddedTexture)
    {
        unsigned int textureID;
        glGenTextures(1, &textureID);

        int width, height, nrComponents;
        unsigned char* data = nullptr;

        if (embeddedTexture->mHeight == 0) {
            // Textura comprimida (PNG, JPG, etc.)
            std::cout << "    Cargando textura comprimida (formato: " << embeddedTexture->achFormatHint << ")" << std::endl;
            data = stbi_load_from_memory(
                reinterpret_cast<unsigned char*>(embeddedTexture->pcData),
                embeddedTexture->mWidth,
                &width, &height, &nrComponents, 0
            );
        } else {
            // Textura no comprimida (raw ARGB8888)
            std::cout << "    Cargando textura raw ARGB" << std::endl;
            width = embeddedTexture->mWidth;
            height = embeddedTexture->mHeight;
            nrComponents = 4;
            data = reinterpret_cast<unsigned char*>(embeddedTexture->pcData);
        }

        if (data) {
            GLenum format = GL_RGBA; // Valor por defecto
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;
            else {
                std::cout << "    ERROR: Número de componentes no soportado: " << nrComponents << std::endl;
                stbi_image_free(data);
                return textureID;
            }

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Solo liberar si es textura comprimida
            if (embeddedTexture->mHeight == 0) {
                stbi_image_free(data);
            }

            std::cout << "    Textura embebida cargada exitosamente (" << width << "x" << height << ")" << std::endl;
        } else {
            std::cout << "    ERROR: Fallo al cargar textura embebida" << std::endl;
        }

        return textureID;
    }
};

// Función auxiliar para cargar texturas desde archivo
inline unsigned int TextureFromFile(const char* path, const std::string &directory, bool gamma)
{
    std::string filename = std::string(path);
    filename = directory + '/' + filename;

    // Si no funciona con '/', intentar con '\'
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RGBA; // Valor por defecto
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else {
            std::cout << "ERROR: Número de componentes no soportado en " << filename << ": " << nrComponents << std::endl;
            stbi_image_free(data);
            return textureID;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Fallo al cargar textura: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

#endif
