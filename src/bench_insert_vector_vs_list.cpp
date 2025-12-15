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
    std::vector<int> v(N,1);
    std::list<int> l(N,1);

    auto vit = v.begin() + v.size()/2;
    auto lit = std::next(l.begin(),l.size()/2);

    timespec t1{}, t2{};

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    v.insert(vit, 42);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
    std::cout << "vector insert: " << ns(t1,t2)/1e6 << " ms\n";

    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);
    l.insert(lit, 42);
    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);
    std::cout << "list insert: " << ns(t1,t2)/1e6 << " ms\n";
}