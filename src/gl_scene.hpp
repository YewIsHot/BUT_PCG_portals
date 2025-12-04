#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <unordered_set>

#include <SDL3/SDL.h>

#include <geGL/geGL.h>
#include <geGL/StaticCalls.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "map_gen.hpp"

#ifndef SRC_DIR
#define SRC_DIR "."
#endif

#ifndef MODELS_DIR
#define MODELS_DIR "."
#endif

#define SS_TILE_SIDE 6.3f
#define SS_WALL_WIDTH SS_TILE_SIDE / (24)

#define COLLISION_DISTANCE 2 * SS_WALL_WIDTH

using namespace ge::gl;

enum class GlBufferType
{
    VERTEX_POS,
    VERTEX_NORM,
    INDEX_BUFFER,
    INSTANCE_MATRIX,
    INSTANCE_ID,
    NUM_BUFFERS
};

class Model
{
public:
    std::vector<unsigned> instanceIds;
    std::vector<glm::mat4> instanceMatrices;

    Model(std::string modelFile, std::vector<glm::mat4>instanceMatrices, unsigned ssboBinding);
    bool load();
    void render(std::shared_ptr<Program> prg);
    void render(std::shared_ptr<Program> prg, glm::vec4 color);
    void glUpdateInstanceId();

private:
    struct Vertices
    {
        std::vector<aiVector3D> positions;
        std::vector<aiVector3D> normals;
        std::vector<aiVector2D> texCoords;
    };

    struct Mesh
    {
        unsigned nVertices;
        unsigned vertexOffset;
        unsigned nIndices;
        unsigned indicesOffset;
        aiColor4D diffuseColor;
    };

    std::string modelFile;

    Vertices vertices;
    std::vector<unsigned int> indices;
    std::vector<Mesh> meshes;

    GLuint VAO;
    GLuint glBuffers[(unsigned)GlBufferType::NUM_BUFFERS] = { 0 };
    unsigned ssboBinding;

    unsigned nVertices = 0;
    unsigned nIndices = 0;

    bool processScene(const aiScene *scene);
    void initGlBuffers();
    void loadGlBuffers();
};

class GLScene
{
public:
    static GLScene create(float width, float height, MapGen* map, std::vector<std::vector<unsigned>> visibilities);
    bool run();
    
    ~GLScene();

private:
    std::string vsFile = SRC_DIR "maze.vert";
    std::string fsFile = SRC_DIR "maze.frag";

    std::string pointerVsFile = SRC_DIR "pointer.vert";
    std::string pointerFsFile = SRC_DIR "pointer.frag";

    std::string doorModelFile = MODELS_DIR "door.glb";
    std::string floorModelFile = MODELS_DIR "floor.glb";
    std::string wallModelFile = MODELS_DIR "wall_long.glb";

    std::string vsSrc = "";
    std::string fsSrc = "";

    std::string pointerVsSrc = "";
    std::string pointerFsSrc = "";

    const vector<glm::ivec2> modelTileStarts = { {0, 0}, {1, 0}, {0, 1}, {0, 0} };
    const vector<float> modelTileRotations = { 0.f, 90.f };

    float windowWidth = -1.;
    float windowHeight = -1.;

    std::map<unsigned, int> keyDown;

    glm::vec2 rotationAngles{ 180.f, 0.f };
    const float distance = 10.f;
    glm::vec3 location{ -0.5f * SS_TILE_SIDE, -1.5f, -0.5f * SS_TILE_SIDE };

    glm::vec4 topDownViewport;

    glm::vec4 topDownWallColor = { 1.f, 1.f, 1.f, 1.f };
    glm::vec4 topDownDoorColor = { 1.f, 1.f, 0.f, 1.f };
    glm::vec4 topDownFloorColor = { 0.1f, 0.1f, 0.1f, 1.f };

    bool noclip = false;
    bool useVisibility = true;
    bool drawMinimap = true;

    MapGen *map;

    glm::uvec2 currentTile = {0, 0};

    std::vector<unsigned> doorTileOffsets;
    std::vector<glm::mat4> doorInstances;

    std::vector<unsigned> floorTileOffsets;
    std::vector<glm::mat4> floorInstances;

    std::vector<unsigned> wallTileOffsets;
    std::vector<glm::mat4> wallInstances;

    std::vector<unsigned> visibleTileIds;
    std::vector<std::vector<unsigned>> visibilities;

    std::vector<float> fpsBuffer;

    GLScene(float width, float height, MapGen* map, std::vector<std::vector<unsigned>> visibilities);
    bool init();

    void printFrameStatistics();

    static std::string loadFile(std::string& fileName);

    bool handleEvents();
    void updateCameraMinimap(std::shared_ptr<Program> prgTopDown, std::shared_ptr<Program> prgPointer);
    void updateCameraFpv(std::shared_ptr<Program> prg, float timeDiff);
    void cameraCollisions(float timeDiff);

    void updateVisibility();
    void updateInstanceIds(Model& model, std::vector<unsigned>& instanceTileOffsets);

    void addVerticalInstancesAt(unsigned x, unsigned y, std::vector<glm::mat4>& instances, TileAttrib verticalAttribUp);
    void addFloorInstancesAt(unsigned x, unsigned y);
    void addInstances();
};