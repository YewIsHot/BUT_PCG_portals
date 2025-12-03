#include "portal_visibility.hpp"

bool Door::operator==(Door& other)
{
    return this->locations == other.locations && this->otherRoomId == other.otherRoomId;
}

Room::Room(unsigned roomId)
{
    this->roomId = roomId;
}

TileAttrib rotateRight(TileAttrib wallAttrib)
{
    return wallAttrib == TileAttrib::WallLeft ? TileAttrib::WallUp : (TileAttrib)((unsigned)wallAttrib << 1u);
}

TileAttrib rotateLeft(TileAttrib wallAttrib)
{
    return wallAttrib == TileAttrib::WallUp ? TileAttrib::WallLeft : (TileAttrib)((unsigned)wallAttrib >> 1u);
}

PortalVisibility PortalVisibility::getFromMap(MapGen* map)
{
    PortalVisibility portal;

    // Add corners and doors
    for (unsigned i = 0; i < map->rooms.size(); i++)
    {
        int startX = map->rooms[i].segments[0].x;
        int startY = map->rooms[i].segments[0].y;

        Room newRoom(map->getTile(startX, startY).roomId);

        TileAttrib wallDirection = TileAttrib::WallUp;

        int x = startX;
        int y = startY;

        while (true)
        {
            MapGen::Tile& tile = map->getTile(x, y);

            if (!map->hasTileAttrib(tile, wallDirection))
            {
                wallDirection = rotateLeft(wallDirection);

                glm::vec2 newCorner = { x + portal.cornerOffsetFormWallAttrib.at(wallDirection).x, y + portal.cornerOffsetFormWallAttrib.at(wallDirection).y };
                if (newRoom.corners.size() > 0 && newRoom.corners[0] == newCorner)
                    break;

                newRoom.corners.push_back(newCorner);

                x += portal.tileOffsetFromWallAttrib.at(wallDirection).x;
                y += portal.tileOffsetFromWallAttrib.at(wallDirection).y;

                continue;
            }

            TileAttrib doorDirection = (TileAttrib)((unsigned)wallDirection << 4);

            if (map->hasTileAttrib(tile, doorDirection))
            {
                Door newDoor;

                newDoor.locations[0].x = x + portal.doorOffsetsFromDoorAttrib.at(doorDirection)[0].x;
                newDoor.locations[0].y = y + portal.doorOffsetsFromDoorAttrib.at(doorDirection)[0].y;
                newDoor.locations[1].x = x + portal.doorOffsetsFromDoorAttrib.at(doorDirection)[1].x;
                newDoor.locations[1].y = y + portal.doorOffsetsFromDoorAttrib.at(doorDirection)[1].y;

                MapGen::Tile& otherTile = map->getTile(x + portal.otherTileOffsetFromDoorAttrib.at(doorDirection).x, y + portal.otherTileOffsetFromDoorAttrib.at(doorDirection).y);
                newDoor.otherRoomId = otherTile.roomId;
                newDoor.doorType = doorDirection;

                newRoom.doors.push_back(newDoor);
            }


            if (!map->hasTileAttrib(tile, rotateRight(wallDirection)))
            {
                x += portal.tileOffsetFromWallAttrib.at(wallDirection).x;
                y += portal.tileOffsetFromWallAttrib.at(wallDirection).y;
            }
            else
            {
                glm::vec2 newCorner = { x + portal.cornerOffsetFormWallAttrib.at(wallDirection).x, y + portal.cornerOffsetFormWallAttrib.at(wallDirection).y };
                if (newRoom.corners.size() > 0 && newRoom.corners[0] == newCorner)
                    break;

                newRoom.corners.push_back(newCorner);

                wallDirection = rotateRight(wallDirection);
            }
        }

        portal.rooms.push_back(newRoom);
    }

    portal.map = map;
    return portal;
}

bool PortalVisibility::isVertical(glm::vec2& first, glm::vec2& second)
{
    return fabs(first.x - second.x) < eps;
}

bool PortalVisibility::isVertical(Door& door)
{
    return isVertical(door.locations[0], door.locations[1]);
}

bool PortalVisibility::isVertical(ViewConeOrTunnel& coneOrTunnel)
{
    return isVertical(coneOrTunnel.Points[0], coneOrTunnel.Points[1]);
}

