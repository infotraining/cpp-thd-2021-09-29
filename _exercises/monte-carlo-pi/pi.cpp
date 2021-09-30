#include <atomic>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <random>

using namespace std;

namespace ver_1_0
{
    void calc_hits(uintmax_t count, uintmax_t& hits)
    {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_real_distribution<double> rnd(-1.0, 1.0);

        for (uintmax_t n = 0; n < count; ++n)
        {
            double x = rnd(gen);
            double y = rnd(gen);
            if (x * x + y * y < 1)
                hits++;
        }
    }
}

namespace ver_2_0
{
    void calc_hits(uintmax_t count, uintmax_t& hits) {
        std::random_device rd;
        std::mt19937_64 gen(rd());
        std::uniform_real_distribution<double> rnd(-1.0, 1.0);

        uintmax_t local_hits{};
        for (uintmax_t n = 0; n < count; ++n)
        {
            double x = rnd(gen);
            double y = rnd(gen);
            if (x * x + y * y < 1)
                local_hits++;
        }

        hits = local_hits;
    }
}


int main()
{
    const uintmax_t N = 120'000'000;

    //////////////////////////////////////////////////////////////////////////////
    // single thread
    {
        cout << "Pi calculation started!" << endl;
        const auto start = chrono::high_resolution_clock::now();

        uintmax_t hits{};
        ver_1_0::calc_hits(N, hits);

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = chrono::high_resolution_clock::now();
        const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }

    const uint32_t cores = std::max(1u,std::thread::hardware_concurrency());

    //////////////////////////////////////////////////////////////////////////////
    // multithreading thread - ver 1
    {
        cout << "\nPi calculation started! Cores no: " << cores << endl;
        const auto start = chrono::high_resolution_clock::now();

        std::vector<std::thread> threads(cores);
        std::vector<uintmax_t> results(cores);

        uintmax_t hits{};

        for (size_t i = 0; i < threads.size(); i++)
        {
            auto count_per_core = N / cores;

            threads[i] = std::thread(&ver_1_0::calc_hits, count_per_core, std::ref(results[i]));
        }

        for (auto& t : threads)
            t.join();

        hits = std::accumulate(results.begin(), results.end(), 0ULL);

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = chrono::high_resolution_clock::now();
        const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }


    /// //////////////////////////////////////////////////////////////////////////////
    // multithreading thread - ver 2
    {
        cout << "\nPi calculation started! Cores no: " << cores << endl;
        const auto start = chrono::high_resolution_clock::now();

        std::vector<std::thread> threads(cores);
        std::vector<uintmax_t> results(cores);

        uintmax_t hits{};

        for (size_t i = 0; i < threads.size(); i++)
        {
            auto count_per_core = N / cores;

            threads[i] = std::thread(&ver_2_0::calc_hits, count_per_core, std::ref(results[i]));
        }

        for (auto& t : threads)
            t.join();

        hits = std::accumulate(results.begin(), results.end(), 0ULL);

        const double pi = static_cast<double>(hits) / N * 4;

        const auto end = chrono::high_resolution_clock::now();
        const auto elapsed_time = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        cout << "Pi = " << pi << endl;
        cout << "Elapsed = " << elapsed_time << "ms" << endl;
    }
}
