#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <random>

using namespace std;

void throwPi(uint32_t ile, uintmax_t& trafione)
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<double> losuj(-1.0, 1.0);

    for (long n = 0; n < ile; ++n)
    {
        double x = losuj(gen);
        double y = losuj(gen);
        if (x * x + y * y < 1)
            trafione++;
    }
}


int main()
{
    const long N = 120'000'000;

    {
        cout << "Pi calculation started!" << endl;
        const auto start = chrono::high_resolution_clock::now();

        uintmax_t hits{};
        throwPi(N, hits);

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = chrono::high_resolution_clock::now();
        const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }

    //////////////////////////////////////////////////////////////////////////////
    // multithreading thread
    {
        uint32_t cores = std::max(1u,std::thread::hardware_concurrency());

        cout << "Pi calculation started!" << endl;
        const auto start = chrono::high_resolution_clock::now();
        std::vector<std::thread> threads(cores);
        std::vector<uintmax_t> wyniki(cores);

        long hits = 0;

        for (uint8_t i = 0; i<threads.size(); i++)
        {
            uint32_t ile = N/cores;

            threads.at(i) = std::thread(&throwPi, ile, std::ref(wyniki.at(i)));
        }

        for (auto& t : threads)
            t.join();

        hits = std::accumulate(wyniki.begin(), wyniki.end(), 0ULL);

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = chrono::high_resolution_clock::now();
        const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }

    //////////////////////////////////////////////////////////////////////////////
}
