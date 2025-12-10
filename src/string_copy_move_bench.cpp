#include <iostream>
#include <string>
#include <vector>
#include <ctime>

long long ns_diff(const timespec& a, const timespec& b)
{
    return (b.tv_sec - a.tv_sec) * 1000000000LL + (b.tv_nsec - a.tv_nsec);
}

int main()
{
    std::string base(1000000,'x'); // 1M chars
    const int N=1000;
    std::vector<std::string> copies(N);
    timespec t1,t2;

    // copy benchmark
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    for (int i=0; i < N; i++)
    {
        copies[i] = base;
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
    long long copy_ns = ns_diff(t1,t2);

    // move benchmark
    std::vector<std::string> moves(N);
    std::vector<std::string> temp(N, base);

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    for (int i=0; i < N; i++)
    {
        moves[i] = std::move(temp[i]);
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
    long long move_ns = ns_diff(t1,t2);

    std::cout << "Copy 1000 strings (each of 1 M chars): " << copy_ns / 1e6 << " ms \n";
    std::cout << "Move 1000 strings (each of 1 M chars): " << move_ns / 1e6 << " ms \n";

    return 0;
}