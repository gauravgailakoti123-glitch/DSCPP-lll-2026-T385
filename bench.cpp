// bench.cpp - command-line benchmark runner:  ./bench [--quick]
#include "benchlib.h"
#include <cstdio>
#include <cstring>

int main(int argc, char **argv)
{
    bool quick = argc > 1 && strcmp(argv[1], "--quick") == 0;
    std::vector<int> treeSizes   = quick ? std::vector<int>{100, 1000, 5000}
                                         : std::vector<int>{100, 500, 1000, 2000, 5000, 10000, 20000};
    std::vector<int> searchSizes = quick ? std::vector<int>{100, 1000, 10000}
                                         : std::vector<int>{100, 1000, 5000, 10000, 50000, 100000};
    int lookups = quick ? 1000 : 2000, runs = quick ? 3 : 5;

    printf("Running benchmarks%s ...\n\n", quick ? " (quick)" : "");
    std::vector<bench::TreeRow>   t = bench::trees(treeSizes);
    std::vector<bench::SearchRow> s = bench::search(searchSizes, lookups, runs);
    bench::TriageResult           q = bench::triage(quick ? 5000 : 20000, 7);
    printf("%s", bench::textReport(t, s, q).c_str());
    return 0;
}
