#include <idofront/argument/Parser.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        auto lengthArgument = idofront::argument::Argument<uint8_t>::New('l', "length")
                                  .Description("The length of the manager.")
                                  .Default(1);
        auto length = idofront::argument::Parse<uint8_t>(argc, argv, lengthArgument);
        auto messageArgument = idofront::argument::Argument<std::string>::New("message")
                                   .Description("The message to be published.")
                                   .Default("Hello, world!");
        auto message = idofront::argument::Parse<std::string>(argc, argv, messageArgument);
        auto help = idofront::argument::Parse<idofront::argument::type::Flag>(argc, argv, idofront::argument::Help);

        if (help.value())
        {
            std::cout << "Usage: " << argv[0] << " [options]\n";
            std::cout << "Options:\n";
            std::cout << lengthArgument.ToHelpString() << "\n";
            std::cout << messageArgument.ToHelpString() << "\n";
            std::cout << idofront::argument::Help.ToHelpString() << "\n";
            return 1;
        }

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
