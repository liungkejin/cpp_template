#include <iostream>
#include "Platform.h"


int main() {
    std::cout << "Hello, World!" << std::endl;
    std::cout << "Platform: " << Platform::name() << std::endl;
    return 0;
}