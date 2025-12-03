#pragma once
#include <cmath>
#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>

#include "glm/glm.hpp"

#include "map_gen.hpp"

struct Door
{
    glm::vec2 locations[2]{};
    unsigned otherRoomId;
    TileAttrib doorType;

    bool operator==(Door& other);
};

class Room
{
public:
    Room(unsigned roomId);

    unsigned roomId;
    std::vector<glm::vec2> corners = {};

    std::vector<Door> doors;
};

class PortalVisibility
{
public:
    static PortalVisibility getFromMap(MapGen* map);

    std::vector<std::vector<unsigned>> getVisibilities();

private:
    std::vector<Room> rooms;
    MapGen* map;
    const float eps = 1e-4f;

    struct ViewConeOrTunnel
    {
        glm::vec2 Vectors[2];
        glm::vec2 Points[2];
    };

    const std::map<TileAttrib, glm::ivec2> cornerOffsetFormWallAttrib =
    {
        {TileAttrib::WallUp, {1, 0}},
        {TileAttrib::WallRight, {1, 1}},
        {TileAttrib::WallDown, {0, 1}},
        {TileAttrib::WallLeft, {0, 0}}
    };

    const std::map<TileAttrib, glm::ivec2> tileOffsetFromWallAttrib =
    {
        {TileAttrib::WallUp, {1, 0}},
        {TileAttrib::WallRight, {0, 1}},
        {TileAttrib::WallDown, {-1, 0}},
        {TileAttrib::WallLeft, {0, -1}}
    };

    const float doorStartOffset = 0.35f;
    const float doorEndOffset = 0.65f;

    const std::map<TileAttrib, std::vector<glm::vec2>> doorOffsetsFromDoorAttrib =
    {
        {TileAttrib::DoorUp, {{doorStartOffset, 0.f}, {doorEndOffset, 0.f}} },
        {TileAttrib::DoorRight, {{1.f, doorStartOffset}, {1.f, doorEndOffset}} },
        {TileAttrib::DoorDown, {{doorStartOffset, 1.f}, {doorEndOffset, 1.f}} },
        {TileAttrib::DoorLeft, {{0.f, doorStartOffset}, {0.f, doorEndOffset}} },
    };

    const std::map<TileAttrib, glm::ivec2> otherTileOffsetFromDoorAttrib =
    {
        {TileAttrib::DoorUp, { 0, -1 }},
        {TileAttrib::DoorRight, { 1, 0 }},
        {TileAttrib::DoorDown, { 0, 1 }},
        {TileAttrib::DoorLeft, { -1, 0 }}
    };

    float extendVectorToPlane(float plane, glm::vec2& vecStart, glm::vec2& vec, bool isVertical);

    bool isVertical(glm::vec2& first, glm::vec2& second);
    bool isVertical(Door& door);
    bool isVertical(ViewConeOrTunnel& coneOrTunnel);
    bool isInBoundingBox(std::vector<glm::vec2>& boundingBox, std::vector<glm::vec2> corners);
    bool hasWallDoor(Room& room, float wallPlane, glm::vec2 wallBorderVals, bool isWallVertical);
    bool isWallBetweenDoors(Room& room, Door& first, Door& second);
    bool areDoorsInSamePlane(Door& first, Door& second);

    ViewConeOrTunnel getViewConeOrTunnel(glm::vec2 fromFirst, glm::vec2 fromSecond, glm::vec2 toFirst, glm::vec2 toSecond, bool isTunnel);
    void addRoomsFromCone(std::unordered_set<unsigned>& visibleRooms, ViewConeOrTunnel& cone, Room& searchedRoom, Room& previousRoom, Door& entranceDoor, Door& initialDoor);
    Door& getDoorFromOtherPerspective(Room& currentRoom, Door& door);

    ViewConeOrTunnel getExtendedConeToLine(ViewConeOrTunnel& old, std::vector<glm::vec2>& line, bool isVertical);
    bool isLineInCone(ViewConeOrTunnel& cone, std::vector<glm::vec2>& line, bool isVertical);
};
