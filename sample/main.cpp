#include "App.h"
#include <iostream>

int main()
{
    try {
        App application(1280, 720);
        application.Run();
    }
    catch (std::exception e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}