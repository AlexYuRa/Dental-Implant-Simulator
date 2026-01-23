# Modelos y Texturas

> 📝 **Nota:** Los archivos `Model.h` y `Mesh.h` son compartidos por ambas versiones del proyecto.
> En la versión POO, los modelos se encapsulan dentro de `SceneObject` (ver `src/core/SceneObject.h`).

## Introducción: Del Arte 3D al Programa

**¿Cómo llega un modelo desde Blender a tu programa?**

1. **Artista 3D** crea el modelo en Blender (con materiales PBR)
2. **Exporta** como archivo GLB (incluye geometría, texturas, materiales, animaciones, huesos)
3. **ASSIMP** (biblioteca) lee el archivo GLB y lo convierte a estructuras de datos C++
4. **Tu programa** carga esas estructuras en la GPU para renderizar

**El Viaje de un Modelo:**
```
Blender (.blend) → Exportar → GLB File → ASSIMP → Estructuras C++ → GPU → Pantalla
                                            ↓
                              Extrae: Geometría, Texturas, Materiales PBR
```

**¿Por qué GLB y no otro formato?**
- **OBJ**: Solo geometría básica, no soporta animaciones ni materiales PBR
- **FBX**: Propietario de Autodesk, formato cerrado
- **GLB**: Estándar abierto, todo incluido, soporta PBR, perfecto para OpenGL

---

## Modelos del Simulador Dental

El proyecto carga **72+ modelos** de instrumental dental desde `assets/models/`:

### Categorías de Modelos

| Categoría | Ejemplos | Arrastrable | Descripción |
|-----------|----------|-------------|-------------|
| **Implantes** | CABEZAL-IMPLANTE10-12.glb | ✅ | Cabezales de implante dental |
| **Tijeras** | tijera1.001.glb, tijera2.001.glb | ✅ | Tijeras quirúrgicas |
| **Fórceps** | forcet_1_(creo).001.glb | ✅ | Pinzas de extracción |
| **Alicates** | alicate1.002.glb | ✅ | Herramientas de agarre |
| **Agujas** | aguja1.001.glb | ✅ | Instrumentos de sutura |
| **Bandejas** | Bandeja1.001.glb | ✅ | Contenedores de instrumental |
| **Anatomía** | DENTADURA ARRIBA.glb, CUERPO.glb | ❌ | Modelos del paciente |
| **Mobiliario** | MESA DE BOTELLAS.glb, Soporte.glb | ❌ | Equipamiento del consultorio |

### Carga de Modelos en Application.h

```cpp
// Modelos arrastrables (instrumental)
app.addStaticModel("assets/models/CABEZAL-IMPLANTE10-12.glb", pos, rot, scale, true);
app.addStaticModel("assets/models/tijera1.001.glb", pos, rot, scale, true);

// Modelos no arrastrables (escena)
app.addStaticModel("assets/models/CUERPO.glb", pos, rot, scale, false);
app.addStaticModel("assets/models/DENTADURA ARRIBA.glb", pos, rot, scale, false);
```

---

## Sistema de Materiales PBR

### Extracción de Propiedades desde GLB

El sistema extrae automáticamente propiedades PBR de los materiales definidos en Blender:

```cpp
// En Model.h - processMesh()
Material meshMaterial;

// Color base (albedo)
aiColor4D baseColor;
if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS) {
    meshMaterial.baseColor = glm::vec3(baseColor.r, baseColor.g, baseColor.b);
}

// Factor metálico
float metallic;
if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
    meshMaterial.metallic = metallic;
}

// Rugosidad
float roughness;
if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
    meshMaterial.roughness = roughness;
}

// Opacidad
meshMaterial.opacity = baseColor.a;
```

### Estructura Material

```cpp
struct Material {
    glm::vec3 baseColor = glm::vec3(0.8f);  // Gris por defecto
    float metallic = 0.0f;                   // No metálico
    float roughness = 0.5f;                  // Semi-rugoso
    float opacity = 1.0f;                    // Opaco
};
```

### Uso en Shader

```glsl
// Si tiene textura: combinar con color base
// Si no tiene textura: usar solo color base
if (hasTexture) {
    objectColor = texture(texture_diffuse1, TexCoords).rgb * material.baseColor;
} else {
    objectColor = material.baseColor;
}
```

---

## Formato GLB (GL Transmission Format Binary)

### ¿Qué es GLB?

