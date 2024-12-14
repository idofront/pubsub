#ifndef IDOFRONT__THREAD__WORKER_HPP
#define IDOFRONT__THREAD__WORKER_HPP

#include <idofront/enumerate/IThreadSafeQueue.hpp>
#include <idofront/thread/Runnable.hpp>
#include <spdlog/spdlog.h>

namespace idofront
{
namespace thread
{
/// @brief Worker class
class Worker : public Runnable
{
  public:
    /// @brief Constructor
    Worker(RunnableQueuePtr queue);

    /// @brief Destructor
    virtual ~Worker();

    void Task();

  private:
    RunnableQueuePtr _QueuePtr;
};
using WorkerPtr = std::shared_ptr<Worker>;
} // namespace thread
} // namespace idofront

#endif
