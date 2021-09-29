#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

using namespace std::literals;

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay)
{
    std::cout << "bw#" << id << " has started..." << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(delay);
    }

    std::cout << "bw#" << id << " is finished..." << std::endl;
}

class BackgroundWork
{
    const int id_;
    const std::string text_;

public:
    BackgroundWork(int id, std::string text)
        : id_{id}
        , text_{std::move(text)}
    {
    }

    void operator()(std::chrono::milliseconds delay) const
    {
        std::cout << "BW#" << id_ << " has started..." << std::endl;

        for (const auto& c : text_)
        {
            std::cout << "BW#" << id_ << ": " << c << std::endl;

            std::this_thread::sleep_for(delay);
        }

        std::cout << "BW#" << id_ << " is finished..." << std::endl;
    }
};

std::thread create_background_work(int id)
{
    const std::string text = "THREAD#"s + std::to_string(id);
    return std::thread(&background_work, id, text, 100ms);
}

class UniqueIntPtr
{
    int* ptr_;
public:
    UniqueIntPtr(int* ptr) : ptr_{ptr}
    {}

    UniqueIntPtr(const UniqueIntPtr&) = delete;

    UniqueIntPtr(UniqueIntPtr&& source) : ptr_{source.ptr_}
    {
        source.ptr_ = nullptr;
    }

    ~UniqueIntPtr()
    {
        delete ptr_;
    }
};

void move_semantics_description()
{
    UniqueIntPtr uptr1(new int(13));

    UniqueIntPtr uptr2 = std::move(uptr1);
}

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    std::thread thd1 = create_background_work(1);

    std::thread thd2 = std::move(thd1);

    thd1 = create_background_work(2);

    std::vector<std::thread> threads;

    threads.push_back(create_background_work(3));
    threads.push_back(create_background_work(4));
    threads.push_back(create_background_work(5));
    threads.push_back(std::move(thd1));
    threads.push_back(std::move(thd2));
    threads.emplace_back(&background_work, 6, "text", 50ms);

    { // popping from vector
        std::thread thd = std::move(threads[1]);
        threads.erase(begin(threads)+1);

        thd.join();
    }

    for(auto& thd : threads)
        if (thd.joinable())
            thd.join();

    std::cout << "Main thread ends..." << std::endl;
}
