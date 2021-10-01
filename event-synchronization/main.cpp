#include <atomic>
#include <cassert>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <algorithm>

using namespace std::literals;

namespace BusyWaitWithAtomic
{
    class Data
    {
        std::vector<int> data_;
        std::atomic<bool> is_ready_{false};

    public:
        void read()
        {
            std::cout << "Start reading..." << std::endl;
            data_.resize(100);

            std::random_device rnd;
            std::generate(begin(data_), end(data_), [&rnd]
                { return rnd() % 1000; });
            std::this_thread::sleep_for(2s);
            std::cout << "End reading..." << std::endl;

            is_ready_.store(true, std::memory_order_release); //XXXXXXXXXXXX
        }

        void process(int id)
        {
            while (!is_ready_.load(std::memory_order_acquire)) //XXXXXXXXXXXX
            {
            }

            long sum = std::accumulate(begin(data_), end(data_), 0L);

            std::cout << "Id: " << id << "; Sum: " << sum << std::endl;
        }
    };
}

namespace IdleWaitConditionVariable
{
    class Data
    {
        std::vector<int> data_;
        bool is_ready_{false};
        std::mutex mtx_is_ready_;
        std::condition_variable cv_is_ready_;

    public:
        void read()
        {
            std::cout << "Start reading..." << std::endl;
            data_.resize(100);

            std::random_device rnd;
            std::generate(begin(data_), end(data_), [&rnd]
                { return rnd() % 1000; });
            std::this_thread::sleep_for(2s);
            std::cout << "End reading..." << std::endl;

            {
                std::lock_guard<std::mutex> lk{mtx_is_ready_};
                is_ready_ = true;
            } // end of critical section
            cv_is_ready_.notify_all();
        }

        void process(int id)
        {
            {
                std::unique_lock<std::mutex> lk{mtx_is_ready_};

                /*
                while (!is_ready_)
                {
                    cv_is_ready_.wait(lk);
                }
                */

                cv_is_ready_.wait(lk, [this] { return is_ready_; });

            }

            long sum = std::accumulate(begin(data_), end(data_), 0L);

            std::cout << "Id: " << id << "; Sum: " << sum << std::endl;
        }
    };
}

int main()
{
    using namespace IdleWaitConditionVariable;

    Data data;

    std::thread thd_producer {[&data]
        { data.read(); }
    };

    std::thread thd_consumer1 {[&data]
        { data.process(1); }
    };

    std::thread thd_consumer2 {[&data]
        { data.process(2); }
    };

    thd_producer.join();
    thd_consumer1.join();
    thd_consumer2.join();
}
