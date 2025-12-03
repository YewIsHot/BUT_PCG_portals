#include <iostream>
#include "map_gen.hpp"
#include "gl_scene.hpp"
#include "portal_visibility.hpp"

int main()
{
    unsigned seed = (unsigned)time(0);
    std::cerr << seed << std::endl << std::endl;
    srand(seed);

    MapGen mapGen = MapGen(50, 50);

    mapGen.generate();
    mapGen.drawScheme(1000.);

    PortalVisibility portal = PortalVisibility::getFromMap(&mapGen);
    auto visibilities = portal.getVisibilities();

    auto scene = GLScene::create(2560.f, 1440.f, &mapGen, visibilities);
    scene.run();

    

    return 0;
}