PortalVisibility::ViewConeOrTunnel PortalVisibility::getViewConeOrTunnel(glm::vec2 fromFirst, glm::vec2 fromSecond, glm::vec2 toFirst, glm::vec2 toSecond, bool isTunnel)
{
    ViewConeOrTunnel viewCone1;
    viewCone1.Vectors[0] = toSecond - fromFirst;
    viewCone1.Vectors[1] = toFirst - fromSecond;
    viewCone1.Points[0] = fromFirst;
    viewCone1.Points[1] = fromSecond;

    ViewConeOrTunnel viewCone2;
    viewCone2.Vectors[0] = toFirst - fromFirst;
    viewCone2.Vectors[1] = toSecond - fromSecond;
    viewCone2.Points[0] = fromFirst;
    viewCone2.Points[1] = fromSecond;

    float dot1 = glm::dot(viewCone1.Vectors[0], viewCone1.Vectors[1]);
    float dot2 = glm::dot(viewCone2.Vectors[0], viewCone2.Vectors[1]);

    float len10 = glm::length(viewCone1.Vectors[0]);
    float len11 = glm::length(viewCone1.Vectors[1]);
    float len20 = glm::length(viewCone2.Vectors[0]);
    float len21 = glm::length(viewCone2.Vectors[1]);

    float dotTheta1 = dot1 / (len10 * len11);
    float dotTheta2 = dot2 / (len20 * len21);

    bool isFirstTunnel = fabs(dotTheta1) - 1.f > fabs(dotTheta2) - 1.f;

    if (isTunnel)
    {
        return isFirstTunnel ? viewCone1 : viewCone2;
    }
    else
    {
        return isFirstTunnel ? viewCone2 : viewCone1;
    }
}

float PortalVisibility::extendVectorToPlane(float plane, glm::vec2& vecStart, glm::vec2& vec, bool isVertical)
{
    float vecVal = isVertical ? vec.x : vec.y;
    float vecValOther = isVertical ? vec.y : vec.x;
    float vecStartCoord = isVertical ? vecStart.x : vecStart.y;
    float vecStartOther = isVertical ? vecStart.y : vecStart.x;

    float mult = (plane - vecStartCoord) / (vecVal + FLT_MIN);
    float val = vecValOther * mult + vecStartOther;

    if (mult < 0) // vector points from the door plane
        val = val < 0 ? -INFINITY : INFINITY;

    return val;
}

bool PortalVisibility::isLineInCone(ViewConeOrTunnel& cone, std::vector<glm::vec2>& line, bool isVertical)
{
    std::vector<float> lineVals(2);
    lineVals[0] = isVertical ? line[0].y : line[0].x;
    lineVals[1] = isVertical ? line[1].y : line[1].x;
    float plane = isVertical ? line[0].x : line[0].y;

    bool isConeBaseVertical = this->isVertical(cone);
    bool isLineVertical = isVertical;

    bool fromVerToHor = isConeBaseVertical && !isLineVertical;
    bool fromHorToVer = !isConeBaseVertical && isLineVertical;

    bool needsAlterCond = (fromVerToHor && ((cone.Vectors[0].x < 0 && cone.Vectors[0].y < 0) || (cone.Vectors[0].x > 0 && cone.Vectors[0].y > 0))) ||
                          (fromHorToVer && ((cone.Vectors[0].x > 0 && cone.Vectors[0].y > 0) || (cone.Vectors[0].x < 0 && cone.Vectors[0].y < 0)));

    const unsigned lineDims = 2;
    for (unsigned i = 0; i < lineDims; i++)
    {
        float extendedVal = extendVectorToPlane(plane, cone.Points[i], cone.Vectors[i], isVertical);

        if (needsAlterCond && std::isinf(fabs(extendedVal)))
            extendedVal *= -1;

        if (!needsAlterCond && std::powf(-1, i) * extendedVal < std::powf(-1, i) * lineVals[i])
            return false;

        if (needsAlterCond && std::powf(-1, i) * extendedVal > std::powf(-1, i) * lineVals[(i + 1) % lineDims])
            return false;
    }

    return true;
}

