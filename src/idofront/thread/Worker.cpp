#include <idofront/thread/Worker.hpp>

namespace idofront
{
namespace thread
{
Worker::Worker(RunnableQueuePtr queue) : _QueuePtr(queue)
{
}

Worker::~Worker() = default;

void Worker::Task()
{
    while (IsContinue())
    {
        auto runnablePtrOpt = _QueuePtr->Dequeue(std::chrono::milliseconds(1000));

        if (!runnablePtrOpt.has_value())
        {
            continue;
        }

        auto runnablePtr = runnablePtrOpt.value();

        if (!runnablePtr)
        {
            spdlog::warn("Runnable is nullptr.");
            continue;
        }

        runnablePtr->Run();
    }
}

} // namespace thread
} // namespace idofront
