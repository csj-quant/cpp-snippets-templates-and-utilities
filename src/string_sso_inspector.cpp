#include <iostream>
#include <string>
#include <cstring>
#include <ctime>

int main()
{
    for (size_t len=0; len<=50; len++)
    {
        std::string s(len, 'a');
        const char* addr = s.data();
        size_t cap = s.capacity();
        size_t sz = s.size();

        std::cout << "len=" << len << " size=" << sz << " data=" << static_cast<const void*>(addr) << std::endl;
    }
    return 0;
}