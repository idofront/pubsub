#ifndef IDOFRONT__TICKET__IDENTIFIABLE_HPP
#define IDOFRONT__TICKET__IDENTIFIABLE_HPP

#include <cstdint>

namespace idofront
{
namespace ticket
{
/// @brief Interface for an identifiable object
class Identifiable
{
  public:
    /// @brief Get the ID of the object
    inline virtual std::uint64_t Id() const = 0;

    /// @brief Compare two identifiable objects
    inline bool operator==(const Identifiable &other) const
    {
        return Id() == other.Id();
    }
};
} // namespace ticket
} // namespace idofront

#endif
