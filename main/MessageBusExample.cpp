#include <idofront/pubsub/MessageBus.hpp>
#include <iostream>
#include <memory>
#include <numeric>

void PublishMessages(const std::shared_ptr<idofront::pubsub::MessageBusInterface<int>> &messageBus,
                     const std::vector<int> &messages, const std::string &topic)
{
    for (auto message : messages)
    {
        messageBus->Publish(message, topic);
    }
}

void UnsubscribeStub(const std::shared_ptr<idofront::pubsub::MessageBusInterface<int>> &messageBus,
                     const std::vector<idofront::ticket::Stub> &stubs, const std::string &topic)
{
    for (auto index = std::size_t(0); index < stubs.size(); index++)
    {
        auto stub = stubs[index];
        if (messageBus->Unsubscribe(stub, topic))
        {
            std::cout << "Subscriber " << index << " unsuscribed from topic: " << topic << std::endl;
        }
    }
}

std::vector<idofront::ticket::Stub> FilterUnsubscribingStub(const std::vector<idofront::ticket::Stub> &stubs)
{
    auto unsubscribingStubs = std::vector<idofront::ticket::Stub>{};
    for (auto i = std::size_t(0); i < stubs.size(); i++)
    {
        if (i % 2 == 0)
        {
            unsubscribingStubs.push_back(stubs[i]);
        }
    }
    return unsubscribingStubs;
}

std::vector<idofront::ticket::Stub> FlattenStubs(
    const std::unordered_map<std::string, std::vector<idofront::ticket::Stub>> &stubs)
{
    auto flatStubs = std::vector<idofront::ticket::Stub>{};
    for (auto const &entry : stubs)
    {
        for (auto const &stub : entry.second)
        {
            flatStubs.push_back(stub);
        }
    }
    return flatStubs;
}

void ShowStubStatus(const std::vector<idofront::ticket::Stub> &stubs)
{
    for (auto i = std::size_t(0); i < stubs.size(); ++i)
    {
        auto msg = (stubs[i].IsExpired()) ? "expired" : "available";
        std::cout << "Subscriber " << i << " is " << msg << std::endl;
    }
}

/// @brief Example of using the message bus
int main()
{
    std::cout << "Example for MessageBus" << std::endl;

    std::cout << "step 1: prepare topics, messages and message bus" << std::endl;
    auto topics = std::vector<std::string>{"topic1", "topic2"};
    auto messages = std::vector<int>{1, 2, 3};
    auto messageBus = idofront::pubsub::MessageBus<int>::Create();

    std::cout << "step 2: subscribe functions to message bus" << std::endl;
    auto subscribers = std::vector<std::function<void(int)>>{};
    for (auto i = std::size_t(0); i < 3 * topics.size(); ++i)
    {
        subscribers.push_back(
            [i](int message) { std::cout << "Subscriber " << i << " received message: " << message << std::endl; });
    }

    auto stubs = std::unordered_map<std::string, std::vector<idofront::ticket::Stub>>{};
    for (auto j = std::size_t(0); j < topics.size(); ++j)
    {
        stubs[topics[j]] = std::vector<idofront::ticket::Stub>{};
        for (auto i = std::size_t(0); i < 3; ++i)
        {
            auto index = j * topics.size() + i;
            auto func = subscribers[index];
            stubs[topics[j]].push_back(messageBus->Subscribe(func, topics[j]));
        }
    }

    std::cout << "step 3: publish messages" << std::endl;
    for (auto topic : topics)
    {
        PublishMessages(messageBus, messages, topic);
    }

    std::cout << "step 4: unsuscribe some subscribers" << std::endl;
    auto flattenedStubs = FlattenStubs(stubs);
    auto unsubscribingStubs = FilterUnsubscribingStub(flattenedStubs);
    for (auto topic : topics)
    {
        UnsubscribeStub(messageBus, unsubscribingStubs, topic);
    }

    std::cout << "step 5: check if subscribers are expired" << std::endl;
    ShowStubStatus(flattenedStubs);

    std::cout << "step 6: publish messages again" << std::endl;
    for (auto topic : topics)
    {
        PublishMessages(messageBus, messages, topic);
    }

    std::cout << "done" << std::endl;
    return 0;
}
