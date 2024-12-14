#ifndef IDOFRONT__PUBSUB__MESSAGE_BUS_HPP
#define IDOFRONT__PUBSUB__MESSAGE_BUS_HPP

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

template <typename T> class MessageBus : public MessageBusInterface<T>
{
  public:
    static std::shared_ptr<MessageBusInterface<T>> Create()
    {
        return std::shared_ptr<MessageBus<T>>(new MessageBus<T>());
    }

    virtual ~MessageBus() override
    {
    }

    virtual void Publish(T message) override
    {
        std::for_each(_Counterfoils.begin(), _Counterfoils.end(),
                      [message](ticket::Counterfoil<std::function<void(T)>> counterfoil) {
                          auto counterfoilPtr = counterfoil.Ptr();
                          auto counterfoilFunc = std::dynamic_pointer_cast<std::function<void(T)>>(counterfoilPtr);
                          (*counterfoilFunc)(message);
                      });
    }

    virtual ticket::Stub Subscribe(std::function<void(T)> subscriber) override
    {
        auto subscriberPtr = std::make_shared<std::function<void(T)>>(subscriber);
        auto ticket = ticket::Receptor::Recept(subscriberPtr);

        // separate the stub and counterfoil
        auto stub = std::get<0>(ticket);
        auto counterfoil = std::get<1>(ticket);

        _Counterfoils.push_back(counterfoil);
        return stub;
    }
    virtual bool Unsubscribe(ticket::Stub stub) override
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
    MessageBus() = default;
    std::vector<ticket::Counterfoil<std::function<void(T)>>> _Counterfoils;
};

} // namespace pubsub
} // namespace idofront

#endif
