#include <iostream>
#include <MiniFB.h>
#include "Renderer.h"

int main() {
    std::cout << "Hello, Ray Trace!";

    Renderer renderer("scenes/scene06.xml", 1920, 1080, 10);
    renderer.Run();

    return 0;
}
