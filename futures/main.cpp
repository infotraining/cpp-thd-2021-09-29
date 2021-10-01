#include <cassert>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

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

void save_to_file(const std::string& filename)
{
    std::cout << "Saving to file: " << filename << std::endl;

    std::this_thread::sleep_for(3s);

    std::cout << "File saved: " << filename << std::endl;
}

void using_async()
{
    std::future<int> r1 = std::async(std::launch::async, &calculate_square, 11);
    std::future<int> r2 = std::async(std::launch::async, [] { return calculate_square(13); });
    std::future<int> r3 = std::async(std::launch::deferred, &calculate_square, 21);
    std::future<void> fsave = std::async(std::launch::async, &save_to_file, "data.txt");

    while(fsave.wait_for(400ms) != std::future_status::ready)
    {
        std::cout << "I'm still waiting for saving a file..." << std::endl;
    }

    std::cout << "r1: " << r1.get() << std::endl;

    std::cout << "r2: " << r2.get() << std::endl;

    try
    {
        int result = r3.get();
        std::cout << "r3: " << result  << std::endl;
    }
    catch(const std::runtime_error& e)
    {
        std::cout << e.what() << std::endl;
    }
}

int bug_fixed()
{
    auto f1 = std::async(std::launch::async, &save_to_file, "data1.txt");
    auto f2 = std::async(std::launch::async, &save_to_file, "data2.txt");
    auto f3 = std::async(std::launch::async, &save_to_file, "data3.txt");
    auto f4 = std::async(std::launch::async, &save_to_file, "data4.txt");
}

void consume(std::shared_future<int> value)
{
    std::cout << "Thread#" << std::this_thread::get_id() << " - " << value.get() << std::endl;
}

void produce_consume()
{
    std::future<int> fcalc = std::async(std::launch::async, &calculate_square, 14);

    std::shared_future<int> sf1 = fcalc.share();

    std::thread thd1{ [sf1] { consume(sf1); }};
    std::thread thd2{ [sf1] { consume(sf1); }};

    thd1.join();
    thd2.join();
}


template <typename Callable>
auto launch_async(Callable&& callable)
{
    using ResultT = decltype(callable());

    std::packaged_task<ResultT()> pt(std::forward<Callable>(callable));
    std::future<ResultT> f = pt.get_future();

    std::thread thd{std::move(pt)};
    thd.detach();

    return f;
}

void no_wait_in_desctructor()
{
    auto f = launch_async([]{ save_to_file("data1");});
    launch_async([]{ save_to_file("data2");});
    launch_async([]{ save_to_file("data3");});

    f.wait();
}

void using_packaged_task()
{
    std::packaged_task<int()> pt1([] { return calculate_square(13); });
    std::packaged_task<int(int)> pt2(&calculate_square);

    std::future<int> f1 = pt1.get_future();
    std::future<int> f2 = pt2.get_future();

    std::thread thd1{std::move(pt1)};
    pt2(23);

    std::cout << "f1: " << f1.get() << std::endl;
    std::cout << "f2: " << f2.get() << std::endl;

    thd1.join();
}

class SquareCalculator
{
    std::promise<int> promise_;
public:
    std::future<int> get_future()
    {
        return promise_.get_future();
    }

    void calculate(int x)
    {
        try
        {
            int result = calculate_square(x);
            promise_.set_value(result);
        }
        catch (...)
        {
            promise_.set_exception(std::current_exception());
        }
    }
};

void using_promise()
{
    SquareCalculator calc;

    auto fresult = calc.get_future();

    std::thread thd{[f = std::move(fresult)]() mutable {
        std::cout << "From thread#" << std::this_thread::get_id() << " - " << f.get() << std::endl;
    }};

    std::this_thread::sleep_for(3s);
    calc.calculate(13);

    thd.join();
}

int main()
{
    using_promise();
}

