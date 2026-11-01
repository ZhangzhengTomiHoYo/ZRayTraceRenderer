#include <iostream>
#include <MiniFB.h>
#include "Renderer.h"

int main() {
    std::cout << "Hello, Ray Trace!";

    Renderer renderer("scenes/scene08.xml", 3, 10, 1920, 1080);
    renderer.Run();

    return 0;
}
