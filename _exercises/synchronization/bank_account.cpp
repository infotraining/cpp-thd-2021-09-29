#include <iostream>
#include <thread>
#include <mutex>

struct SyncedCout
{
    std::ostream& out;
    std::mutex mtx_out;
};

template <typename T>
SyncedCout& operator<<(SyncedCout& sc, const T& value)
{
    std::lock_guard<std::mutex> lk{sc.mtx_out};
    sc.out << value;

    return sc;
}

SyncedCout synced_cout{std::cout};

class BankAccount
{
    const int id_;
    double balance_;
    mutable std::mutex mtx_;

public:
    BankAccount(int id, double balance)
        : id_(id)
        , balance_(balance)
    {
    }

    void print() const
    {        
        synced_cout << "Bank Account #" << id_ << "; Balance = " << balance() << "\n";
    }

    void transfer(BankAccount& to, double amount)
    {
        //balance_ -= amount;
        withdraw(amount);

        //to.balance_ += amount;
        to.deposit(amount);
    }

    void withdraw(double amount)
    {
        std::lock_guard<std::mutex> lk{mtx_};
        balance_ -= amount;
    }

    void deposit(double amount)
    {
        std::lock_guard<std::mutex> lk{mtx_};
        balance_ += amount;
    }

    int id() const
    {
        return id_;
    }

    double balance() const
    {
        std::lock_guard<std::mutex> lk{mtx_};
        return balance_;
    }
};

void make_withdraws(BankAccount& ba, int no_of_operations)
{
    for (int i = 0; i < no_of_operations; ++i)
        ba.withdraw(1.0);
}

void make_deposits(BankAccount& ba, int no_of_operations)
{
    for (int i = 0; i < no_of_operations; ++i)
        ba.deposit(1.0);
}

void make_transfer(BankAccount& from, BankAccount& to, int no_of_operations, double amount)
{
    for(int i = 0; i < no_of_operations; ++i)
        from.transfer(to, amount);
}

int main()
{
    const int NO_OF_ITERS = 10'000'000;

    BankAccount ba1(1, 10'000);
    BankAccount ba2(2, 10'000);

    std::cout << "Before threads are started: ";
    ba1.print();
    ba2.print();

    std::thread thd1(&make_withdraws, std::ref(ba1), NO_OF_ITERS);
    std::thread thd2(&make_deposits, std::ref(ba1), NO_OF_ITERS);
    std::thread thd3(&make_transfer, std::ref(ba1), std::ref(ba2), NO_OF_ITERS, 1.0);
    std::thread thd4(&make_transfer, std::ref(ba2), std::ref(ba1), NO_OF_ITERS, 1.0);

    thd1.join();
    thd2.join();
    thd3.join();
    thd4.join();

    std::cout << "After all threads are done: ";
    ba1.print();
    ba2.print();
}
