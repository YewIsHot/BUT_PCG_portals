#include "gl_scene.hpp"

GLScene GLScene::create(float width, float height, MapGen *map, std::vector<std::vector<unsigned>> visibilities)
{
    GLScene portals(width, height, map, visibilities);
    if (!portals.init())
        throw std::runtime_error("Could not open shader files!");

    return portals;
}

GLScene::GLScene(float width, float height, MapGen* map, std::vector<std::vector<unsigned>> visibilities)
{
    windowWidth = width;
    windowHeight = height;
    this->map = map;
    this->visibilities = visibilities;

    topDownViewport = {
        0,
        windowHeight - windowHeight / 2.5f,
        windowHeight / 2.5f,
        windowHeight / 2.5f 
    };
}

void GLScene::printFrameStatistics()
{
    if (frameTimes.size() == 0)
        return;

    frameTimes.erase(frameTimes.begin());
    std::sort(frameTimes.begin(), frameTimes.end(), greater<float>());

    std::cout << "Median FPS: " << 1 / frameTimes[frameTimes.size() / 2] << std::endl;

    float avg = 0;
    float percent99 = 0;
    for (unsigned i = 0; i < frameTimes.size(); i++)
    {
        avg += frameTimes[i];

        if (i == frameTimes.size() / 99)
        {
            percent99 = 1 / (avg / i);
        }
    }

    std::cout << "Average FPS: " << 1 / (avg / frameTimes.size()) << std::endl;
    std::cout << "99% FPS: " << percent99 << std::endl;


}

GLScene::~GLScene()
{
    printFrameStatistics();
}

std::string GLScene::loadFile(std::string& fileName)
{
    std::ifstream frag(fileName);

    if (!frag.is_open())
    {
        std::cerr << "Could not open file " << fileName << std::endl;
        return "";
    }

    std::stringstream file;
    file << frag.rdbuf();

    return file.str();
}

bool GLScene::init()
{
    vsSrc = loadFile(vsFile);
    if (vsSrc.empty())
        return false;

    fsSrc = loadFile(fsFile);
    if (fsSrc.empty())
        return false;

    pointerVsSrc = loadFile(pointerVsFile);
    if (pointerVsSrc.empty())
        return false;

    pointerFsSrc = loadFile(pointerFsFile);
    if (pointerFsSrc.empty())
        return false;

    return true;
}

bool GLScene::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT || (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE))
            return false;

        if (event.type == SDL_EVENT_KEY_DOWN)
            keyDown[event.key.key] = 1;

        if (event.type == SDL_EVENT_KEY_UP)
            keyDown[event.key.key] = 0;

        if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (event.motion.state & SDL_BUTTON_RMASK)
            {
                rotationAngles.x += event.motion.xrel / 4;

                rotationAngles.y += event.motion.yrel / 4;
                rotationAngles.y = max(min(rotationAngles.y, 90.f), -90.f);
            }
        }
    }

    if ((bool)keyDown[SDLK_X])
        noclip = true;

    if ((bool)keyDown[SDLK_C])
        noclip = false;

    if ((bool)keyDown[SDLK_Q])
        useVisibility = true;

    if ((bool)keyDown[SDLK_E])
        useVisibility = false;

    if ((bool)keyDown[SDLK_F])
        drawMinimap = true;

    if ((bool)keyDown[SDLK_G])
        drawMinimap = false;

    return true;
}

