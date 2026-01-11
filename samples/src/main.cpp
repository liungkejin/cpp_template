#include <iostream>
#include "Platform.h"


int main() {
    std::cout << "Platform: " << Platform::name() << std::endl;
    std::cout << Platform::hello() << std::endl;
    return 0;
}