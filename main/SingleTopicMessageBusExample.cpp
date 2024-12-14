#include <idofront/pubsub/SingleTopicMessageBus.hpp>
#include <iostream>
#include <memory>

/// @brief Example of using the message bus
int main()
{
    std::cout << "Example for SingleTopicMessageBus" << std::endl;

    std::cout << "step 1: prepare messages and message bus" << std::endl;
    auto messages = std::vector<int>{1, 2, 3};
    auto messageBus = idofront::pubsub::SingleTopicMessageBus<int>::Create();

    auto stubs = std::vector<idofront::ticket::Stub>{};
    auto functions = std::vector<std::function<void(int)>>();
    for (auto i = 0; i < 5; ++i)
    {
        functions.push_back(
            [i](int message) { std::cout << "Subscriber " << i << " received message: " << message << std::endl; });
    }

    std::cout << "step 2: subscribe functions to message bus" << std::endl;
    std::transform(functions.begin(), functions.end(), std::back_inserter(stubs),
                   [messageBus](std::function<void(int)> function) { return messageBus->Subscribe(function); });

    std::cout << "step 3: publish messages" << std::endl;
    for (auto message : messages)
    {
        messageBus->Publish(message);

        // print separator
        std::cout << std::endl;
    }

    std::cout << "step 4: unsuscribe some subscribers" << std::endl;
    auto unscribingStubIndices = std::vector<int>{0, 2, 4};
    for (auto stubIndex : unscribingStubIndices)
    {
        if (messageBus->Unsubscribe(stubs[stubIndex]))
        {
            std::cout << "Subscriber " << stubIndex << " unsuscribed" << std::endl;
        }
    }
    std::cout << std::endl;

    std::cout << "step 5: check if subscribers are expired" << std::endl;
    for (auto i = 0; i < stubs.size(); ++i)
    {
        auto msg = (stubs[i].IsExpired()) ? "expired" : "available";
        std::cout << "Subscriber " << i << " is " << msg << std::endl;
    }
    std::cout << std::endl;

    std::cout << "step 6: publish messages again" << std::endl;
    for (auto message : messages)
    {
        messageBus->Publish(message);

        // print separator
        std::cout << std::endl;
    }

    std::cout << "done" << std::endl;
    return 0;
}
