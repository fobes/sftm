# sftm
Simple fast task manager. 
System for management your concurrent tasks. Object-oriented programming. Thread pool based. Custom memory allocators. No third party dependencies.

Use Visual Studio 2019 for building.

Starting:
```c++
CSftmTaskManager& tm = CSftmTaskManager::GetInstance();

const auto nWorkersCount = std::thread::hardware_concurrency();
if (!tm.Start(nWorkersCount, []() {}))
    return;
```
Test task:
```c++
class CTestTask : public CSftmTask
{
public:
    CTestTask(CSftmChainController* pChainController) :CSftmTask(pChainController) {}
    virtual ~CTestTask() {}

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
```c++
auto pCurrentWorker = CSftmWorker::GetCurrentThreadWorker();

CSftmChainController chainController;
pCurrentWorker->PushTask(new CTestTask(&chainController));
pCurrentWorker->WorkUntil(chainController);
```
Push async task:
```c++
auto pCurrentWorker = CSftmWorker::GetCurrentThreadWorker();

pCurrentWorker->PushTask(new CTestTask(nullptr));
```
Stopping:
```c++
CSftmTaskManager::GetInstance().Stop();
```
