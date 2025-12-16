#include "Physics.hpp"
#include "Renderer.hpp"
#include <iostream>

int main(int argc, char** argv)
{
    Physics physics;

    physics.initialize();

    Renderer renderer(&physics);

    try
    {
        renderer.run(argc, argv);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Wyjątek podczas uruchamiania: " << e.what() << '\n';
        return 1;
    }

    physics.cleanup();

    return 0;
}