#include "gl_scene.hpp"

Model::Model(std::string modelFile, std::vector<glm::mat4>instanceMatrices, unsigned ssboBinding)
{
    this->modelFile = modelFile;
    this->instanceMatrices = instanceMatrices;
    this->ssboBinding = ssboBinding;
}

void Model::initGlBuffers()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glGenBuffers((GLsizei)GlBufferType::NUM_BUFFERS, glBuffers);
    glBindVertexArray(0);
}

void Model::loadGlBuffers()
{
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, glBuffers[(unsigned)GlBufferType::VERTEX_POS]);
    glBufferData(GL_ARRAY_BUFFER, vertices.positions.size() * sizeof(aiVector3D), vertices.positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, glBuffers[(unsigned)GlBufferType::VERTEX_NORM]);
    glBufferData(GL_ARRAY_BUFFER, vertices.normals.size() * sizeof(aiVector3D), vertices.normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffers[(unsigned)GlBufferType::INDEX_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBuffers[(unsigned)GlBufferType::INSTANCE_MATRIX]);
    glBufferData(GL_SHADER_STORAGE_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), instanceMatrices.data(), GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboBinding, glBuffers[(unsigned)GlBufferType::INSTANCE_MATRIX]);

    glBindBuffer(GL_ARRAY_BUFFER, glBuffers[(unsigned)GlBufferType::INSTANCE_ID]);
    glBufferData(GL_ARRAY_BUFFER, instanceIds.size() * sizeof(unsigned), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, 0, 0);
    glVertexAttribDivisor(2, 1);

    glBindVertexArray(0);
}

void Model::glUpdateInstanceId()
{
    glBindBuffer(GL_ARRAY_BUFFER, glBuffers[(unsigned)GlBufferType::INSTANCE_ID]);
    glBufferData(GL_ARRAY_BUFFER, instanceIds.size() * sizeof(unsigned), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceIds.size() * sizeof(unsigned), instanceIds.data());
}

bool Model::processScene(const aiScene* scene)
{
    meshes.resize(scene->mNumMeshes);

    for (unsigned i = 0; i < meshes.size(); i++) {
        meshes[i].nVertices = scene->mMeshes[i]->mNumVertices;
        meshes[i].nIndices = scene->mMeshes[i]->mNumFaces * 3;
        meshes[i].vertexOffset = nVertices;
        meshes[i].indicesOffset = nIndices;

        nVertices += scene->mMeshes[i]->mNumVertices;
        nIndices += meshes[i].nIndices;
    }

    vertices.positions.resize(nVertices);
    vertices.normals.resize(nVertices);
    vertices.texCoords.resize(nVertices);

    indices.resize(nIndices);

    for (unsigned meshId = 0; meshId < scene->mNumMeshes; meshId++)
    {
        // vertices
        for (unsigned vertexId = 0; vertexId < scene->mMeshes[meshId]->mNumVertices; vertexId++)
        {
            unsigned globalVertexId = meshes[meshId].vertexOffset + vertexId;

            const aiVector3D& pos = scene->mMeshes[meshId]->mVertices[vertexId];
            vertices.positions[globalVertexId] = pos;

            if (scene->mMeshes[meshId]->mNormals)
            {
                const aiVector3D& norm = scene->mMeshes[meshId]->mNormals[vertexId];
                vertices.normals[globalVertexId] = norm;
            }
            else
            {
                vertices.normals[globalVertexId] = aiVector3D(0);
            }
        }

        // indices
        for (unsigned faceId = 0; faceId < scene->mMeshes[meshId]->mNumFaces; faceId++)
        {
            unsigned globalIndexId = meshes[meshId].indicesOffset + faceId * 3;

            const aiFace& face = scene->mMeshes[meshId]->mFaces[faceId];
            indices[globalIndexId] = face.mIndices[0];
            indices[globalIndexId + 1] = face.mIndices[1];
            indices[globalIndexId + 2] = face.mIndices[2];
        }

        // colors
        auto color = aiColor4D(1.f);
        scene->mMaterials[meshId]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        meshes[meshId].diffuseColor = color;
    }
    return true;
}

bool Model::load()
{
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(modelFile, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices);

    if (!scene || !scene->mRootNode)
    {
        std::cerr << "Failed to loading " << modelFile << " :" << importer.GetErrorString() << std::endl;
        return false;
    }

    initGlBuffers();
    processScene(scene);
    loadGlBuffers();

    return true;
}

void Model::render(std::shared_ptr<Program> prg)
{
    assert(meshes.size() > 0);

    glBindVertexArray(VAO);

    for (unsigned meshId = 0; meshId < meshes.size(); meshId++)
    {
        assert(meshes[meshId].nVertices > 0);

        float r = meshes[meshId].diffuseColor.r;
        float g = meshes[meshId].diffuseColor.g;
        float b = meshes[meshId].diffuseColor.b;
        float a = meshes[meshId].diffuseColor.a;

        prg->set4f("meshColor", r, g, b, a);
        glDrawElementsInstancedBaseVertex(
            GL_TRIANGLES,
            meshes[meshId].nIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned) * meshes[meshId].indicesOffset),
            instanceIds.size(),
            meshes[meshId].vertexOffset);
    }

    glBindVertexArray(0);
}

void Model::render(std::shared_ptr<Program> prg, glm::vec4 color)
{
    assert(meshes.size() > 0);

    glBindVertexArray(VAO);

    for (unsigned meshId = 0; meshId < meshes.size(); meshId++)
    {
        assert(meshes[meshId].nVertices > 0);

        prg->set4f("meshColor", color.r, color.g, color.b, color.a);
        glDrawElementsInstancedBaseVertex(
            GL_TRIANGLES,
            meshes[meshId].nIndices,
            GL_UNSIGNED_INT,
            (void*)(sizeof(unsigned) * meshes[meshId].indicesOffset),
            instanceIds.size(),
            meshes[meshId].vertexOffset);
    }

    glBindVertexArray(0);
}
