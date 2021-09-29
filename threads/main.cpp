#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include "joining_thread.hpp"

using namespace std::literals;

void background_work(size_t id, const std::string& text, std::chrono::milliseconds delay)
{
    std::cout << "bw#" << id << " -  "<< std::this_thread::get_id() << " has started..." << std::endl;

    for (const auto& c : text)
    {
        std::cout << "bw#" << id << ": " << c << std::endl;

        std::this_thread::sleep_for(delay);
    }

    std::cout << "bw#" << id << " -  "<< std::this_thread::get_id() << " is finished..." << std::endl;
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

int main()
{
    std::cout << "Main thread starts..." << std::endl;
    const std::string text = "Hello Threads";

    {
        std::thread thd_empty;
        std::cout << "thd_empty id: " << thd_empty.get_id() << std::endl;

        ext::joining_thread thd1(&background_work, 1, text, 500ms);
        std::thread thd2(&background_work, 2, "Multithreading", 750ms);
        std::thread thd3(BackgroundWork(3, "Functor"), 250ms);

        BackgroundWork bw(4, "My object");
        std::thread thd4(bw, 1s); // bw is copied to a thread
        ext::joining_thread thd5(std::cref(bw), 100ms); // bw is passed be reference

        std::thread thd6([]{ background_work(5, "lambda", 150ms); });

        thd2.join(); // blocking operation - waiting for thd2 to finish
        thd3.join();
        thd4.join();
        thd6.join();
    }

    /////////////////////////////////////

    const std::vector<int> source = {1, 4, 5, 6, 7, 23, 645, 665, 42};

    std::vector<int> target;
    std::vector<int> backup;

    std::cout << "******************** START" << std::endl;

    {
        ext::joining_thread thd_copy([&] { std::copy(begin(source), end(source), std::back_inserter(target));
        });
        ext::joining_thread thd_backup([&] { std::copy(begin(source), end(source), std::back_inserter(backup));
            std::this_thread::sleep_for(2s);
        });

        thd_copy.join();
        std::cout << "******************** After joining copy" << std::endl;

        std::cout << "target: ";
        for(const auto& item : target)
            std::cout << item << " ";
        std::cout << std::endl;
    } // thd_backup implicit join in destructor
    std::cout << "******************** After joining backup" << std::endl;

    std::cout << "backup: ";
    for(const auto& item : backup)
        std::cout << item << " ";
    std::cout << std::endl;

    std::cout << "Main thread ends..." << std::endl;
}
