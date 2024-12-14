#ifndef IDOFRONT__PUBSUB__ASYNC_MESSAGE_BUS_INTERFACE_HPP
#define IDOFRONT__PUBSUB__ASYNC_MESSAGE_BUS_INTERFACE_HPP

#include <future>
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
    virtual std::future<void> PublishAsync(T message, const std::string &topic = "") = 0;
    virtual std::future<ticket::Stub> SubscribeAsync(std::function<void(T)> subscriber,
                                                     const std::string &topic = "") = 0;
    virtual std::future<bool> UnsubscribeAsync(ticket::Stub stub, const std::string &topic = "") = 0;
};
} // namespace pubsub
} // namespace idofront

#endif