void GLScene::cameraCollisions(float timeDiff)
{
    MapGen::Tile& tile = map->getTile(currentTile.x, currentTile.y);

    if (map->hasTileAttrib(tile, TileAttrib::DoorLeft))
    {
        if ((abs(location.z) - currentTile.y * SS_TILE_SIDE < SS_TILE_SIDE / 3 ||
            abs(location.z) - currentTile.y * SS_TILE_SIDE > 2 * SS_TILE_SIDE / 3) &&
            abs(location.x) - currentTile.x * SS_TILE_SIDE < COLLISION_DISTANCE)
        {
            location -= glm::vec3(1.f, 0.f, 0.f) * (distance * timeDiff);
        }
    }
    else if (map->hasTileAttrib(tile, TileAttrib::WallLeft))
    {
        if (abs(location.x) - currentTile.x * SS_TILE_SIDE < COLLISION_DISTANCE)
        {
            location -= glm::vec3(1.f, 0.f, 0.f) * (distance * timeDiff);
        }
    }

    if (map->hasTileAttrib(tile, TileAttrib::DoorRight))
    {
        if ((abs(location.z) - currentTile.y * SS_TILE_SIDE < SS_TILE_SIDE / 3 ||
            abs(location.z) - currentTile.y * SS_TILE_SIDE > 2 * SS_TILE_SIDE / 3) &&
            (currentTile.x + 1) * SS_TILE_SIDE - abs(location.x) < COLLISION_DISTANCE)
        {
            location += glm::vec3(1.f, 0.f, 0.f) * (distance * timeDiff);
        }
    }
    else if (map->hasTileAttrib(tile, TileAttrib::WallRight))
    {
        if ((currentTile.x + 1) * SS_TILE_SIDE - abs(location.x) < COLLISION_DISTANCE)
        {
            location += glm::vec3(1.f, 0.f, 0.f) * (distance * timeDiff);
        }
    }

    if (map->hasTileAttrib(tile, TileAttrib::DoorUp))
    {
        if ((abs(location.x) - currentTile.x * SS_TILE_SIDE < SS_TILE_SIDE / 3 ||
            abs(location.x) - currentTile.x * SS_TILE_SIDE > 2 * SS_TILE_SIDE / 3) &&
            abs(location.z) - currentTile.y * SS_TILE_SIDE < COLLISION_DISTANCE)
        {
            location -= glm::vec3(0.f, 0.f, 1.f) * (distance * timeDiff);
        }
    }
    else if (map->hasTileAttrib(tile, TileAttrib::WallUp))
    {
        if (abs(location.z) - currentTile.y * SS_TILE_SIDE < COLLISION_DISTANCE)
        {
            location -= glm::vec3(0.f, 0.f, 1.f) * (distance * timeDiff);
        }
    }

    if (map->hasTileAttrib(tile, TileAttrib::DoorDown))
    {
        if ((abs(location.x) - currentTile.x * SS_TILE_SIDE < SS_TILE_SIDE / 3 ||
            abs(location.x) - currentTile.x * SS_TILE_SIDE > 2 * SS_TILE_SIDE / 3) &&
            (currentTile.y + 1) * SS_TILE_SIDE - abs(location.z) < COLLISION_DISTANCE)
        {
            location += glm::vec3(0.f, 0.f, 1.f) * (distance * timeDiff);
        }
    }
    else if (map->hasTileAttrib(tile, TileAttrib::WallDown))
    {
        if ((currentTile.y + 1) * SS_TILE_SIDE - abs(location.z) < COLLISION_DISTANCE)
        {
            location += glm::vec3(0.f, 0.f, 1.f) * (distance * timeDiff);
        }
    }

    
}

void GLScene::updateCameraMinimap(std::shared_ptr<Program> topDownPrg, std::shared_ptr<Program> pointerPrg)
{
    auto topDownTranslation = glm::translate(glm::mat4(1.f), glm::vec3(location.x, -2.f, location.z));
    const auto topDownRotation = glm::rotate(glm::mat4(1.f), glm::radians(90.f), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1.f), glm::radians(180.f), glm::vec3(0, 1, 0));
    auto topDownView = topDownRotation * topDownTranslation;

    auto pointerView = glm::rotate(glm::mat4(1.f), glm::radians(rotationAngles.x), glm::vec3(0, 0, -1));

    const float topDownCorrection = 10;
    auto projTopDown = glm::ortho(
        topDownViewport.z / topDownCorrection,
        -topDownViewport.z / topDownCorrection,
        topDownViewport.w / topDownCorrection,
        -topDownViewport.w / topDownCorrection,
        0.f, 10.f);

    topDownPrg->setMatrix4fv("view", (float*)&topDownView);
    topDownPrg->setMatrix4fv("proj", (float*)&projTopDown);

    pointerPrg->setMatrix4fv("view", (float*)&pointerView);
    pointerPrg->setMatrix4fv("proj", (float*)&projTopDown);
}

