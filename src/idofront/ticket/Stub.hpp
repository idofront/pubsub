#ifndef IDOFRONT__TICKET__STUB_HPP
#define IDOFRONT__TICKET__STUB_HPP

#include <idofront/ticket/Identifiable.hpp>
#include <memory>

namespace idofront
{
namespace ticket
{

/// @brief Stub for a ticket
class Stub : public Identifiable
{
  public:
    friend class Receptor;

    /// @brief Get the ID of the object
    std::uint64_t Id() const
    {
        if (_Ptr.expired())
        {
            return 0;
        }

        auto sharedPtr = _Ptr.lock();
        auto ptr = sharedPtr.get();
        return reinterpret_cast<std::uint64_t>(ptr);
    }

    /// @brief Check if the stub is expired
    bool IsExpired() const
    {
        return _Ptr.expired();
    }

  private:
    Stub(std::shared_ptr<void> ptr) : _Ptr(ptr)
    {
    }
    std::weak_ptr<void> _Ptr;
};

} // namespace ticket
} // namespace idofront

#endif
