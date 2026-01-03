#include "app/Engine.h"
#include <iostream>
#include <exception>

int main()
{
    try
    {
        cozy::app::Engine engine;
        engine.Run();
    }
    catch (const std::exception &e)
    {
        // Caught errors from Renderer Init, Window creation, etc.
        std::cerr << "[FATAL ERROR]: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    catch (...)
    {
        std::cerr << "[FATAL ERROR]: An unknown error occurred." << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}