GLB es la versión binaria del formato glTF 2.0, diseñado para transmisión eficiente de escenas 3D.

**Piensa en GLB como un archivo ZIP:**
- Contiene todo lo necesario en un solo archivo
- Está comprimido (binario vs texto)
- Mantiene las relaciones entre componentes

**Ventajas:**
- Todo en un solo archivo (geometría, texturas, animaciones)
- Compacto y eficiente
- Estándar de la industria
- Excelente soporte en herramientas 3D

**Estructura:**
```
GLB File
├── JSON Chunk (metadata, escena, nodes, meshes)
├── Binary Chunk (vertex data, indices)
└── Images (texturas embebidas)
```

## Carga de Modelos con ASSIMP

**¿Qué es ASSIMP?**

ASSIMP (Open Asset Import Library) es como un "traductor universal" de modelos 3D. Puede leer más de 40 formatos diferentes y convertirlos a una estructura común que tu programa puede entender.

**¿Por qué no leer GLB directamente?**
- El formato GLB es complejo (JSON + binario + imágenes)
- ASSIMP hace todo el trabajo sucio por ti
- Automáticamente maneja diferencias entre formatos
- Optimiza y procesa los datos

**ASSIMP actúa como intermediario:**
```
Archivo GLB → ASSIMP lee y parsea → Estructuras aiScene, aiMesh, aiNode → Tu código extrae datos → Estructuras propias (Model, Mesh, Vertex)
```

---

### Proceso de Importación

```cpp
class Model {
public:
    Model(string const &path) {
        loadModel(path);
    }

private:
    void loadModel(string const &path) {
        // 1. Importar con ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_Triangulate |           // Convertir a triángulos
            aiProcess_GenSmoothNormals |      // Generar normales
            aiProcess_FlipUVs |               // Voltear UVs (OpenGL)
            aiProcess_CalcTangentSpace        // Calcular tangentes
        );

        // 2. Verificar errores
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE
            || !scene->mRootNode) {
            cout << "ERROR::ASSIMP::" << importer.GetErrorString();
            return;
        }

        // 3. Extraer directorio para texturas
        m_directory = path.substr(0, path.find_last_of('/'));

        // 4. Procesar jerarquía de nodos
        processNode(scene->mRootNode, scene);
    }
};
```

### Flags de Procesamiento ASSIMP

**¿Qué hacen estos flags?**

ASSIMP puede hacer procesamiento automático mientras carga el modelo. Es como tener un asistente que limpia y optimiza tus datos.

```cpp
aiProcess_Triangulate          // Convierte polígonos a triángulos
aiProcess_GenSmoothNormals     // Genera normales suaves
aiProcess_FlipUVs              // Voltea coordenadas UV (Y-up a OpenGL)
aiProcess_CalcTangentSpace     // Calcula tangentes para normal mapping
aiProcess_JoinIdenticalVertices // Optimiza vértices duplicados
```

**Explicación detallada:**

1. **aiProcess_Triangulate** - *Muy importante*
   - **Problema**: Los artistas pueden usar cuadrados, pentágonos, etc.
   - **Solución**: OpenGL solo entiende triángulos
   - **Acción**: Convierte todos los polígonos a triángulos automáticamente

2. **aiProcess_GenSmoothNormals** - *Para iluminación*
   - **Qué son normales**: Vectores perpendiculares a la superficie (indican "hacia dónde mira")
   - **Para qué**: Calcular iluminación correctamente
   - **Acción**: Genera normales suavizadas en las aristas para que no se vean facetadas

3. **aiProcess_FlipUVs** - *Coordenadas de textura*
   - **Problema**: Blender usa Y hacia arriba, OpenGL usa Y hacia abajo
   - **Resultado sin flip**: Texturas aparecen al revés verticalmente
   - **Acción**: Voltea las coordenadas UV para OpenGL

4. **aiProcess_CalcTangentSpace** - *Para normal mapping avanzado*
   - **Uso**: Si quisieras usar normal maps (texturas que simulan detalles 3D)
   - **Acción**: Calcula vectores tangente y bitangente necesarios

5. **aiProcess_JoinIdenticalVertices** - *Optimización*
   - **Problema**: El mismo vértice puede estar duplicado
   - **Acción**: Detecta y elimina duplicados, usa índices
   - **Beneficio**: Menos memoria, más rápido

## Procesamiento de Nodos

