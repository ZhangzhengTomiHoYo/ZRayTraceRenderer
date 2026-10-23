#include <iostream>
#include <MiniFB.h>
#include "Renderer.h"

int main() {
    std::cout << "Hello, Ray Trace!";

    Renderer renderer(1920, 1080);
    renderer.Run();

    return 0;
}