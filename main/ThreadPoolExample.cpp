#include <idofront/thread/LambdaRunnable.hpp>
#include <idofront/thread/Pool.hpp>

class DummyRunnable : public idofront::thread::Runnable
{
  public:
    DummyRunnable(int id) : _Id(id)
    {
    }

  protected:
    void PreTask() override
    {
        auto message = "Task " + std::to_string(_Id) + " is launching." + "\n";
        std::cout << message << std::flush;
    }
    void Task() override
    {
        Sleep(std::chrono::milliseconds(1000));
        if (!IsContinue())
        {
            return;
        }
        auto message = "Task " + std::to_string(_Id) + " is running." + "\n";
        std::cout << message << std::flush;
    }
    void PostTask() override
    {
        auto message = "Task " + std::to_string(_Id) + " is finished." + "\n";
        std::cout << message << std::flush;
    }

  private:
    int _Id;
};

int main()
{
    auto pool = std::make_shared<idofront::thread::Pool>(4);

    pool->Submit(std::make_shared<idofront::thread::LambdaRunnable>(
        []() {
            std::this_thread::sleep_for(std::chrono::milliseconds(600));
            auto msg = "LambdaRunnable is running.\n";
            std::cout << msg << std::flush;
        },
        []() {
            auto msg = "LambdaRunnable is launching.\n";
            std::cout << msg << std::flush;
        },
        []() {
            auto msg = "LambdaRunnable is finished.\n";
            std::cout << msg << std::flush;
        }));

    for (int i = 0; i < 3; i++)
    {
        auto runnable = std::make_shared<DummyRunnable>(i);
        runnable->RegisterCallback([i]() {
            auto message = "Callback that is registered for Task " + std::to_string(i) + " is called." + "\n";
            std::cout << message << std::flush;
        });
        pool->Submit(runnable);
    }

    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::cout << "* Try to terminate the pool." << std::endl;
    pool->TryTerminate();

    std::cout << "* Wait for the pool to finish." << std::endl;
    pool->Wait();
}
