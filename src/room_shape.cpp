#include "map_gen.hpp"

const RoomShape RoomShapeFactory::I = { {0, 0}, {0, 1}, {0, 2}, {0, 3} };
const RoomShape RoomShapeFactory::L = { {0, 0}, {0, 1}, {0, 2}, {1, 2} };
const RoomShape RoomShapeFactory::O = { {0, 0}, {1, 0}, {0, 1}, {1, 1} };
const RoomShape RoomShapeFactory::z = { {0, 0}, {1, 0}, {1, 1}, {2, 1} };
const RoomShape RoomShapeFactory::Z = { {0, 0}, {1, 0}, {1, 1}, {1, 2}, {2, 2} };
const RoomShape RoomShapeFactory::T = { {0, 0}, {1, 0}, {2, 0}, {1, 1} };
const RoomShape RoomShapeFactory::U = { {0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}, {2, 1 }, {2, 0} };
const RoomShape RoomShapeFactory::X = { {0, 0}, {-1, 1},{0, 1}, {1, 1}, {0, 2} };
const RoomShape RoomShapeFactory::W = { {0, 0}, {0, 1}, {1, 1}, {1, 2}, {2, 2} };
const RoomShape RoomShapeFactory::H = { {0, 0}, {1, 0}, {2, 0}, {1, 1}, {0, 2}, {1, 2}, {2, 2} };

RoomShape::RoomShape()
{
    segments = std::vector<Point>();
}

RoomShape::RoomShape(vector<Point> segments)
{
    this->segments = segments;
}

RoomShape::RoomShape(initializer_list<Point> segments)
{
    this->segments = segments;
}

RoomShape RoomShape::deepCopy() const
{
    RoomShape newRoom = RoomShape();

    newRoom.config = this->config;
    newRoom.segments = std::vector<Point>(this->segments.size());
    std::copy(this->segments.begin(), this->segments.end(), newRoom.segments.begin());

    return newRoom;
}

RoomShape& RoomShape::rotate90(bool toRight)
{
    for (Point& seg : segments)
    {
        if (toRight)
            seg.rotate90R();
        else
            seg.rotate90L();
    }

    return *this;
}

RoomShape& RoomShape::translate(int x, int y)
{
    for (Point& seg : segments)
    {
        seg.translate(x, y);
    }

    return *this;
}

RoomShape& RoomShape::invert()
{
    for (Point& seg : segments)
    {
        seg.invert(true);
    }

    return *this;
}

RoomShape RoomShape::getConfig(unsigned configId) const
{
    assert(config == 0);
    assert(configId < N_CONFIGS);

    RoomShape newRoom = deepCopy();

    for (int i = 0; i < configId; i++)
    {
        newRoom.config++;
        newRoom.rotate90(true);

        if (newRoom.config == 4)
            newRoom.invert();
    }
    
    return newRoom;
}

RoomShape& RoomShape::recalculateOrigin()
{
    Point tmpOrigin = { INT_MAX, INT_MAX };

    for (Point& seg : segments)
    {
        if ((seg.y < tmpOrigin.y) || (seg.y == tmpOrigin.y && seg.x < tmpOrigin.x))
            tmpOrigin = seg;
    }

    tmpOrigin.x = abs(tmpOrigin.x);
    tmpOrigin.y = abs(tmpOrigin.y);
    translate(tmpOrigin.x, tmpOrigin.y);

    auto key = [](const Point& a, const Point& b)
    {
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
    };

    sort(segments.begin(), segments.end(), key);

    return *this;
}

string RoomShape::to_string()
{
    std::string res;
    for (Point& seg : segments)
    {
        res += seg.to_string() + ", ";
    }

    res.pop_back();
    res.pop_back();
    return res;
}