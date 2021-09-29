#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

template <typename T>
struct ThreadResult
{
    T value;
    std::exception_ptr eptr;
};

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay, ThreadResult<char>& result)
{
    std::cout << "bw#" << id << " has started..." << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(delay);
    }

    try
    {
        result.value = text.at(3);
    }
    catch(...)
    {
        result.eptr = std::current_exception();
        return;
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::vector<ThreadResult<char>> results(2);
    std::vector<std::thread> threads(2);

    threads[0] = std::thread{&background_work, 1, "text", 50ms, std::ref(results[0])};
    threads[1] = std::thread{&background_work, 2, "OK", 50ms, std::ref(results[1])};

    for(auto& thd : threads)
      thd.join();

    for(auto& r : results)
    {
        if (!r.eptr)
        {
            std::cout << "Result: " << r.value << std::endl;
        }
        else
        {
            try
            {
                std::rethrow_exception(r.eptr);
            }
            catch (const std::out_of_range& e)
            {
                std::cout << "Main has caught an exception " << e.what() << std::endl;
            }
        }
    }

    std::cout << "Main thread ends..." << std::endl;
}
