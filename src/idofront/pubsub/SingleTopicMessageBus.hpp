#ifndef IDOFRONT__PUBSUB__SINGLE_TOPIC_MESSAGE_BUS_HPP
#define IDOFRONT__PUBSUB__SINGLE_TOPIC_MESSAGE_BUS_HPP

#include <functional>
#include <idofront/pubsub/MessageBusInterface.hpp>
#include <idofront/ticket/Counterfoil.hpp>
#include <idofront/ticket/Receptor.hpp>
#include <iostream>
#include <memory>
#include <tuple>
#include <vector>

namespace idofront
{
namespace pubsub
{

/// @brief Just a simple message bus that can only handle one topic
template <typename T> class SingleTopicMessageBus : public MessageBusInterface<T>
{
  public:
    static std::shared_ptr<MessageBusInterface<T>> Create()
    {
        return std::shared_ptr<SingleTopicMessageBus<T>>(new SingleTopicMessageBus<T>());
    }

    virtual ~SingleTopicMessageBus() override
    {
    }

    /// @brief Publish a message.
    /// @param message Message to publish
    /// @param topic Topic of the message. Ignored in this implementation.
    virtual void Publish(T message, const std::string &) override
    {
        std::for_each(_Counterfoils.begin(), _Counterfoils.end(),
                      [message](ticket::Counterfoil<std::function<void(T)>> counterfoil) {
                          auto counterfoilPtr = counterfoil.Ptr();
                          auto counterfoilFunc = std::dynamic_pointer_cast<std::function<void(T)>>(counterfoilPtr);
                          (*counterfoilFunc)(message);
                      });
    }

    /// @brief Subscribe to the message bus.
    /// @param subscriber Subscriber function
    /// @param topic Topic of the message. Ignored in this implementation.
    /// @return Stub to the subscriber
    virtual ticket::Stub Subscribe(std::function<void(T)> subscriber, const std::string &) override
    {
        auto subscriberPtr = std::make_shared<std::function<void(T)>>(subscriber);
        auto ticket = ticket::Receptor::Recept(subscriberPtr);

        // separate the stub and counterfoil
        auto stub = std::get<0>(ticket);
        auto counterfoil = std::get<1>(ticket);

        _Counterfoils.push_back(counterfoil);
        return stub;
    }

    /// @brief Unsubscribe from the message bus.
    /// @param stub Stub to unsubscribe
    /// @param topic Topic of the message. Ignored in this implementation.
    /// @return True if the subscriber was unsubscribed, false otherwise
    virtual bool Unsubscribe(ticket::Stub stub, const std::string &) override
    {
        auto it = std::find(_Counterfoils.begin(), _Counterfoils.end(), stub);
        if (it == _Counterfoils.end())
        {
            return false;
        }

        _Counterfoils.erase(it);
        return true;
    }

  private:
    SingleTopicMessageBus() = default;
    std::vector<ticket::Counterfoil<std::function<void(T)>>> _Counterfoils;
};

} // namespace pubsub
} // namespace idofront

#endif
