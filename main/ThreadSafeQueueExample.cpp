#include <idofront/enumerate/IThreadSafeQueue.hpp>
#include <idofront/enumerate/ThreadSafeQueue.hpp>
#include <iostream>

int main()
{
    auto timeoutThreshold = std::chrono::milliseconds(1000);
    auto queue = std::make_shared<idofront::enumerate::ThreadSafeQueue<int>>();

    queue->Enqueue(1);
    auto isEnqueueSuccessfully = queue->TryEnqueue(2, timeoutThreshold);

    auto value1 = queue->Dequeue(timeoutThreshold);
    auto value2 = int(0);
    auto isDequeueSuccessfully = queue->TryDequeue(value2);

    std::cout << "isEnqueueSuccessfully: " << isEnqueueSuccessfully << std::endl;
    std::cout << "value1 has value: " << value1.has_value() << std::endl;
    std::cout << "value1: " << value1.value() << std::endl;
    std::cout << "isDequeueSuccessfully: " << isDequeueSuccessfully << std::endl;
    std::cout << "value2: " << value2 << std::endl;
}
