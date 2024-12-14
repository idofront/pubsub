#ifndef IDOFRONT__THREAD__LAMBDA_RUNNABLE_HPP
#define IDOFRONT__THREAD__LAMBDA_RUNNABLE_HPP

#include <chrono>
#include <functional>
#include <idofront/thread/Runnable.hpp>

namespace idofront
{
namespace thread
{
/// @brief LambdaRunnable class
/// @note This class is a Runnable class that can be created with lambda functions.
/// In this implementation, you cannot use Sleep() or SleepUntil() functions in the lambda functions.
class LambdaRunnable : public Runnable
{
  public:
    LambdaRunnable(std::function<void()> task, std::function<void()> preTask = nullptr,
                   std::function<void()> postTask = nullptr)
        : _PreTask(preTask), _Task(task), _PostTask(postTask)
    {
    }

  protected:
    virtual void PreTask() override
    {
        if (_PreTask)
        {
            _PreTask();
        }
    }
    virtual void Task() override
    {
        if (_Task)
        {
            _Task();
        }
        else
        {
            // Protect the thread from infinite loop
            TryTerminate();
        }
    }
    virtual void PostTask() override
    {
        if (_PostTask)
        {
            _PostTask();
        }
    }

  private:
    std::function<void()> _PreTask;
    std::function<void()> _Task;
    std::function<void()> _PostTask;
};

} // namespace thread
} // namespace idofront

#endif
