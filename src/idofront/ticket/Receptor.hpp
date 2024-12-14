#ifndef IDOFRONT__TICKET__RECEPTOR_HPP
#define IDOFRONT__TICKET__RECEPTOR_HPP

#include <idofront/ticket/Counterfoil.hpp>
#include <idofront/ticket/Stub.hpp>
#include <tuple>

namespace idofront
{
namespace ticket
{

/// @brief Receptor for creating a stub and counterfoil
class Receptor
{
  public:
    /// @brief Create a stub and counterfoil
    template <class T> static std::tuple<Stub, Counterfoil<T>> Recept(std::shared_ptr<T> ptr)
    {
        return std::make_tuple(Stub(ptr), Counterfoil<T>(ptr));
    }
};

} // namespace ticket
} // namespace idofront

#endif
