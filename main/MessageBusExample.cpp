#include <idofront/pubsub/MessageBus.hpp>
#include <iostream>
#include <memory>
#include <numeric>

class Scenario
{
  public:
    Scenario(const std::string &topic) : _Topic(topic)
    {
    }
    virtual ~Scenario() = default;
    const std::string Topic()
    {
        return _Topic;
    }
    const std::shared_ptr<idofront::pubsub::MessageBusInterface<int>> MessageBus()
    {
        return _MessageBus;
    }
    virtual void UnregisterAllSubscriber()
    {
        std::for_each(_Stubs.begin(), _Stubs.end(),
                      [this](idofront::ticket::Stub stub) { MessageBus()->Unsubscribe(stub, Topic()); });
    }
    void RegisterSubscriber(std::function<void(int)> subscriber)
    {
        _Stubs.push_back(MessageBus()->Subscribe(subscriber, Topic()));
    }

  protected:
    const std::vector<idofront::ticket::Stub> Stubs()
    {
        return _Stubs;
    }

  private:
    static std::shared_ptr<idofront::pubsub::MessageBusInterface<int>> _MessageBus;
    const std::string _Topic;
    std::vector<idofront::ticket::Stub> _Stubs;
};

std::shared_ptr<idofront::pubsub::MessageBusInterface<int>> Scenario::_MessageBus =
    idofront::pubsub::MessageBus<int>::Create();

int main()
{
    std::cout << "Example for MessageBus" << std::endl;

    std::cout << "step 1: prepare scenarios" << std::endl;

    auto scenarioA = std::make_shared<Scenario>("topic A");
    scenarioA->RegisterSubscriber(
        [](int message) { std::cout << "Scenario A received message: " << message << std::endl; });

    auto scenarioB = std::make_shared<Scenario>("topic B");
    scenarioB->RegisterSubscriber(
        [](int message) { std::cout << "Scenario B received message: " << message << std::endl; });

    auto scenarios = std::vector<std::shared_ptr<Scenario>>{scenarioA, scenarioB};

    auto messageBus = scenarios[0]->MessageBus();
    auto topics = std::vector<std::string>{""};
    std::transform(scenarios.begin(), scenarios.end(), std::back_inserter(topics),
                   [](std::shared_ptr<Scenario> scenario) { return scenario->Topic(); });

    std::cout << "step 2: publish message to message bus directly" << std::endl;
    auto message = 100;
    for (auto topic : topics)
    {
        std::cout << "Publish message(" << message << ") to topic: " << (topic.empty() ? "default" : topic)
                  << std::endl;
        messageBus->Publish(message++, topic);
    }

    std::cout << "step 3: unregister all subscribers in scenario A" << std::endl;
    scenarioA->UnregisterAllSubscriber();

    std::cout << "step 4: publish message to message bus directly" << std::endl;
    for (auto topic : topics)
    {
        std::cout << "Publish message(" << message << ") to topic: " << (topic.empty() ? "default" : topic)
                  << std::endl;
        messageBus->Publish(message++, topic);
    }

    std::cout << "done" << std::endl;
    return 0;
}
