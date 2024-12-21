#include <idofront/argument/Parser.hpp>
#include <iostream>

int main(int argc, char *argv[])
{
    try
    {
        auto parser = idofront::argument::Parser(argc, argv);

        auto lengthArgument = idofront::argument::Argument<uint8_t>::New('l', "length")
                                  .Description("The length of the manager.")
                                  .Default(1);
        auto length = parser.Parse(lengthArgument);

        auto messageArgument = idofront::argument::Argument<std::string>::New("message")
                                   .Description("The message to be published.")
                                   .Default("Hello, world!");
        auto message = parser.Parse(messageArgument);

        auto help = parser.Parse<idofront::argument::type::Flag>(idofront::argument::Help);

        if (help.Value().value())
        {
            parser.ShowHelp();
            return 0;
        }

        std::cout << "Length: " << (uint64_t)length.Value().value() << "\n" << std::flush;
        std::cout << "Message: " << message.Value().value() << "\n" << std::flush;
    }
    catch (const std::exception &e)
    {
        auto message = std::string("Error: ") + e.what() + "\n";
        std::cerr << message << std::flush;
        return 1;
    }

    return 0;
}