PortalVisibility::ViewConeOrTunnel PortalVisibility::getExtendedConeToLine(ViewConeOrTunnel& old, std::vector<glm::vec2>& line, bool isVertical)
{
    ViewConeOrTunnel newCone;
    float plane = isVertical ? line[0].x : line[0].y;

    glm::vec2 coneEnd;

    const unsigned lineDims = 2;
    for (unsigned i = 0; i < lineDims; i++)
    {
        newCone.Points[i] = old.Points[i];

        //float n = extendVectorToPlane(plane, old.borderPoints[i], old.borderVectors[i], isVertical);
        newCone.Vectors[i] = line[(i + 1) % 2] - old.Points[i];

        // TODO kdybych se citil moc fancy
        // pravdepodobne to zase potrebuje alternativni podminku pro ty dva edgecasy :)

        //if ((std::powf(-1, i)) * n > std::powf(-1, i) * lineVals[(i + 1) % 2]) // is entire line in the cone?
        //{
        //    newCone.borderVectors[i] = line[(i + 1) % 2] - old.borderPoints[i];
        //}
        //else
        //{
        //    float x = isVertical ? line[0].x : n;
        //    float y = isVertical ? n : line[0].y;
        //    auto culledPoint = glm::vec2(x, y);
        //    newCone.borderVectors[i] = culledPoint - old.borderPoints[i];
        //}
    }

    return newCone;
}

Door& PortalVisibility::getDoorFromOtherPerspective(Room& currentRoom, Door& door)
{

    // TODO wrong, doesnt work with mutliple door between same rooms
    // always returns the first
    Room& neigbour = rooms[door.otherRoomId];

    for (auto& nDoor : neigbour.doors)
    {
        if (nDoor.otherRoomId == currentRoom.roomId)
            return nDoor;
    }

    throw "door is not part of the currentRoom";
}

bool PortalVisibility::isInBoundingBox(std::vector<glm::vec2>& boundingBox, std::vector<glm::vec2> corners)
{
    if (corners[0].x < boundingBox[0].x && corners[1].x < boundingBox[0].x)
        return false;

    if (corners[0].x > boundingBox[1].x && corners[1].x > boundingBox[1].x)
        return false;

    if (corners[0].y < boundingBox[0].y && corners[1].y < boundingBox[0].y)
        return false;

    if (corners[0].y > boundingBox[1].y && corners[1].y > boundingBox[1].y)
        return false;

    return true;
}

bool PortalVisibility::hasWallDoor(Room& room, float wallPlane, glm::vec2 wallBorderVals, bool isWallVertical)
{
    for (auto& door : room.doors)
    {
        bool isDoorVertical = isVertical(door);

        if (isDoorVertical != isWallVertical)
            continue;

        float doorPlane = isDoorVertical ? door.locations[0].x : door.locations[0].y;
        if (fabs(doorPlane - wallPlane) > eps)
            continue;

        glm::vec2 doorBorderVals;
        doorBorderVals[0] = isDoorVertical ? door.locations[0].y : door.locations[0].x;
        doorBorderVals[1] = isDoorVertical ? door.locations[1].y : door.locations[1].x;

        bool hasDoor =
            doorBorderVals[0] > wallBorderVals[0] && doorBorderVals[0] < wallBorderVals[1] &&
            doorBorderVals[1] > wallBorderVals[0] && doorBorderVals[1] < wallBorderVals[1];

        if (hasDoor)
            return true;
    }

    return false;
}

bool PortalVisibility::isWallBetweenDoors(Room& room, Door& first, Door& second)
{
    ViewConeOrTunnel tunnel = getViewConeOrTunnel(first.locations[0], first.locations[1], second.locations[0], second.locations[1], true);

    // shift vector back so that we can use extend without getting INF/-INF
    // doesnt matter which vector to use as the shift as they both have the same direction
    tunnel.Points[0] -= tunnel.Vectors[0];
    tunnel.Points[1] -= tunnel.Vectors[1];

    bool isFirstVertical = isVertical(first);
    float firstPlane = isFirstVertical ? first.locations[0].x : first.locations[0].y;

    bool isSecondVertical = isVertical(second);
    float secondPlane = isSecondVertical ? second.locations[0].x : second.locations[0].y;

    std::vector<glm::vec2> boundingBox(2);
    boundingBox[0].x = min({ first.locations[0].x, first.locations[1].x, second.locations[0].x, second.locations[1].x });
    boundingBox[0].y = min({ first.locations[0].y, first.locations[1].y, second.locations[0].y, second.locations[1].y });
    boundingBox[1].x = max({ first.locations[0].x, first.locations[1].x, second.locations[0].x, second.locations[1].x });
    boundingBox[1].y = max({ first.locations[0].y, first.locations[1].y, second.locations[0].y, second.locations[1].y });

    for (unsigned i = 0; i < room.corners.size(); i++)
    {
        unsigned nextId = (i + 1) % room.corners.size();

        if (!isInBoundingBox(boundingBox, { room.corners[i], room.corners[nextId] }))
            continue;

        bool isVertical = this->isVertical(room.corners[i], room.corners[nextId]);

        float firstVal = isVertical ? room.corners[i].y : room.corners[i].x;
        float secondVal = isVertical ? room.corners[nextId].y : room.corners[nextId].x;
        float plane = isVertical ? room.corners[i].x : room.corners[i].y;
        
        // skip walls which contain the doors
        if (isVertical == isFirstVertical && firstPlane == plane)
            continue;

        if (isVertical == isSecondVertical && secondPlane == plane)
            continue;

        glm::vec2 borderVals;
        borderVals[0] = min(firstVal, secondVal);
        borderVals[1] = max(firstVal, secondVal);

        if (hasWallDoor(room, plane, borderVals, isVertical))
            continue;

        float n1 = extendVectorToPlane(plane, tunnel.Points[0], tunnel.Vectors[0], isVertical);
        float n2 = extendVectorToPlane(plane, tunnel.Points[1], tunnel.Vectors[1], isVertical);

        if (n1 > borderVals[0] && n1 < borderVals[1] && n2 > borderVals[0] && n2 < borderVals[1])
            return true;
    }

    return false;
}

