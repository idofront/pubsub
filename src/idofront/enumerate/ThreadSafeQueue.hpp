#ifndef IDOFRONT__ENUMERATE__THREAD_SAFE_QUEUE_HPP
#define IDOFRONT__ENUMERATE__THREAD_SAFE_QUEUE_HPP

#include <condition_variable>
#include <idofront/enumerate/IThreadSafeQueue.hpp>
#include <mutex>
#include <queue>

namespace idofront
{
namespace enumerate
{
template <typename T> class ThreadSafeQueue : public IThreadSafeQueue<T>
{
  public:
    virtual void Enqueue(T value) override
    {
        std::lock_guard<std::mutex> lock(_Mutex);
        _Queue.push(std::move(value));
        _ConditionVariable.notify_one();
    }

    virtual bool TryEnqueue(T value, std::chrono::milliseconds timeout) override
    {
        std::unique_lock<std::mutex> lock(_Mutex);
        if (_ConditionVariable.wait_for(lock, timeout, [this] { return !_Queue.empty(); }))
        {
            _Queue.push(std::move(value));
            _ConditionVariable.notify_one();
            return true;
        }
        return false;
    }

    virtual std::optional<T> Dequeue(std::chrono::milliseconds timeout) override
    {
        std::unique_lock<std::mutex> lock(_Mutex);
        if (_ConditionVariable.wait_for(lock, timeout, [this] { return !_Queue.empty(); }))
        {
            T value = std::move(_Queue.front());
            _Queue.pop();
            return value;
        }
        else
        {
            return std::nullopt;
        }
    }

    virtual bool TryDequeue(T &value) override
    {
        std::lock_guard<std::mutex> lock(_Mutex);
        if (_Queue.empty())
        {
            return false;
        }
        value = std::move(_Queue.front());
        _Queue.pop();
        return true;
    }

    virtual bool Empty() const override
    {
        std::lock_guard<std::mutex> lock(_Mutex);
        return _Queue.empty();
    }

  private:
    std::queue<T> _Queue;
    mutable std::mutex _Mutex;
    std::condition_variable _ConditionVariable;
};
} // namespace enumerate
} // namespace idofront

#endif
