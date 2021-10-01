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

int main()
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
