/*
You are given a 200 million–character stream consisting only of:
a–z letters
spaces
punctuation

You must compute the length of the longest valid token, where a token is:
a maximal contiguous sequence of ASCII letters [A-Za-z]
ignoring numbers, punctuation, and whitespace
Example:

Input (small example):
"this,is.a-very...longWORD123with interruptions"
Tokens:
this
is
a
very
longWORD
with
interruptions

Longest token length = 12 ("interruptions")
Process 200 million characters in < 4 ms
NO dynamic allocation inside loop
Only one pass allowed (O(N))

This is exactly the kind of problem that appears in:

Market data feed parsing
Tokenizing FIX/ITCH messages
Cleaning alt-data streams (news, transcripts)
NLP preprocessing inside low-latency engines
*/

#include <iostream>
#include <vector>
#include <ctime>
#include <cctype>

inline long long ns_diff(const timespec &a, const timespec &b)
{
    return (b.tv_sec - a.tv_sec) * 1000000000LL + (b.tv_nsec - a.tv_nsec);
}

int main()
{
    const int N = 200000000;
    std::vector<char> s(N);

    // This line is simply creating a huge synthetic string (length = N) with a repeatable pattern so that:
    // the benchmark is deterministic
    // the data is realistic (letters + occasional spaces)
    // the compiler cannot optimize anything away
    // Every 50th character becomes a space ' ' and for all other positions, use a letter a–z
    for (int i = 0; i < N; i++)
        s[i] = (i % 50 == 0) ? ' ' : char('a' + (i % 26));

    timespec t1{}, t2{};
    clock_gettime(CLOCK_MONOTONIC_RAW, &t1);

    int current = 0;
    int best = 0;

    // Single pass: O(N) and branch-light
    for (int i = 0; i < N; i++)
    {
        // ASCII fast check: (c | 32) keeps a/A mapping
        char c = s[i] | 32;

        bool isLetter = (c >= 'a' && c <= 'z');

        // Convert boolean to int without branch
        current = isLetter ? current + 1 : 0;

        // Track best
        best = (current > best) ? current : best;
    }

    clock_gettime(CLOCK_MONOTONIC_RAW, &t2);

    std::cout << "Longest valid token length = " << best << "\n";
    std::cout << "Time = " << ns_diff(t1, t2) / 1e6 << " ms\n";

    return 0;
}

/*
*Longest valid token length = 49
Time = 330.869 ms

For each character you're doing:
a bitwise OR
two comparisons (>= 'a' && <= 'z')
a branchless update
a max update
Scalar throughput on a typical CPU is:
500M – 800M operations per second
So:
200M ops / 600M ops/sec ≈ 0.33 seconds
Which matches measured:
330 ms
 */
