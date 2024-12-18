#include <idofront/pubsub/SingleTopicMessageBus.hpp>
#include <idofront/thread/Pool.hpp>
#include <thread>
#include <vector>

class DataObject;
using MessageBusInterface = idofront::pubsub::MessageBusInterface<DataObject>;
using MessageBusInterfacePtr = std::shared_ptr<MessageBusInterface>;

class DataObject
{
  public:
    DataObject() : _Data()
    {
    }

  private:
    std::vector<uint8_t> _Data;
};

class Subject : public idofront::thread::Runnable
{
  public:
    Subject(MessageBusInterfacePtr busPtr) : _BusPtr(busPtr)
    {
    }

    void Task() override
    {
        auto dataObject = ReadDataObject();
        std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        if (!dataObject.has_value())
        {
            return;
        }
        NotifyDataObject(dataObject.value());
    }

    bool IsStandby() const
    {
        return !_DataObject.has_value();
    }

    bool SetDataObject(DataObject dataObject)
    {
        std::unique_lock<std::mutex> lock(_Mutex);
        if (!IsStandby())
        {
            return false;
        }
        _DataObject = dataObject;
        _Condition.notify_all();

        return true;
    }

    void PostTask() override
    {
        // Do nothing
    }

  private:
    std::condition_variable _Condition;
    std::mutex _Mutex;

    std::optional<DataObject> _DataObject;
    MessageBusInterfacePtr _BusPtr;

    std::optional<DataObject> ReadDataObject()
    {
        std::unique_lock<std::mutex> lock(_Mutex);
        auto isConditionSatisfied = _Condition.wait_for(lock, std::chrono::seconds(1), [this] { return !IsStandby(); });
        if (!isConditionSatisfied)
        {
            return std::nullopt;
        }

        if (IsStandby())
        {
            std::cerr << "Error: No data object is set.\n" << std::flush;
            return std::nullopt;
        }
        auto dataObject = _DataObject.value();
        _DataObject.reset();

        return dataObject;
    }

    void NotifyDataObject(DataObject &dataObject)
    {
        _BusPtr->Publish(dataObject);
    }
};

class Dealer
{
  public:
    Dealer(MessageBusInterfacePtr busPtr, std::size_t threadNumber = std::thread::hardware_concurrency())
        : _Pool(threadNumber), _Counter(0), _StartTime(std::chrono::system_clock::now())
    {
        for (std::size_t i = 0; i < threadNumber; i++)
        {
            auto subjectPtr = std::make_shared<Subject>(busPtr);
            _SubjectPtrs.push_back(subjectPtr);
            _Pool.Submit(subjectPtr);
        }
    }
    ~Dealer()
    {
        auto endTime = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - _StartTime);
        auto throughput = static_cast<double>(_Counter) / duration.count() * 1000.0;

        std::cout << "Report: " << _Counter << " data objects are processed.\n" << std::flush;
        std::cout << "Duration(sec): " << duration.count() / 1000.0 << "\n" << std::flush;
        std::cout << "Throughput(obj/sec): " << throughput << "\n" << std::flush;
        _Pool.TryTerminate();
        _Pool.Wait();
    }
    void Subscribe(DataObject dataObject)
    {
        while (true)
        {
            for (auto subjectPtr : _SubjectPtrs)
            {
                if (subjectPtr->IsStandby())
                {
                    subjectPtr->SetDataObject(dataObject);
                    _Counter++;
                    return;
                }
            }
            std::this_thread::sleep_for(std::chrono::nanoseconds(1));
        }
    }

  private:
    idofront::thread::Pool _Pool;
    std::vector<std::shared_ptr<Subject>> _SubjectPtrs;
    std::size_t _Counter;
    std::chrono::system_clock::time_point _StartTime;
};

using DealerPtr = std::shared_ptr<Dealer>;

class Manager
{
  public:
    Manager()
        : _PublishCounter(0), _SubscribeCounter(0),
          _FirstBusPtr(idofront::pubsub::SingleTopicMessageBus<DataObject>::Create()),
          _SecondBusPtr(idofront::pubsub::SingleTopicMessageBus<DataObject>::Create()),
          _ThirdBusPtr(idofront::pubsub::SingleTopicMessageBus<DataObject>::Create()),
          _FirstDealerPtr(std::make_shared<Dealer>(_SecondBusPtr, 100)),
          _SecondDealerPtr(std::make_shared<Dealer>(_ThirdBusPtr, 100)),
          _FirstBusStub(_FirstBusPtr->Subscribe([this](DataObject obj) { _FirstDealerPtr->Subscribe(obj); })),
          _SecondBusStub(_SecondBusPtr->Subscribe([this](DataObject obj) { _SecondDealerPtr->Subscribe(obj); })),
          _ThirdBusStub(_ThirdBusPtr->Subscribe([this](DataObject) { _SubscribeCounter++; }))
    {
    }

    virtual ~Manager()
    {
        _FirstBusPtr->Unsubscribe(_FirstBusStub);
        _SecondBusPtr->Unsubscribe(_SecondBusStub);
        _ThirdBusPtr->Unsubscribe(_ThirdBusStub);

        _FirstDealerPtr.reset();
        _SecondDealerPtr.reset();

        _FirstBusPtr.reset();
        _SecondBusPtr.reset();
        _ThirdBusPtr.reset();

        std::cout << std::string("Publish Coutner: " + std::to_string(_PublishCounter) + "\n") << std::flush;
        std::cout << std::string("Subscribe Coutner: " + std::to_string(_SubscribeCounter) + "\n") << std::flush;
    }

    void Publish(DataObject dataObject)
    {
        _FirstBusPtr->Publish(dataObject);
        _PublishCounter++;
    }

  private:
    std::size_t _PublishCounter;
    std::size_t _SubscribeCounter;
    MessageBusInterfacePtr _FirstBusPtr;
    MessageBusInterfacePtr _SecondBusPtr;
    MessageBusInterfacePtr _ThirdBusPtr;
    DealerPtr _FirstDealerPtr;
    DealerPtr _SecondDealerPtr;
    idofront::ticket::Stub _FirstBusStub;
    idofront::ticket::Stub _SecondBusStub;
    idofront::ticket::Stub _ThirdBusStub;
};

int main()
{
    auto manager = Manager();

    auto now = std::chrono::system_clock::now();
    auto reserveStopTime = now + std::chrono::seconds(10);
    while (std::chrono::system_clock::now() < reserveStopTime)
    {
        manager.Publish(DataObject());
    }

    return 0;
}
