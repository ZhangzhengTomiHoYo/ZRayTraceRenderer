#include <iostream>
#include <MiniFB.h>
#include "Renderer.h"

int main() {
    std::cout << "Hello, Ray Trace!";

    Renderer renderer("scenes/scene_2.xml", 1920, 1080);
    renderer.Run();

    return 0;
}