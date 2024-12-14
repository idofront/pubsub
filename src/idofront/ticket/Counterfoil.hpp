#ifndef IDOFRONT__TICKET__COUNTERFOIL_HPP
#define IDOFRONT__TICKET__COUNTERFOIL_HPP

#include <idofront/ticket/Identifiable.hpp>
#include <memory>

namespace idofront
{
namespace ticket
{

/// @brief Counterfoil for a ticket
/// @tparam T
template <class T> class Counterfoil : public Identifiable
{
  public:
    friend class Receptor;

    /// @brief Get the pointer
    std::shared_ptr<T> Ptr() const
    {
        return _Ptr;
    }

    /// @brief Get the ID of the object
    std::uint64_t Id() const
    {
        auto ptr = _Ptr.get();
        return reinterpret_cast<std::uint64_t>(ptr);
    }

  private:
    Counterfoil(std::shared_ptr<T> ptr) : _Ptr(ptr)
    {
    }
    std::shared_ptr<T> _Ptr;
};

} // namespace ticket
} // namespace idofront

#endif
