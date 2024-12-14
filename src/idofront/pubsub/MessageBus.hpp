#ifndef IDOFRONT__PUBSUB__MESSAGE_BUS_HPP
#define IDOFRONT__PUBSUB__MESSAGE_BUS_HPP

#include <idofront/pubsub/MessageBusInterface.hpp>
#include <idofront/pubsub/SingleTopicMessageBus.hpp>
#include <optional>
#include <unordered_map>

namespace idofront
{
namespace pubsub
{
template <typename T> class MessageBus : public MessageBusInterface<T>
{
  public:
    static std::shared_ptr<MessageBusInterface<T>> Create()
    {
        return std::shared_ptr<MessageBus<T>>(new MessageBus<T>());
    }

    virtual ~MessageBus() override = default;

    virtual void Publish(T message, const std::string &topic) override
    {
        auto messageBus = ResolveOrCreate(topic);
        messageBus->Publish(message);
    }

    virtual ticket::Stub Subscribe(std::function<void(T)> subscriber, const std::string &topic) override
    {
        auto messageBus = ResolveOrCreate(topic);
        return messageBus->Subscribe(subscriber);
    }

    virtual bool Unsubscribe(ticket::Stub stub, const std::string &topic) override
    {
        auto messageBusOpt = Resolve(topic);
        if (messageBusOpt.has_value())
        {
            return messageBusOpt.value()->Unsubscribe(stub);
        }

        return false;
    }

  private:
    MessageBus() = default;

    /// @brief Resolve message bug. If not exist, returns nullopt.
    /// @param topic Topic of the message bus
    /// @return Message bus
    /// @note
    /// If specified topic exits, return related message bus pointer. However, it is not known that the message bus
    /// pointer is valid or not.
    std::optional<std::shared_ptr<MessageBusInterface<T>>> Resolve(const std::string &topic)
    {
        auto it = _MessageBuses.find(topic);
        if (it != _MessageBuses.end())
        {
            return it->second;
        }

        return std::nullopt;
    }

    /// @brief Resolve message bug. If not exist, create a new one.
    /// @param topic Topic of the message bus
    /// @return Message bus
    std::shared_ptr<MessageBusInterface<T>> ResolveOrCreate(const std::string &topic)
    {
        auto messageBusOpt = Resolve(topic);
        if (messageBusOpt.has_value())
        {
            return messageBusOpt.value();
        }
        else
        {
            auto messageBus = SingleTopicMessageBus<T>::Create();
            _MessageBuses.emplace(topic, messageBus);
            return messageBus;
        }
    }

    std::unordered_map<std::string, std::shared_ptr<MessageBusInterface<T>>> _MessageBuses;
};
} // namespace pubsub
} // namespace idofront

#endif
