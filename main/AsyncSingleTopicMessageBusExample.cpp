#include <idofront/pubsub/AsyncMessageBus.hpp>
#include <idofront/pubsub/AsyncSingleTopicMessageBus.hpp>
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
    const std::shared_ptr<idofront::pubsub::AsyncMessageBusInterface<int>> MessageBus()
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
    static std::shared_ptr<idofront::pubsub::AsyncMessageBusInterface<int>> _MessageBus;
    const std::string _Topic;
    std::vector<idofront::ticket::Stub> _Stubs;
};

std::shared_ptr<idofront::pubsub::AsyncMessageBusInterface<int>> Scenario::_MessageBus =
    idofront::pubsub::AsyncSingleTopicMessageBus<int>::Create();

void RegisterSubscriber(std::shared_ptr<Scenario> scenario)
{
    auto topic = scenario->Topic();
    auto quick_subscriber = [](int message) {
        std::cout << "Quick subscriber for topic: " << message << " received message: " << message << std::endl;
    };
    scenario->RegisterSubscriber(quick_subscriber);

    auto slow_subscriber = [topic](int message) {
        std::this_thread::sleep_for(std::chrono::milliseconds(3000));
        std::cout << "Slow subscriber for topic: " << topic << " received message: " << message << std::endl;
    };
    scenario->RegisterSubscriber(slow_subscriber);
}
int main()
{
    std::cout << "Example for AsyncMessageBus" << std::endl;

    std::cout << "step 1: prepare scenarios" << std::endl;

    auto scenario = std::make_shared<Scenario>("");
    RegisterSubscriber(scenario);

    auto scenarios = std::vector<std::shared_ptr<Scenario>>{scenario};

    auto messageBus = scenarios[0]->MessageBus();
    auto topics = std::vector<std::string>{""};
    std::transform(scenarios.begin(), scenarios.end(), std::back_inserter(topics),
                   [](std::shared_ptr<Scenario> scenario) { return scenario->Topic(); });

    std::cout << "step 2: publish message to message bus directly" << std::endl;
    auto message = 100;
    {
        auto futures = std::vector<std::future<void>>{};
        for (auto topic : topics)
        {
            std::cout << "Publish message(" << message << ") to topic: " << (topic.empty() ? "default" : topic)
                      << std::endl;
            futures.push_back(messageBus->PublishAsync(message++, topic));
        }
        std::for_each(futures.begin(), futures.end(), [](std::future<void> &future) { future.wait(); });
    }

    std::cout << "step 3: unregister all subscribers in scenario A" << std::endl;
    scenario->UnregisterAllSubscriber();

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
