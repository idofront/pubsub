#ifndef IDOFRONT__ARGUMENT__PARSER_HPP
#define IDOFRONT__ARGUMENT__PARSER_HPP

#include <cctype>
#include <functional>
#include <iostream>
#include <locale>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace idofront
{
namespace argument
{
namespace type
{
class Flag
{
  public:
    Flag(bool value = false) : _Value(value)
    {
    }
    operator bool() const
    {
        return _Value;
    }
    static const Flag True;
    static const Flag False;
    const std::string ToString() const
    {
        return _Value ? "true" : "false";
    }

  private:
    bool _Value;
};

const Flag Flag::True = Flag(true);
const Flag Flag::False = Flag(false);
} // namespace type

template <typename T> class Argument
{
  public:
    static Argument New(const char &shortName, const std::string &name)
    {
        return Argument(std::string(1, shortName), name);
    }

    static Argument New(const std::string &name)
    {
        return Argument(name);
    }

    static Argument Update(const Argument &argument, const std::optional<T> &value)
    {
        auto updatedArgument = argument;
        updatedArgument._Value = value;
        return updatedArgument;
    }

    std::string Name() const
    {
        return _Name;
    }

    std::string ShortName() const
    {
        return _ShortName;
    }

    Argument Description(const std::string &description)
    {
        _Description = description;
        return *this;
    }

    std::string Description() const
    {
        return _Description;
    }

    Argument Default(const T &defaultValue)
    {
        _DefaultValue = defaultValue;
        return *this;
    }

    std::optional<T> Default() const
    {
        return _DefaultValue;
    }

    Argument IsRequired(bool isRequired = true)
    {
        _IsRequired = isRequired;
        return *this;
    }

    bool IsRequired() const
    {
        return _IsRequired;
    }

    std::optional<T> Value() const
    {
        return this->_Value.has_value() ? this->_Value : _DefaultValue;
    }

    const std::string ToHelpString() const
    {
        auto upperCaseName = std::string(_Name);
        std::transform(upperCaseName.begin(), upperCaseName.end(), upperCaseName.begin(),
                       [](char const &c) { return std::toupper(c); });

        auto isFlag = std::is_same_v<T, type::Flag>;

        std::stringstream ss;
        ss << "  ";
        if (!_ShortName.empty())
        {
            ss << "-" << _ShortName << ", ";
        }
        ss << "--" << _Name;
        if (!isFlag)
        {
            ss << " <" << upperCaseName << ">";
        }
        ss << "\n";
        ss << "    " << _Description;
        ss << " (Default: " << _DefaultValue << ")";
        return ss.str();
    }

  private:
    std::string _Name;
    std::string _ShortName;
    std::string _Description;
    T _DefaultValue;
    bool _IsRequired;
    std::optional<T> _Value;

    Argument(const std::string &shortName, const std::string &name)
        : _Name(name), _ShortName(shortName), _Description(), _IsRequired(false)
    {
    }

    Argument(const std::string &name) : Argument("", name)
    {
    }

    friend class Parser;
};

template <typename T>
std::optional<T> Parse(std::vector<std::string> argv, const Argument<T> &argument,
                       std::function<T(const std::string &)> converter)
{
    std::size_t argc = argv.size();

    auto argumentName = argument.Name();
    auto actualValue = std::string();

    auto isArgumentMatched = [](const Argument<T> &argument, const std::string &currentArgumentName) {
        switch (currentArgumentName.size())
        {
        case 0:
        case 1:
            return false;
        case 2:
            if (argument.ShortName().empty())
            {
                return false;
            }
            return ("-" + argument.ShortName()) == currentArgumentName;
        default:
            return ("--" + argument.Name()) == currentArgumentName;
        }
    };

    for (auto i = std::size_t(0); i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            auto currentArgumentName = std::string(argv[i]);
            if (isArgumentMatched(argument, currentArgumentName))
            {
                if (std::is_same_v<T, type::Flag>)
                {
                    actualValue = type::Flag::True.ToString();
                }
                else
                {
                    auto nextValueExists = (i + 1) < argc;
                    if (nextValueExists)
                    {
                        actualValue = std::string(nextValueExists ? argv[++i] : "");
                        continue;
                    }
                    else
                    {
                        throw std::invalid_argument("Argument \"" + argumentName + "\" requires a value.");
                    }
                }
            }
        }
    }

    if (actualValue.empty())
    {
        if (argument.IsRequired())
        {
            std::stringstream ss;
            ss << "Argument \"" << argumentName << "\" is required.";
            auto message = ss.str();
            throw std::invalid_argument(message);
        }

        return argument.Default();
    }

    try
    {
        return converter(actualValue);
    }
    catch (const std::exception &e)
    {
        std::stringstream ss;
        ss << "Failed to parse the argument \"" << argumentName << "\": " << e.what();
        auto message = ss.str();
        throw std::invalid_argument(message);
    }
}

template <typename T>
std::optional<T> Parse(int argc, char *argv[], const Argument<T> &argument,
                       std::function<T(const std::string &)> converter)
{
    return Parse(std::vector<std::string>(argv, argv + argc), argument, converter);
}

inline Argument<type::Flag> Help =
    Argument<type::Flag>::New('h', "help").Description("Show help message.").Default(type::Flag::False);

template <typename T> std::optional<T> Parse(std::vector<std::string> argv, const Argument<T> &argument)
{
    static_assert(std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
                      std::is_same_v<T, int64_t> || std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                      std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, float> ||
                      std::is_same_v<T, double> || std::is_same_v<T, long double> || std::is_same_v<T, std::string> ||
                      std::is_same_v<T, bool> || std::is_same_v<T, type::Flag>,
                  "Type not supported automatically. Please provide a converter function.");

    auto getConverter = std::function<std::function<T(const std::string &)>(const Argument<T> &)>(
        [](const Argument<T> &) -> std::function<T(const std::string &)> {
            if constexpr (std::is_same_v<T, int8_t> || std::is_same_v<T, int16_t> || std::is_same_v<T, int32_t> ||
                          std::is_same_v<T, int64_t>)
            {
                return [](const std::string &value) { return std::stoll(value); };
            }
            else if constexpr (std::is_same_v<T, uint8_t> || std::is_same_v<T, uint16_t> ||
                               std::is_same_v<T, uint32_t> || std::is_same_v<T, uint64_t>)
            {
                return [](const std::string &value) { return std::stoull(value); };
            }
            else if constexpr (std::is_same_v<T, float>)
            {
                return [](const std::string &value) { return std::stof(value); };
            }
            else if constexpr (std::is_same_v<T, double>)
            {
                return [](const std::string &value) { return std::stod(value); };
            }
            else if constexpr (std::is_same_v<T, long double>)
            {
                return [](const std::string &value) { return std::stold(value); };
            }
            else if constexpr (std::is_same_v<T, std::string>)
            {
                return [](const std::string &value) { return value; };
            }
            else if constexpr (std::is_same_v<T, bool>)
            {
                return [](const std::string &value) {
                    if (value == "true")
                    {
                        return true;
                    }
                    else if (value == "false")
                    {
                        return false;
                    }
                    else
                    {
                        try
                        {
                            auto integerValue = std::stoll(value);
                            return static_cast<bool>(integerValue);
                        }
                        catch (const std::exception &e)
                        {
                            throw std::invalid_argument("Value must be either \"true\"(1) or \"false\"(0).");
                        }
                    }
                };
            }
            else if constexpr (std::is_same_v<T, type::Flag>)
            {
                return [](const std::string &) { return type::Flag::True; };
            }
            else
            {
                throw std::invalid_argument("Type not supported.");
            }
        });

    auto converter = getConverter(argument);

    return Parse(argv, argument, converter);
}

template <typename T> std::optional<T> Parse(int argc, char *argv[], const Argument<T> &argument)
{
    return Parse(std::vector<std::string>(argv, argv + argc), argument);
}

#include <unordered_map>
class ArgumentInformation
{
  public:
    template <typename T> static ArgumentInformation New(const Argument<T> &argument)
    {
        auto isFlag = std::is_same_v<T, type::Flag>;
        return ArgumentInformation(argument.Name(), argument.ShortName(), argument.Description(), isFlag,
                                   argument.IsRequired());
    }

    ArgumentInformation(const std::string &name, const std::string &shortName, const std::string &description,
                        bool isFlag, bool isRequired)
        : _Name(name), _ShortName(shortName), _Description(description), _IsFlag(isFlag), _IsRequired(isRequired)
    {
    }

    std::string Name() const
    {
        return _Name;
    }

    std::string ShortName() const
    {
        return _ShortName;
    }

    std::string Description() const
    {
        return _Description;
    }

    bool IsFlag() const
    {
        return _IsFlag;
    }

    bool IsRequired() const
    {
        return _IsRequired;
    }

  private:
    std::string _Name;
    std::string _ShortName;
    std::string _Description;
    bool _IsFlag;
    bool _IsRequired;
};

class Parser
{
  public:
    Parser(int argc, char *argv[]) : _ArgumentValues(std::vector<std::string>(argv, argv + argc))
    {
    }

    template <typename T> Argument<T> Parse(const Argument<T> &argument)
    {
        auto information = ArgumentInformation::New(argument);
        _ArgumentInformations.push_back(information);

        auto result = ::idofront::argument::Parse(_ArgumentValues, argument);
        return result.has_value() ? idofront::argument::Argument<T>::Update(argument, result) : argument;
    }

    void ShowHelp(std::size_t maxWidth = 80)
    {
        auto helpLines = std::vector<std::string>{
            "Usage: " + _ArgumentValues[0] + " [options]",
            "Options:",
        };

        std::vector<std::tuple<std::string, std::string>> argumentHelps;
        std::transform(_ArgumentInformations.begin(), _ArgumentInformations.end(), std::back_inserter(argumentHelps),
                       [](const ArgumentInformation &information) {
                           auto upperCaseName = std::string(information.Name());
                           std::transform(upperCaseName.begin(), upperCaseName.end(), upperCaseName.begin(),
                                          [](char const &c) { return std::toupper(c); });

                           auto helpString =
                               "--" + information.Name() +
                               (information.ShortName().empty() ? "" : " (-" + information.ShortName() + ")");
                           if (!information.IsFlag())
                           {
                               helpString += " <" + upperCaseName + ">";
                           }

                           return std::make_tuple(helpString, information.Description());
                       });

        auto keysMaxLength = std::size_t(std::accumulate(argumentHelps.begin(), argumentHelps.end(), 0,
                                                         [](std::size_t l, std::tuple<std::string, std::string> tuple) {
                                                             auto helpString = std::get<0>(tuple);
                                                             return std::max(l, helpString.size());
                                                         }));

        auto isOneLine = keysMaxLength < (maxWidth / 3);

        std::for_each(argumentHelps.begin(), argumentHelps.end(), [&](std::tuple<std::string, std::string> tuple) {
            auto helpString = std::get<0>(tuple);
            auto description = std::get<1>(tuple);

            if (isOneLine)
            {
                auto padding = std::string(keysMaxLength - helpString.size(), ' ');
                auto helpLine = helpString + padding + "  " + description;
                // TODO: Wrap the description if it is too long.
                helpLines.push_back(helpLine);
            }
            else
            {
                helpLines.push_back(helpString);
                helpLines.push_back("  " + description);
                // TODO: Wrap the description if it is too long.
            }
        });

        std::for_each(helpLines.begin(), helpLines.end(), [](const std::string &line) {
            std::cout << line << "\n" << std::flush;
        });
    }

  private:
    std::vector<std::string> _ArgumentValues;
    std::vector<ArgumentInformation> _ArgumentInformations;
};

} // namespace argument
} // namespace idofront

#endif
