#include <iostream>
#include <MiniFB.h>
#include "Renderer.h"

int main() {
    std::cout << "Hello, Ray Trace!";

    Renderer renderer(100, 100);
    renderer.Run();

    return 0;
}