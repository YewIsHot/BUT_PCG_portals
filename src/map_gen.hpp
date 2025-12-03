#pragma once

#include <vector>
#include <map>
#include <assert.h>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <format>
#include <initializer_list>

#include <glm/glm.hpp>

#include "cppgraphics.hpp"

#define N_CONFIGS 8

using namespace std;

enum class TileAttrib
{
    Empty = 0,

    WallUp = 0b0001,
    WallRight = 0b0010,
    WallDown = 0b0100,
    WallLeft = 0b1000,

    DoorUp = 0b00010000,
    DoorRight = 0b00100000,
    DoorDown = 0b01000000,
    DoorLeft = 0b10000000
};

enum class Direction
{
    Up = 1,
    Right = 2,
    Down = 3,
    Left = 3
};

class Point
{
public:
    int x;
    int y;

    Point();
    Point(int x, int y);

    void rotate90R();
    void rotate90L();
    void translate(int x, int y);
    void invert(bool invertX = true);
    string to_string();

private:

};

class RoomShape
{
public:
    RoomShape();
    RoomShape(vector<Point> segments);
    RoomShape(initializer_list<Point> segments);

    vector<Point> segments;

    RoomShape getConfig(unsigned configId) const;
    RoomShape deepCopy() const;
    string to_string();

    RoomShape& recalculateOrigin();

    RoomShape& translate(int x, int y);
    RoomShape& rotate90(bool toRight = true);
    RoomShape& invert();

private:
    // coresponds to current rotation and inversion
    int config = 0;
};

class RoomShapeFactory
{
public:
    const static std::vector<RoomShape> getDefaultShapes()
    {       
        std::vector<RoomShape> shapes = { 
            I,
            L,
            O,
            z,
            Z,
            T,
            U,
            X,
            W,
            H
        };

        for (auto& shape : shapes)
            shape.recalculateOrigin();

        return shapes;
    }

    const static RoomShape I;
    const static RoomShape L;
    const static RoomShape O;
    const static RoomShape z;
    const static RoomShape Z;
    const static RoomShape T;
    const static RoomShape U;
    const static RoomShape X;
    const static RoomShape W;
    const static RoomShape H;
};

class MapGen
{
public:
    MapGen(unsigned width, unsigned height);

    unsigned width;
    unsigned height;

    const vector<int> directionsPacked = { -1, 0, 1, 0 ,-1 };

    const std::map<TileAttrib, glm::ivec2> otherTileOffsetFromDoorAttrib =
    {
        {TileAttrib::DoorUp, { 0, -1 }},
        {TileAttrib::DoorRight, { 1, 0 }},
        {TileAttrib::DoorDown, { 0, 1 }},
        {TileAttrib::DoorLeft, { -1, 0 }}
    };

    const std::map < TileAttrib, TileAttrib> oppositeDoorAttrib =
    {
        {TileAttrib::DoorUp, TileAttrib::DoorDown},
        {TileAttrib::DoorRight, TileAttrib::DoorLeft},
        {TileAttrib::DoorDown, TileAttrib::DoorUp},
        {TileAttrib::DoorLeft, TileAttrib::DoorRight}
    };

    vector<RoomShape> rooms = {};

    void generate();
    bool createCustom(std::vector<RoomShape> rooms);
    bool addDoor(TileAttrib doorAttrib, unsigned x, unsigned y);

    void drawScheme(double width);

    struct Tile
    {
        int roomId = -1;
        unsigned status = 0u;
    };

    Tile& getTile(unsigned x, unsigned y);
    Tile& getTileDirect(unsigned n);
    bool isTileInRoom(Tile& tile);
    void addTileAttrib(Tile& tile, TileAttrib attrib);
    void removeTileAttrib(Tile& tile, TileAttrib attrib);
    bool hasTileAttrib(Tile& tile, TileAttrib attrib);

private:
    vector<RoomShape> shapes;

    double windowWidth = -1.;
    double windowHeight = -1.;

    int lastRoomId = -1;

    struct
    {
        int defaultGrid = cg::Black;
        int floor = cg::DarkGreen;
        int wall = cg::White;
        int door = cg::Blue;
    } drawColors;

    vector<Tile> grid;

    int getNewRoomId();

    bool constructRoom(unsigned x, unsigned y);
    bool placeRoom(RoomShape & room, bool addDoors = true);

    void drawInit(double width);
    void drawTile(unsigned x, unsigned y);
};
