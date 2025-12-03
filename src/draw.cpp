#include "map_gen.hpp"

void MapGen::drawInit(double width)
{
    this->windowWidth = width;
    this->windowHeight = (double)((this->height * windowWidth) / this->width);

    cg::create_window("MapGen", windowWidth, windowHeight);
    cg::set_inactive_color(.1, .1, .1);

    cg::set_color(drawColors.defaultGrid);
    for (int i = 0; i < this->width + 1; i++)
    {
        cg::line((windowWidth / this->width) * i, 0, (windowWidth / this->width) * i, windowHeight);
        cg::line((windowWidth / this->width) * i + 1, 0, (windowWidth / this->width) * i + 1, windowHeight);
    }

    for (int i = 0; i < this->height + 1; i++)
    {
        cg::line(0, (windowHeight / this->height) * i, windowWidth - 1, (windowHeight / this->height) * i);
        cg::line(0, (windowHeight / this->height) * i + 1, windowWidth - 1, (windowHeight / this->height) * i + 1);
    }
}

void MapGen::drawTile(unsigned x, unsigned y)
{
    Tile& tile = getTile(x, y);
    if (!isTileInRoom(tile))
        return;

    double tileWidth = windowWidth / width;
    double tileHeight = windowHeight / height;
    double x1 = x * tileWidth;
    double y1 = y * tileHeight;

    cg::set_color(drawColors.floor);
    cg::set_fill_color(drawColors.floor);
    cg::rectangle(x1, y1, tileWidth, tileHeight);

    cg::set_color(drawColors.wall);
    if (hasTileAttrib(tile, TileAttrib::WallUp))
    {
        cg::line(x1, y1, x1 + tileWidth, y1);
    }

    if (hasTileAttrib(tile, TileAttrib::WallRight))
    {
        cg::line(x1 + tileWidth, y1, x1 + tileWidth, y1 + tileHeight);
    }

    if (hasTileAttrib(tile, TileAttrib::WallDown))
    {
        cg::line(x1, y1 + tileHeight, x1 + tileWidth, y1 + tileHeight);
    }

    if (hasTileAttrib(tile, TileAttrib::WallLeft))
    {
        cg::line(x1, y1, x1, y1 + tileHeight);
    }

    cg::set_color(drawColors.door);
    if (hasTileAttrib(tile, TileAttrib::DoorUp))
    {
        cg::line(x1 + tileWidth * 0.25, y1, x1 + tileWidth * 0.75, y1);
    }

    if (hasTileAttrib(tile, TileAttrib::DoorRight))
    {
        cg::line(x1 + tileWidth, y1 + tileHeight * 0.25, x1 + tileWidth, y1 + tileHeight * 0.75);
    }

    if (hasTileAttrib(tile, TileAttrib::DoorDown))
    {
        cg::line(x1 + tileWidth * 0.25, y1 + tileHeight, x1 + tileWidth * 0.75, y1 + tileHeight);
    }

    if (hasTileAttrib(tile, TileAttrib::DoorLeft))
    {
        cg::line(x1, y1 + tileHeight * 0.25, x1, y1 + tileHeight * 0.75);
    }
}

void MapGen::drawScheme(double width)
{
    drawInit(width);

    for (unsigned y = 0; y < height; y++)
    {
        for (unsigned x = 0; x < this->width; x++)
        {
            drawTile(x, y);
        }
    }

    cg::wait_until_closed();
}