### Jerarquía de Escena

```
Scene Root
├── Node 1 (Mesh A, Mesh B)
│   ├── Child Node 1.1 (Mesh C)
│   └── Child Node 1.2
└── Node 2 (Mesh D)
    └── Child Node 2.1 (Mesh E, Mesh F)
```

### processNode (Recursivo)

```cpp
void Model::processNode(aiNode *node, const aiScene *scene) {
    // 1. Procesar todas las mallas del nodo
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(processMesh(mesh, scene));
    }

    // 2. Recursión: procesar nodos hijos
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}
```

## Procesamiento de Mallas

### processMesh

```cpp
Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // 1. PROCESAR VÉRTICES
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        // Posición
        vertex.Position = glm::vec3(
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        );

        // Normal
        if (mesh->HasNormals()) {
            vertex.Normal = glm::vec3(
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            );
        }

        // Coordenadas de textura
        if (mesh->mTextureCoords[0]) {
            vertex.TexCoords = glm::vec2(
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            );
        } else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        // Inicializar bone data por defecto
        SetVertexBoneDataToDefault(vertex);

        vertices.push_back(vertex);
    }

    // 2. PROCESAR ÍNDICES
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 3. PROCESAR MATERIAL Y TEXTURAS
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Texturas difusas
    vector<Texture> diffuseMaps = loadMaterialTextures(
        material, aiTextureType_DIFFUSE, "texture_diffuse", scene
    );
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // Texturas especulares
    vector<Texture> specularMaps = loadMaterialTextures(
        material, aiTextureType_SPECULAR, "texture_specular", scene
    );
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // 4. PROCESAR BONES (para modelos animados)
    ExtractBoneWeightForVertices(vertices, mesh, scene);

    return Mesh(vertices, indices, textures);
}
```

## Sistema de Bones

### SetVertexBoneDataToDefault

```cpp
void Model::SetVertexBoneDataToDefault(Vertex& vertex) {
    // Inicializar a valores inválidos
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++) {
        vertex.BoneIDs[i] = -1;
        vertex.Weights[i] = 0.0f;
    }
}
```

### ExtractBoneWeightForVertices

```cpp
void Model::ExtractBoneWeightForVertices(
    vector<Vertex>& vertices,
    aiMesh* mesh,
    const aiScene* scene
) {
    // Para cada bone en la malla
    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        string boneName = mesh->mBones[boneIndex]->mName.C_Str();

        // Obtener o crear ID del bone
        if (m_BoneInfoMap.find(boneName) == m_BoneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = m_BoneCounter;
            newBoneInfo.offset = AssimpGLMHelpers::ConvertMatrixToGLMFormat(
                mesh->mBones[boneIndex]->mOffsetMatrix
            );
            m_BoneInfoMap[boneName] = newBoneInfo;
            boneID = m_BoneCounter;
            m_BoneCounter++;
        } else {
            boneID = m_BoneInfoMap[boneName].id;
        }

        // Para cada vértice influenciado por este bone
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            // Añadir bone influence al vértice
            SetVertexBoneData(vertices[vertexId], boneID, weight);
        }
    }
}
```

### SetVertexBoneData

```cpp
void Model::SetVertexBoneData(Vertex& vertex, int boneID, float weight) {
    // Buscar slot vacío (BoneID == -1)
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
        if (vertex.BoneIDs[i] < 0) {
            vertex.Weights[i] = weight;
            vertex.BoneIDs[i] = boneID;
            break;
        }
    }
}
```

## Sistema de Texturas

### Carga de Texturas del Material

```cpp
vector<Texture> Model::loadMaterialTextures(
    aiMaterial *mat,
    aiTextureType type,
    string typeName,
    const aiScene* scene
) {
    vector<Texture> textures;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        // Verificar si ya fue cargada (cache)
        bool skip = false;
        for (unsigned int j = 0; j < m_textures_loaded.size(); j++) {
            if (std::strcmp(m_textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(m_textures_loaded[j]);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture;

            // Detectar si es textura embebida
            if (str.C_Str()[0] == '*') {
                // Textura embebida
                texture.id = TextureFromEmbedded(str, scene);
            } else {
                // Textura externa
                texture.id = TextureFromFile(str.C_Str(), m_directory);
            }

            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            m_textures_loaded.push_back(texture);
        }
    }

    return textures;
}
```

### Texturas Embebidas

