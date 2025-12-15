#include <vector>
#include <list>
#include <ctime>
#include <iostream>

inline long long ns(const timespec& a, const timespec& b)
{
    return (b.tv_sec-a.tv_sec)*1e9 + (b.tv_nsec-a.tv_nsec);
}

int main()
{
    const int N = 200000;
    std::vector<int> va(N,1), vb(N,2);
    std::list<int> a(N,1), b(N,2);

    timespec t1{}, t2{};

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    a.splice(a.begin(), b, b.begin());
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
    std::cout << "list splice: " << ns(t1,t2) << " ns\n";

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    va.insert(va.begin(), vb.front()); vb.erase(vb.begin());
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
    std::cout << "vector move: " << ns(t1,t2) << " ns\n";
}