void GLScene::updateCameraFpv(std::shared_ptr<Program> fpvPrg, float timeDiff)
{
    auto rotationY = glm::rotate(glm::mat4(1.f), glm::radians(rotationAngles.x), glm::vec3(0, 1, 0));
    auto rotationX = glm::rotate(glm::mat4(1.f), glm::radians(rotationAngles.y), glm::vec3(1, 0, 0));
    auto translation = glm::translate(glm::mat4(1.f), location);
    auto rotation = rotationX * rotationY;
    auto view = rotation * translation;

    currentTile.x = -location.x / SS_TILE_SIDE;
    currentTile.y = -location.z / SS_TILE_SIDE;

    auto viewT = glm::transpose(rotationY * translation);

    glm::vec3 locationUpdate =
          glm::vec3(viewT[2]) * (float)keyDown[SDLK_W]
        - glm::vec3(viewT[2]) * (float)keyDown[SDLK_S]
        + glm::vec3(viewT[0]) * (float)keyDown[SDLK_A]
        - glm::vec3(viewT[0]) * (float)keyDown[SDLK_D];

    if (locationUpdate.x != 0 || locationUpdate.y != 0 || locationUpdate.z != 0)
        locationUpdate = glm::normalize(locationUpdate);

    location += locationUpdate * (distance * timeDiff);

    if (noclip)
    {
        location -= glm::vec3(viewT[1]) * (float)keyDown[SDLK_SPACE] * (distance * timeDiff);
        location += glm::vec3(viewT[1]) * (float)keyDown[SDLK_LCTRL] * (distance * timeDiff);
    }
    else
    {
        location.y = -1.5f;

        if (currentTile.x < map->width && currentTile.y < map->height)
            cameraCollisions(timeDiff);
    }

    auto proj = glm::tweakedInfinitePerspective(glm::quarter_pi<float>(), windowWidth / windowHeight, 0.01f);

    fpvPrg->setMatrix4fv("view", (float*)&view);
    fpvPrg->setMatrix4fv("proj", (float*)&proj);

    auto defaultDir = glm::vec4(0.f, 0.f, 1.f, 0.f);
    auto dir = defaultDir * rotation;
    fpvPrg->set3f("dir", dir.x, dir.y, dir.z);
}

// Add instance Ids of either doors or walls
void GLScene::addVerticalInstancesAt(unsigned x, unsigned y, std::vector<glm::mat4>& instances, TileAttrib verticalAttribUp)
{
    MapGen::Tile& tile = map->getTile(x, y);

    for (unsigned i = 0; i < modelTileStarts.size(); i++)
    {
        if (!map->hasTileAttrib(tile, (TileAttrib)((unsigned)verticalAttribUp << i)))
            continue;

        if (verticalAttribUp == TileAttrib::WallUp &&
            map->hasTileAttrib(tile, (TileAttrib)((unsigned)TileAttrib::DoorUp << i)))
            continue;

        auto translationVec = glm::vec3((x + modelTileStarts[i].x) * SS_TILE_SIDE, 0.f, (y + modelTileStarts[i].y) * SS_TILE_SIDE);
        auto translation = glm::translate(glm::mat4(1.f), translationVec);

        auto rotation = glm::rotate(glm::mat4(1.f), glm::radians(modelTileRotations[i % 2]), glm::vec3(0.f, -1.f, 0.f));

        instances.push_back(translation * rotation);
    }
}

void GLScene::addFloorInstancesAt(unsigned x, unsigned y)
{
    // tile is always 3X3
    for (int lY = 0; lY < 3; lY++)
    {
        for (int lX = 0; lX < 3; lX++)
        {
            auto translation = glm::vec3((x * SS_TILE_SIDE) + (lX * (SS_TILE_SIDE / 3)), 0.f, (y * SS_TILE_SIDE) + (lY * (SS_TILE_SIDE / 3)));
            floorInstances.push_back(glm::translate(glm::mat4(1.f), translation));
        }
    }
}

void GLScene::addInstances()
{
    for (int y = 0; y < map->height; y++)
    {
        for (int x = 0; x < map->width; x++)
        {
            doorTileOffsets.push_back(doorInstances.size());
            wallTileOffsets.push_back(wallInstances.size());
            floorTileOffsets.push_back(floorInstances.size());

            MapGen::Tile& tile = map->getTile(x, y);
            if (!map->isTileInRoom(tile))
                continue;

            addVerticalInstancesAt(x, y, wallInstances, TileAttrib::WallUp);
            addVerticalInstancesAt(x, y, doorInstances, TileAttrib::DoorUp);
            addFloorInstancesAt(x, y);
        }
    }

    // add final size as a backstop
    doorTileOffsets.push_back(doorInstances.size());
    wallTileOffsets.push_back(wallInstances.size());
    floorTileOffsets.push_back(floorInstances.size());
}

// 
void GLScene::updateInstanceIds(Model& model, std::vector<unsigned>& instanceTileOffsets)
{
    model.instanceIds = {};
    std::unordered_set<glm::mat4> toBeRendered;

    for (auto tileId : visibleTileIds)
    {
        assert(tileId < map->width * map->height);

        unsigned firstId = instanceTileOffsets[tileId];
        unsigned nextTileFirstId = instanceTileOffsets[tileId + 1];

        if (firstId == nextTileFirstId)
            continue;

        unsigned lastId = nextTileFirstId - 1;

        for (unsigned i = firstId; i <= lastId; i++)
        {
            if (toBeRendered.contains(model.instanceMatrices[i]))
                continue;

            model.instanceIds.push_back(i);
            toBeRendered.insert(model.instanceMatrices[i]);
        }
    }

    model.glUpdateInstanceId();
}

