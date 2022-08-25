#include <iostream>

int main(int argc, char *argv[])
{
    while (true)
    {
        std::string command;
        std::cout << "uCommand$";
        std::cin >> command;
        if (command == "quit")
        {
            break;
        }
        else if (command == "connect")
        {
            // Get the module name
            
        }
        else
        {
            if (command != "help")
            {
                std::cout << "Unhandled command: " << command << std::endl;
            }
            std::cout << "Options:" << std::endl;
            std::cout << "   help                  Prints this message." << std::endl;
            std::cout << "   connect [ModuleName]  Connects to the module." << std::endl;
            std::cout << "   quit                  Exits this application." << std::endl;
        }
    }
    return EXIT_SUCCESS;
}
