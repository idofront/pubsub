#include <idofront/argument/Parser.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        auto length = idofront::argument::Parse<uint8_t>(argc, argv,
                                                         idofront::argument::Argument<uint8_t>::New('l', "length")
                                                             .Description("The length of the manager.")
                                                             .Default(1));
        auto message = idofront::argument::Parse<std::string>(argc, argv,
                                                              idofront::argument::Argument<std::string>::New("message")
                                                                  .Description("The message to be published.")
                                                                  .Default("Hello, world!"));
        std::cout << "Length: " << (uint64_t)length.value() << "\n" << std::flush;
        std::cout << "Message: " << message.value() << "\n" << std::flush;
    }
    catch (const std::exception &e)
    {
        auto message = std::string("Error: ") + e.what() + "\n";
        std::cerr << message << std::flush;
        return 1;
    }

    return 0;
}
