#ifndef IDOFRONT__THREAD__POOL_HPP
#define IDOFRONT__THREAD__POOL_HPP

#include <condition_variable>
#include <functional>
#include <future>
#include <idofront/enumerate/ThreadSafeQueue.hpp>
#include <idofront/thread/Future.hpp>
#include <idofront/thread/Runnable.hpp>
#include <idofront/thread/Worker.hpp>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace idofront
{
namespace thread
{
class Pool
{
  public:
    Pool(std::size_t threadCount);
    virtual ~Pool();

    /// @brief Submit a runnable to the thread pool
    /// @param runnablePtr
    /// @return Future object to control the thread. If no thread is available, return nullopt.
    virtual std::shared_ptr<Future> Submit(std::shared_ptr<Runnable> runnablePtr);

    /// @brief Get the number of active threads
    /// @return The number of active threads
    virtual std::size_t ActiveThreadCount() const;

    /// @brief Get the number of threads in the thread pool
    virtual std::size_t ThreadCount() const;

    /// @brief Try to terminate all threads
    virtual void TryTerminate();

    /// @brief Wait for all threads to finish
    virtual void Wait();

  private:
    std::vector<WorkerPtr> _Workers;
    std::vector<std::thread> _Threads;
    std::vector<std::shared_ptr<Future>> _Futures;
    mutable std::mutex _Mutex;
    mutable std::mutex _FuturesMutex;
    std::condition_variable _TerminatedCondition;
    RunnableQueuePtr _RunnableQueuePtr;
};

} // namespace thread
} // namespace idofront

#endif
