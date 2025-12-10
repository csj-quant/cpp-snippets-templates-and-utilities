#include <iostream>
#include <string>
#include <ctime>

int main()
{
    std::string s;
    const int LIMIT = 500;

    std::cout << "i\tsize\tcapacity\tdata\n";
    std::cout << "-------------------------------------\n";

    for (int i = 0; i < LIMIT; i++)
    {
        s.push_back('a');
        std::cout << i << "\t" << s.size() << "\t" << s.capacity() << "\t" << static_cast<const void*>(s.data())<< "\n";
    }
    return 0;
}