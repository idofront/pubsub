#include <idofront/argument/Parser.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        auto length = idofront::argument::Parse<int>(
            argc, argv,
            idofront::argument::Argument<int>::New('l', "length").Description("The length of the manager.").Default(1));
        std::cout << "Length: " << length.value() << "\n" << std::flush;
    }
    catch (const std::exception &e)
    {
        auto message = std::string("Error: ") + e.what() + "\n";
        std::cerr << message << std::flush;
        return 1;
    }

    return 0;
}
