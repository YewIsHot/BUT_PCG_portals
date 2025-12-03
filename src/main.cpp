#include <iostream>
#include "map_gen.hpp"
#include "gl_scene.hpp"
#include "portal_visibility.hpp"

int main()
{
    unsigned seed = (unsigned)time(0);
    std::cerr << seed << std::endl << std::endl;
    srand(seed);
    //srand(1764623472);
    //srand(5);
    //srand(1764615030);

    MapGen mapGen = MapGen(50, 50);

    //if (!mapGen.createCustom({
    //    RoomShapeFactory::L.getConfig(1),
    //    RoomShapeFactory::U.deepCopy().rotate90().recalculateOrigin().translate(1, 1),
    //    RoomShapeFactory::I.getConfig(0).translate(2,4),
    //    //RoomShapeFactory::I.getConfig(0).translate(3,4),
    //    }))
    //{
    //    std:cerr << "Map create failed" << std::endl;
    //    exit(1);
    //}
    //mapGen.addDoor(TileAttrib::DoorDown, 1, 0);
    //mapGen.addDoor(TileAttrib::DoorRight, 0, 1);
    //mapGen.addDoor(TileAttrib::DoorUp, 2, 4);
    //mapGen.addDoor(TileAttrib::DoorUp, 3, 4);

    //if (!mapGen.createCustom({
    //    RoomShapeFactory::I.getConfig(0),
    //    RoomShapeFactory::O.getConfig(0).translate(1, 2),
    //    RoomShapeFactory::T.getConfig(0).translate(0, 4),
    //    RoomShapeFactory::z.getConfig(1).translate(2, 5),
    //    RoomShapeFactory::X.getConfig(0).translate(2, 7),
    //    RoomShapeFactory::W.getConfig(0).translate(3, 9)
    //    }))
    //{
    //    std:cerr << "Map create failed" << std::endl;
    //    exit(1);
    //}

    //mapGen.addDoor(TileAttrib::DoorRight, 0, 2);
    //mapGen.addDoor(TileAttrib::DoorUp, 0, 4);
    //mapGen.addDoor(TileAttrib::DoorRight, 1, 5);
    //mapGen.addDoor(TileAttrib::DoorDown, 2, 4);
    //mapGen.addDoor(TileAttrib::DoorRight, 1, 7);
    //mapGen.addDoor(TileAttrib::DoorDown, 2, 6);
    //mapGen.addDoor(TileAttrib::DoorRight, 2, 9);
    //mapGen.addDoor(TileAttrib::DoorDown, 3, 8);

    //if (!mapGen.createCustom({
    //    RoomShapeFactory::I.getConfig(1).recalculateOrigin(),
    //    RoomShapeFactory::I.getConfig(1).recalculateOrigin().translate(0, 1),
    //    RoomShapeFactory::I.getConfig(0).translate(0, 2),
    //    RoomShapeFactory::I.getConfig(0).translate(1, 2),
    //    }))
    //{
    //    std:cerr << "Map create failed" << std::endl;
    //    exit(1);
    //}

    //mapGen.addDoor(TileAttrib::DoorDown, 0, 0);
    //mapGen.addDoor(TileAttrib::DoorDown, 0, 1);
    //mapGen.addDoor(TileAttrib::DoorRight, 0, 2);
    //mapGen.addDoor(TileAttrib::DoorRight, 0, 3);
    //mapGen.addDoor(TileAttrib::DoorRight, 0, 4);
    //mapGen.addDoor(TileAttrib::DoorRight, 0, 5);

    mapGen.generate();
    mapGen.drawScheme(1000.);

    PortalVisibility portal = PortalVisibility::getFromMap(&mapGen);
    auto visibilities = portal.getVisibilities();

    auto scene = GLScene::create(2560.f, 1440.f, &mapGen, visibilities);
    scene.run();

    

    return 0;
}