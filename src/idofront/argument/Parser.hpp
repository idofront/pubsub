#ifndef IDOFRONT__ARGUMENT__PARSER_HPP
#define IDOFRONT__ARGUMENT__PARSER_HPP

#include <cctype>
#include <functional>
#include <locale>
#include <numeric>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
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
std::optional<T> Parse(int argc, char *argv[], const Argument<T> &argument,
                       std::function<T(const std::string &)> converter)
{
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

    for (int i = 0; i < argc; i++)
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

inline Argument<type::Flag> Help =
    Argument<type::Flag>::New('h', "help").Description("Show help message.").Default(type::Flag::False);

template <typename T> std::optional<T> Parse(int argc, char *argv[], const Argument<T> &argument)
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

    return Parse(argc, argv, argument, converter);
}

} // namespace argument
} // namespace idofront

#endif