bool PortalVisibility::areDoorsInSamePlane(Door& first, Door& second)
{
    bool firstVertical = isVertical(first);
    bool secondVertical = isVertical(second);

    if (firstVertical != secondVertical)
        return false;

    return firstVertical ?
        first.locations[0].x == second.locations[0].x :
        first.locations[0].y == second.locations[0].y;
}

void PortalVisibility::addRoomsFromCone(std::unordered_set<unsigned>& visibleRooms, ViewConeOrTunnel& cone, Room& searchedRoom, Room& previousRoom, Door& entranceDoor, Door& initialDoor)
{
    for (auto& door : searchedRoom.doors)
    {
        if (door.otherRoomId == initialDoor.otherRoomId)
            continue;

        if (door.otherRoomId == previousRoom.roomId)
            continue;

        if (isWallBetweenDoors(searchedRoom, entranceDoor, door))
            continue;

        bool isVertical = this->isVertical(door);
        std::vector<glm::vec2> line = { door.locations[0], door.locations[1] };

        if (!isLineInCone(cone, line, isVertical))
            continue;

        //ViewCone newCone = getExtendedConeToLine(cone, line, isVertical);
        ViewConeOrTunnel newCone = getViewConeOrTunnel(cone.Points[0], cone.Points[1], line[0], line[1], false);

        visibleRooms.insert(door.otherRoomId);
        addRoomsFromCone(visibleRooms, newCone, rooms[door.otherRoomId], searchedRoom, door, initialDoor);
    }
}

std::vector<std::vector<unsigned>> PortalVisibility::getVisibilities()
{
    std::vector<std::vector<unsigned>> visibilities(rooms.size());

    #pragma omp parallel for schedule(dynamic, 16)
    for (int i = 0; i < rooms.size(); i++)
    {
        std::unordered_set<unsigned> visibleRooms;
        visibleRooms.insert(rooms[i].roomId);

        for (auto& door : rooms[i].doors)
        {
            Room& neighborRoom = rooms[door.otherRoomId];
            visibleRooms.insert(door.otherRoomId);

            for (auto& neighborDoor : neighborRoom.doors)
            {
                if (neighborDoor.otherRoomId == rooms[i].roomId)
                    continue;

                if (areDoorsInSamePlane(door, neighborDoor))
                    continue;

                if (isWallBetweenDoors(neighborRoom, door, neighborDoor))
                    continue;

                ViewConeOrTunnel viewCone = getViewConeOrTunnel(door.locations[0], door.locations[1], neighborDoor.locations[0], neighborDoor.locations[1], false);

                Room& searchedRoom = rooms[neighborDoor.otherRoomId];
                visibleRooms.insert(searchedRoom.roomId);
                
                addRoomsFromCone(visibleRooms, viewCone, searchedRoom, neighborRoom, neighborDoor, door);
            }
        }

        visibilities[i] = std::vector<unsigned>(visibleRooms.begin(), visibleRooms.end());
        //std::cerr << i << " / " << rooms.size() << std::endl;
    }

    return visibilities;
}
;