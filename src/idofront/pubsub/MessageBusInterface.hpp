#ifndef IDOFRONT__PUBSUB__MESSAGE_BUS_INTERFACE_HPP
#define IDOFRONT__PUBSUB__MESSAGE_BUS_INTERFACE_HPP

#include <functional>
#include <idofront/ticket/Stub.hpp>

namespace idofront
{
namespace pubsub
{
/// @brief Interface for a message bus
/// @tparam T
template <typename T> class MessageBusInterface
{
  public:
    virtual ~MessageBusInterface() = default;

    /// @brief Publish a message
    /// @param message Message to publish
    virtual void Publish(T message, const std::string &topic = "") = 0;

    /// @brief Subscribe to the message bus
    /// @param subscriber Subscriber function
    /// @return Stub to the subscriber
    virtual ticket::Stub Subscribe(std::function<void(T)> subscriber, const std::string &topic = "") = 0;

    /// @brief Unsubscribe from the message bus
    /// @param stub Stub to unsubscribe
    /// @return True if the subscriber was unsubscribed, false otherwise
    virtual bool Unsubscribe(ticket::Stub stub, const std::string &topic = "") = 0;
};
} // namespace pubsub
} // namespace idofront

#endif