void GLScene::updateVisibility()
{
    if (currentTile.x >= map->width || currentTile.y >= map->height)
        return;

    MapGen::Tile tile = map->getTile(currentTile.x, currentTile.y);
    if (!map->isTileInRoom(tile))
        return;

    visibleTileIds = {};
    for (auto roomId : visibilities[tile.roomId])
    {
        for (auto tileCoords : map->rooms[roomId].segments)
        {
            visibleTileIds.push_back(tileCoords.y * map->width + tileCoords.x);
        }
    }
}

bool GLScene::run()
{
    auto window = SDL_CreateWindow("PGR", windowWidth, windowHeight, SDL_WINDOW_OPENGL);
    auto context = SDL_GL_CreateContext(window);
    ge::gl::init();

    //create shaders
    auto vs = std::make_shared<Shader>(GL_VERTEX_SHADER, vsSrc.c_str());
    auto fs = std::make_shared<Shader>(GL_FRAGMENT_SHADER, fsSrc.c_str());
    auto fpvPrg = std::make_shared<Program>(vs, fs);
    auto topDownPrg = std::make_shared<Program>(vs, fs);

    auto pointerVs = std::make_shared<Shader>(GL_VERTEX_SHADER, pointerVsSrc.c_str());
    auto pointerFs = std::make_shared<Shader>(GL_FRAGMENT_SHADER, pointerFsSrc.c_str());
    auto pointerPrg = std::make_shared<Program>(pointerVs, pointerFs);

    if (auto m = glGetError()) std::cerr << m << std::endl;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(0.3f, 0.5f, 1.f, 1.f);

    addInstances();

    // Load models
    Model doorModel(doorModelFile, doorInstances, 0);
    doorModel.load();

    Model wallModel(wallModelFile, wallInstances, 1);
    wallModel.load();

    Model floorModel(floorModelFile, floorInstances, 2);
    floorModel.load();

    const std::vector<Model*> models = { &doorModel, &wallModel, &floorModel };
    const std::vector<std::vector<unsigned>*> modelsTileOffsets = { &doorTileOffsets, &wallTileOffsets, &floorTileOffsets };

    auto startTime = SDL_GetPerformanceCounter();

    while (true)
    {
        if (!handleEvents())
            break;

        auto stopTime = SDL_GetPerformanceCounter();
        float timeDiff = (stopTime - startTime) / (float)SDL_GetPerformanceFrequency();
        frameTimes.push_back(timeDiff);
        startTime = stopTime;

        updateCameraFpv(fpvPrg, timeDiff);

        // update

        if (useVisibility)
        {
            updateVisibility();
        }
        else
        {
            visibleTileIds = {};
            for (unsigned i = 0; i < map->width * map->height; i++)
                visibleTileIds.push_back(i);

        }

        for (unsigned i = 0; i < models.size(); i++)
        {
            updateInstanceIds(*models[i], *modelsTileOffsets[i]);
        }
        

        // draw main scene

        fpvPrg->use();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, windowWidth, windowHeight);

        fpvPrg->set1i("modelType", 0);
        doorModel.render(fpvPrg);

        fpvPrg->set1i("modelType", 1);
        wallModel.render(fpvPrg);

        fpvPrg->set1i("modelType", 2);
        floorModel.render(fpvPrg);

        if (drawMinimap)
        {
            updateCameraMinimap(topDownPrg, pointerPrg);

            // draw topdown

            topDownPrg->use();

            glViewport(topDownViewport.x, topDownViewport.y, topDownViewport.z, topDownViewport.w);

            glEnable(GL_SCISSOR_TEST);
            glScissor(topDownViewport.x, topDownViewport.y, topDownViewport.z, topDownViewport.w);

            glClear(GL_DEPTH_BUFFER_BIT);

            topDownPrg->set1i("modelType", 0);
            doorModel.render(topDownPrg, topDownDoorColor);

            topDownPrg->set1i("modelType", 1);
            wallModel.render(topDownPrg, topDownWallColor);

            topDownPrg->set1i("modelType", 2);
            floorModel.render(topDownPrg, topDownFloorColor);

            // draw pointer

            pointerPrg->use();
            glClear(GL_DEPTH_BUFFER_BIT);

            glDrawArrays(GL_TRIANGLES, 0, 3);

            glDisable(GL_SCISSOR_TEST);
        }

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    return true;
}
  