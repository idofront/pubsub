#ifndef IDOFRONT__PUBSUB__ASYNC_MESSAGE_BUS_INTERFACE_HPP
#define IDOFRONT__PUBSUB__ASYNC_MESSAGE_BUS_INTERFACE_HPP

#include <idofront/pubsub/MessageBusInterface.hpp>

namespace idofront
{
namespace pubsub
{
/// @brief Interface for a message bus
/// @tparam T
template <typename T> class AsyncMessageBusInterface : public MessageBusInterface<T>
{
  public:
    virtual ~AsyncMessageBusInterface() = default;
};
} // namespace pubsub
} // namespace idofront

#endif
