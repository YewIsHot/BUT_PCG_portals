#include "map_gen.hpp"

#define COORD_ASSERT(x, y) assert(x < width); assert(y < height)

MapGen::MapGen(unsigned width, unsigned height)
{
    this->width = width;
    this->height = height;

    grid.resize(width * height);
    shapes = RoomShapeFactory::getDefaultShapes();
}

MapGen::Tile& MapGen::getTile(unsigned x, unsigned y)
{
    COORD_ASSERT(x, y);

    return grid[y * width + x];
}

MapGen::Tile& MapGen::getTileDirect(unsigned n)
{
    assert(n < grid.size());

    return grid[n];
}

int MapGen::getNewRoomId()
{
    return ++lastRoomId;
}

void MapGen::addTileAttrib(Tile& tile, TileAttrib attrib)
{
    tile.status |= (unsigned)attrib;
}

void MapGen::removeTileAttrib(Tile& tile, TileAttrib attrib)
{
    tile.status &= ~((unsigned)attrib);
}

bool MapGen::hasTileAttrib(Tile& tile, TileAttrib attrib)
{
    return tile.status & (unsigned)attrib;
}

bool MapGen::isTileInRoom(Tile& tile)
{
    return !(tile.roomId == -1);
}

bool MapGen::addDoor(TileAttrib doorAttrib, unsigned x, unsigned y)
{
    unsigned nX = x + otherTileOffsetFromDoorAttrib.at(doorAttrib).x;
    unsigned nY = y + otherTileOffsetFromDoorAttrib.at(doorAttrib).y;

    if (x >= width || y >= height)
        return false;

    if (nX >= width || nY >= height)
        return false;

    Tile& tile = getTile(x, y);
    Tile& nTile = getTile(nX, nY);

    if (!isTileInRoom(tile) || !isTileInRoom(nTile))
        return false;

    if (tile.roomId == nTile.roomId)
        return false;

    addTileAttrib(tile, doorAttrib);
    addTileAttrib(nTile, oppositeDoorAttrib.at(doorAttrib));

    return true;
}

bool MapGen::placeRoom(RoomShape& room, bool addDoors)
{
    unsigned x = room.segments[0].x;
    unsigned y = room.segments[0].y;

    Tile& startTile = getTile(x, y);
    assert(!isTileInRoom(startTile));

    for (int i = 1; i < room.segments.size(); i++)
    {
        unsigned tileX = (unsigned)(room.segments[i].x);
        unsigned tileY = (unsigned)(room.segments[i].y);

        if (tileX >= width || tileY >= height)
            return false;

        Tile& tile = getTile(tileX, tileY);
        if (isTileInRoom(tile))
            return false;
    }

    int roomId = getNewRoomId();
    rooms.push_back(room);

    for (Point& seg : room.segments)
    {
        Tile& tile = getTile(seg.x, seg.y);
        tile.roomId = roomId;
    }

    // Add walls
    for (Point& seg : room.segments)
    {

        int tileX = seg.x;
        int tileY = seg.y;
        Tile& tile = getTile(seg.x, seg.y);


        for (int i = 0; i < directionsPacked.size() - 1; i++)
        {
            int nY = tileY + directionsPacked[i];
            int nX = tileX + directionsPacked[i + 1];

            TileAttrib wallAttrib = (TileAttrib)(1u << i);

            if ((unsigned)nX < width && (unsigned)nY < height)
            {
                Tile& neighbour = getTile(nX, nY);
                if (neighbour.roomId != roomId)
                    addTileAttrib(tile, wallAttrib);
            }
            else
            {
                addTileAttrib(tile, wallAttrib);
            }
        }
    }

    if (addDoors)
    {
        // add doors to adjacent room/rooms
        addDoor(TileAttrib::DoorUp, x, y);
        addDoor(TileAttrib::DoorLeft, x, y);
    }

    return true;
}

bool MapGen::constructRoom(unsigned x, unsigned y)
{
    auto availableShapes = std::vector<RoomShape>(shapes.size());

    for (int i = 0; i < shapes.size(); i++)
    {
        availableShapes[i] = shapes[i].deepCopy();
    }

    while (availableShapes.size() > 0)
    {
        int shapeId = rand() % availableShapes.size();

        auto availableConfigs = std::vector<int>(N_CONFIGS);
        for (int i = 0; i < N_CONFIGS; i++)
        {
            availableConfigs[i] = i;
        }

        while (availableConfigs.size() > 0)
        {
            int configId = rand() % availableConfigs.size();
            RoomShape newRoom = availableShapes[shapeId].getConfig(configId);
            newRoom.translate(x, y);

            assert(newRoom.segments[0].x == x);
            assert(newRoom.segments[0].y == y);

            if (placeRoom(newRoom, true))
                return true;

            availableConfigs.erase(availableConfigs.begin() + configId);
        }

        availableShapes.erase(availableShapes.begin() + shapeId);
    }

    return false;
}

void MapGen::generate()
{
    for (unsigned y = 0; y < height; y++)
    {
        for (unsigned x = 0; x < width; x++)
        {
            Tile& tile = getTile(x, y);

            if (isTileInRoom(tile))
                continue;

            constructRoom(x, y);
        }
    }
}

bool MapGen::createCustom(std::vector<RoomShape> rooms)
{
    for (auto& room : rooms)
    {
        if (!placeRoom(room, false))
            return false;
    }

    return true;
}
