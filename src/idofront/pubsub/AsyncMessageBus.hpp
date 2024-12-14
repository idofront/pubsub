#ifndef IDOFRONT__PUBSUB__ASYNC_MESSAGE_BUS_HPP
#define IDOFRONT__PUBSUB__ASYNC_MESSAGE_BUS_HPP

#include <idofront/pubsub/AsyncMessageBusInterface.hpp>
#include <idofront/pubsub/AsyncSingleTopicMessageBus.hpp>
#include <optional>
#include <unordered_map>

namespace idofront
{
namespace pubsub
{
template <typename T> class AsyncMessageBus : public AsyncMessageBusInterface<T>
{
  public:
    static std::shared_ptr<AsyncMessageBusInterface<T>> Create()
    {
        return std::shared_ptr<AsyncMessageBus<T>>(new AsyncMessageBus<T>());
    }

    virtual ~AsyncMessageBus() override = default;

    virtual void Publish(T message, const std::string &topic) override
    {
        auto messageBus = ResolveOrCreate(topic);
        messageBus->Publish(message);
    }

    virtual std::future<void> PublishAsync(T message, const std::string &topic) override
    {
        auto messageBus = ResolveOrCreate(topic);
        return messageBus->PublishAsync(message, topic);
    }

    virtual ticket::Stub Subscribe(std::function<void(T)> subscriber, const std::string &topic) override
    {
        auto messageBus = ResolveOrCreate(topic);
        return messageBus->Subscribe(subscriber);
    }

    virtual std::future<ticket::Stub> SubscribeAsync(std::function<void(T)> subscriber,
                                                     const std::string &topic) override
    {
        auto messageBus = ResolveOrCreate(topic);
        return messageBus->SubscribeAsync(subscriber, topic);
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

    virtual std::future<bool> UnsubscribeAsync(ticket::Stub stub, const std::string &topic) override
    {
        auto messageBusOpt = Resolve(topic);
        if (messageBusOpt.has_value())
        {
            return messageBusOpt.value()->UnsubscribeAsync(stub);
        }

        return std::async(std::launch::deferred, []() { return false; });
    }

  private:
    AsyncMessageBus() = default;

    /// @brief Resolve message bug. If not exist, returns nullopt.
    /// @param topic Topic of the message bus
    /// @return Message bus
    /// @note
    /// If specified topic exits, return related message bus pointer. However, it is not known that the message bus
    /// pointer is valid or not.
    std::optional<std::shared_ptr<AsyncMessageBusInterface<T>>> Resolve(const std::string &topic)
    {
        std::unique_lock<std::mutex> lock(_MessageBussesMutex);
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
    std::shared_ptr<AsyncMessageBusInterface<T>> ResolveOrCreate(const std::string &topic)
    {
        std::unique_lock<std::mutex> lock(_MessageBussesMutex);
        auto messageBusOpt = Resolve(topic);
        if (messageBusOpt.has_value())
        {
            return messageBusOpt.value();
        }
        else
        {
            auto messageBus = AsyncSingleTopicMessageBus<T>::Create();
            _MessageBuses.emplace(topic, messageBus);
            return messageBus;
        }
    }

    std::unordered_map<std::string, std::shared_ptr<AsyncMessageBusInterface<T>>> _MessageBuses;
    std::mutex _MessageBussesMutex;

    AsyncMessageBus(const AsyncMessageBus &) = delete;
    AsyncMessageBus &operator=(const AsyncMessageBus &) = delete;
    AsyncMessageBus(AsyncMessageBus &&) = delete;
    AsyncMessageBus &operator=(AsyncMessageBus &&) = delete;
};
} // namespace pubsub
} // namespace idofront

#endif
