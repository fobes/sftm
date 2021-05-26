# sftm
Simple fast task manager

Use Visual Studio 2019 for building.

Starting:
```
CSftmTaskManager& tm = CSftmTaskManager::GetInstance();

const auto nWorkersCount = std::thread::hardware_concurrency();
if (!tm.Start(nWorkersCount, []() {}))
    return;
```
Test task:
```
class CTestTask : public CSftmTask
{
public:
    CTestTask(CSftmChainController* pChainController) :CSftmTask(pChainController) {}
    virtual ~CSyncEndTask() {}

public:
    virtual void Execute(CSftmWorker& worker) noexcept override
    {
        /*
        Your code...
        */
        
        delete this;
    }
};
```
Push sync task:
```
auto pCurrentWorker = CSftmWorker::GetCurrentThreadWorker();

CSftmChainController chainController;
pCurrentWorker->PushTask(new CTestTask(&chainController));
pCurrentWorker->WorkUntil(chainController);
```
Push async task:
```
auto pCurrentWorker = CSftmWorker::GetCurrentThreadWorker();

pCurrentWorker->PushTask(new CTestTask(&chainController));
```
Stopping:
```
CSftmTaskManager::GetInstance().Stop();
```
