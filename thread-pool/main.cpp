#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <future>
#include <random>
#include "thread_safe_queue.hpp"

using namespace std::literals;

using Task = std::function<void()>;

namespace ver_1_0
{
    static Task end_of_work;

    class ThreadPool
    {
        std::vector<std::thread> threads_;
        ThreadSafeQueue<Task> q_tasks_;

        void run()
        {
            while(true)
            {
                Task task;
                q_tasks_.pop(task);

                if (finish_work(task))
                    return;

                task();
            }
        }

        bool finish_work(Task& task)
        {
            return task == nullptr; // check if task is a poisoning pill
        }
    public:
        ThreadPool(size_t size) : threads_(size)
        {
            for(size_t i = 0; i < size; ++i)
                threads_[i] = std::thread{ [this] { run(); } };
        }

        ~ThreadPool()
        {
            // sending to threads poisoning pills
            for(size_t i = 0; i < threads_.size(); ++i)
                q_tasks_.push(end_of_work);

            for(auto& thd : threads_)
                if (thd.joinable())
                    thd.join();
        }

        void submit(Task task)
        {
            q_tasks_.push(task);
        }
    };
}

namespace ver_1_1
{
    class ThreadPool
    {
        std::vector<std::thread> threads_;
        ThreadSafeQueue<Task> q_tasks_;
        std::atomic<bool> is_done_{false};

        void run()
        {
            while(true)
            {
                Task task;
                q_tasks_.pop(task);

                task();

                if (is_done_)
                    return;
            }
        }

    public:
        ThreadPool(size_t size) : threads_(size)
        {
            for(size_t i = 0; i < size; ++i)
                threads_[i] = std::thread{ [this] { run(); } };
        }

        template <typename Callable>
        auto submit(Callable&& task) //-> decltype(task())
        {
            using ResultT = decltype(task());

            auto pt = std::make_shared<std::packaged_task<ResultT()>>(task);
            std::future<ResultT> fresult = pt->get_future();

            q_tasks_.push([pt] {(*pt)(); });

            return fresult;
        }

        ~ThreadPool()
        {
            for(size_t i = 0; i < threads_.size(); ++i)
                submit([this] { is_done_ = true; });

            for(auto& thd : threads_)
                if (thd.joinable())
                    thd.join();
        }
    };
}

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay)
{
    std::cout << "bw#" << id << " has started in a thread#" << std::this_thread::get_id() << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(delay);
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

int calculate_square(int x)
{
    std::cout << "Starting calculation for " << x << " in " << std::this_thread::get_id() << std::endl;

    std::random_device rd;
    std::uniform_int_distribution<> distr(100, 5000);

    std::this_thread::sleep_for(std::chrono::milliseconds(distr(rd)));

    if (x % 3 == 0)
        throw std::runtime_error("Error#3");

    return x * x;
}

int main()
{
    using namespace ver_1_1;

    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    ThreadPool thread_pool(4);

    std::vector<std::future<int>> fresults;

    for(int i = 1; i < 20; ++i)
        fresults.push_back(thread_pool.submit([i] { return calculate_square(i); }));


    for(auto& f : fresults)
    {
        try
        {
            int r = f.get();
            std::cout << "result: " << r << std::endl;
        }
        catch(const std::exception& e)
        {
            std::cout << e.what() << std::endl;
        }
    }

    std::cout << "Main thread ends..." << std::endl;
}
