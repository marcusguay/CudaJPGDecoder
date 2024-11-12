#pragma once

#include <thread>
#include <mutex>
#include <queue>
#include <iostream>
#include <atomic>
#include <functional>
#include <future>
#include <condition_variable>


class ThreadPool
{

private:
    std::atomic_bool stop = false;
    std::queue<std::packaged_task<void()>> tasks;
    std::vector<std::thread> threads;
    std::mutex mutex;
    std::condition_variable cv;
    std::condition_variable isDone;
    int numThreads;
    std::atomic<int> operations = 0;

public:
    ThreadPool()
    {
        numThreads = std::thread::hardware_concurrency();

        auto fn = [this]()
        {
            while (!stop)
            {

                std::packaged_task<void()> task;
                {
                    std::unique_lock<std::mutex> lock(mutex);

                    cv.wait(lock, [this]
                            { return (stop || !tasks.empty()); });

                    if (stop)
                    {
                        return;
                    }

                    task = std::move(tasks.front());
                    tasks.pop();
                }

                task();

                if (--operations == 0)
                {
                    isDone.notify_one();
                }
            }
        };

        for (int i = 0; i < numThreads; i++)
        {
            threads.emplace_back(fn);
        }
    }

    void addTask(std::function<void()> fn)
    {
        std::unique_lock<std::mutex> lock(mutex);
        operations++;
        tasks.push(std::packaged_task<void()>(fn));
        cv.notify_one();
    }

    void waitUntilCompleted()
    {
        if(operations > 0){
        std::unique_lock<std::mutex> lock(mutex);
        isDone.wait(lock);
        }
    }


    ~ThreadPool()
    {

        std::cout << "Thread finished executing with " << tasks.size() << " tasks remaining \n";
        {
            std::unique_lock<std::mutex> lock(mutex);
            stop = true;
        }

        cv.notify_all();

        for (auto &thread : threads)
        {
            thread.join();
        }
    }
};