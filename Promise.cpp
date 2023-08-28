#include <iostream>
#include <thread>
#include <atomic>

template <class F>
class Task
{
private:
    F* function;

public:
    Task(F* func) : function(func) {}

    template<class ...args>
    void Call(args... arguments)
    {
        function(arguments...);
    };
};

class Promise
{
private:
    std::thread th;
    std::atomic_bool isPromiseFinished;
    bool* isObjectDestroyed = nullptr;

public:
    template <class F, class ...args>
    Promise(F* function, args... arguments)
    {
        isPromiseFinished.store(false);

        th = std::thread([&]()
        {
            function(arguments...);
            isPromiseFinished.store(true);
        });
    }

    template <class L>
    Promise(L function)
    {
        isPromiseFinished.store(false);

        th = std::thread([&]()
        {
            isObjectDestroyed = new bool;
            *isObjectDestroyed = false;
            bool* localIsObjectDestroyed = isObjectDestroyed;
            function();
            if (!*localIsObjectDestroyed) isPromiseFinished.store(true);
            delete localIsObjectDestroyed;
        });
    }

    void Detach()
    {
        th.detach();
    }

    void Join()
    {
        if (th.joinable())
            th.join();
    }

    bool GetIsPromiseFinished()
    {
        return isPromiseFinished.load();
    }

    ~Promise()
    {
        if (isObjectDestroyed != nullptr)
            *isObjectDestroyed = true;
    }
};

template <class T>
class Future
{
private:
    T Data;
    std::atomic_bool IsDataReady;
public:
    Future() 
    { 
        IsDataReady.store(false); 
    }

    void SetData(T data) 
    { 
        Data = data;
        IsDataReady.store(true);
    }

    T GetData() 
    { 
        return Data; 
    }

    bool GetIsDataReady() 
    {
        return IsDataReady.load(); 
    }
};

#include <unistd.h>

int main()
{
    Promise p1([]()
    {
        std::cout << "Start promise" << std::endl;
        sleep(5);
        Promise p([]()
        {
            std::cout << "Start promise in promise" << std::endl;
            sleep(3);
            std::cout << "End promise in promise" << std::endl;
        });
        p.Detach();

        std::cout << "End promise" << std::endl;
    });
    p1.Detach();

    for (int i = 0; i < 10; ++i)
    {
        std::cout << "main thread iteration: " << i << std::endl;
        // if (p.GetIsPromiseFinished())
        //     std::cout << "promise finished" << std::endl;
        // else
        //     std::cout << "promise not finished" << std::endl;

        sleep(1);
    }

    p1.Join();
    
    // std::cout << std::endl;
    // std::cout << std::endl;

    // std::cout << "Start waiting data from future" << std::endl;

    // Future<int> dataToWait;
    // Promise p2([&dataToWait]()
    // {
    //     std::cout << "Emulate long calculations" << std::endl;
    //     sleep(5);
    //     dataToWait.SetData(1);
    //     std::cout << dataToWait.GetIsDataReady() << std::endl;
    //     std::cout << "Finish long calculations" << std::endl;
    // });

    // while (!dataToWait.GetIsDataReady());
    // std::cout << "Finally await data: " << dataToWait.GetData() << std::endl;

    // p2.Join();
}