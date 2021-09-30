#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>

using namespace std::literals;

namespace unsafe
{
    void increment_counter(int& counter)
    {
        for(int i = 0; i < 1'000'000; ++i)
            ++counter;
    }
}

namespace explain
{
    template <typename MutexType>
    class lock_guard
    {
        MutexType& mtx_;
    public:
        lock_guard(MutexType& mtx) : mtx_{mtx}
        {
            mtx_.lock();
        }

        lock_guard(const lock_guard&) = delete;
        lock_guard& operator=(const lock_guard&) = delete;

        ~lock_guard()
        {
            mtx_.unlock();
        }
    };
}

void increment_counter(int& counter, std::mutex& mtx_counter)
{
    for(int i = 0; i < 1'000'000; ++i)
    {
        {
            explain::lock_guard<std::mutex> lk(mtx_counter); ////////////// start of critical section
            ++counter;
        } ////////////// end of critical section - destructor of lk calls mtx_counter.unlock()

        //std::cout << "Outside critical section" << std::endl;
    }
}

class Lambda_42376452763452763
{
    int& counter_;
    std::mutex& mtx_counter_;
public:
    Lambda_42376452763452763(int& c, std::mutex& m) : counter_{c}, mtx_counter_{m}
    {}

    void operator()() { increment_counter(counter_, mtx_counter_); }
};

int main()
{
    int counter{};
    std::mutex mtx_counter;

    std::thread thd1{&increment_counter, std::ref(counter), std::ref(mtx_counter)};
    std::thread thd2{[&counter, &mtx_counter](){ increment_counter(counter, mtx_counter); }};

    thd1.join();
    thd2.join();

    std::cout << "Counter: " << counter << std::endl;
}