```cpp
unsigned int Model::TextureFromEmbedded(aiString str, const aiScene* scene) {
    // Extraer índice de textura
    int index = std::atoi(&str.C_Str()[1]);
    const aiTexture* embeddedTexture = scene->mTextures[index];

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = nullptr;

    // Cargar desde memoria usando stb_image
    data = stbi_load_from_memory(
        reinterpret_cast<unsigned char*>(embeddedTexture->pcData),
        embeddedTexture->mWidth,
        &width, &height, &nrComponents, 0
    );

    if (data) {
        GLenum format = (nrComponents == 3) ? GL_RGB : GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height,
                     0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Configurar filtrado
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }

    return textureID;
}
```

### Texturas Externas

```cpp
unsigned int TextureFromFile(const char *path, const string &directory) {
    string filename = string(path);
    filename = directory + '/' + filename;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height,
                                    &nrComponents, 0);

    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                     format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}
```

## Renderizado de Modelos

### Clase Mesh - Draw

```cpp
void Mesh::Draw(Shader &shader) {
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;

    // Bind texturas
    for (unsigned int i = 0; i < m_textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        string number;
        string name = m_textures[i].type;

        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);

        // Set uniform
        shader.setInt((name + number).c_str(), i);

        // Bind textura
        glBindTexture(GL_TEXTURE_2D, m_textures[i].id);
    }

    // Renderizar malla
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset
    glActiveTexture(GL_TEXTURE0);
}
```

### Clase Model - Draw

```cpp
void Model::Draw(Shader &shader) {
    // Dibujar todas las mallas
    for (unsigned int i = 0; i < m_meshes.size(); i++) {
        m_meshes[i].Draw(shader);
    }
}
```

## Setup de Buffer Objects

### Mesh::setupMesh()

```cpp
void Mesh::setupMesh() {
    // Generar buffers
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    // Bind VAO
    glBindVertexArray(m_VAO);

    // VBO: Vertex Buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
                 &m_vertices[0], GL_STATIC_DRAW);

    // EBO: Element Buffer (indices)
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int),
                 &m_indices[0], GL_STATIC_DRAW);

    // Attribute 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Attribute 1: Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, Normal));

    // Attribute 2: TexCoords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, TexCoords));

    // Attribute 3: BoneIDs
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(Vertex),
                          (void*)offsetof(Vertex, BoneIDs));

    // Attribute 4: Weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, Weights));

    glBindVertexArray(0);
}
```

## Exportar Modelos desde Blender

### Pasos para GLB Óptimo

1. **Preparar Modelo:**
   ```
   - Seleccionar todo (A)
   - Object → Apply → All Transforms (Ctrl+A)
   - Object → Apply → Visual Geometry to Mesh
   ```

2. **Exportar GLB:**
   ```
   File → Export → glTF 2.0 (.glb/.gltf)

   Opciones importantes:
   ✓ Remember Export Settings
   ✓ Selected Objects (o vacío para todo)
   ✓ Apply Modifiers
   ✓ UVs
   ✓ Normals
   ✓ Tangents (si usas normal mapping)
   ✓ Vertex Colors
   ✓ Materials → Export
   ✓ Images → Automatic (embebidas)
   ✓ Compression → No compression (para depuración)

   Si tiene animación:
   ✓ Animation → Animation
   ✓ Shape Keys → Shape Keys
   ✓ Skinning
   ✓ Bake All Bone Animations
   ```

3. **Verificar:**
   ```
   - Abrir en Blender nuevamente
   - Verificar texturas
   - Verificar animación
   - Verificar escala y orientación
   ```

## Troubleshooting

### Problema: Modelo no se carga
```cpp
// Verificar output de ASSIMP
const aiScene* scene = importer.ReadFile(path, flags);
if (!scene) {
    std::cout << "ERROR::ASSIMP::" << importer.GetErrorString();
}
```

### Problema: Texturas negras
- Verificar que están embebidas en GLB
- Verificar que `verifyTextures()` reporta texturas > 0
- Verificar que shader usa `texture_diffuse1`

### Problema: Modelo deformado
- Verificar Apply Transforms en Blender
- Verificar escala en exportación GLB
- Verificar offset matrices de bones

### Problema: Animación corrupta
- Verificar que GLB tiene animación
- Verificar TicksPerSecond > 0
- Verificar que bones están correctamente rigeados

---

**Próximo:** [Cámara](CAMERA.md)
