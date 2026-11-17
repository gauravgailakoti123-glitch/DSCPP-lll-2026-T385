// benchlib.h - benchmarks for Milestones M3/M4 (shared by the console, bench tool and web page)
#ifndef BENCHLIB_H
#define BENCHLIB_H

#include <string>
#include <vector>

namespace bench {

// BST vs AVL: insert n keys in the given order, then look every key up once
struct TreeRow {
    int           n;
    std::string   order;            // "sorted" or "random"
    int           bstHeight, avlHeight;
    double        bstInsertCmp, avlInsertCmp;   // average comparisons per insert
    double        bstSearchCmp, avlSearchCmp;   // average comparisons per successful search
    unsigned long avlRotations;
    bool          avlBalanced;
};

// sequential vs binary vs AVL vs hash lookups of random existing ids
struct SearchRow {
    int    n;
    double seqCmp, binCmp, avlCmp, chainProbes, openProbes;   // average comparisons / probes per lookup
    double seqNs, binNs, avlNs, chainNs, openNs;              // average time per lookup in nanoseconds
};

// priority queue vs FIFO, simulated clinic (one row per severity 1..5)
struct TriageRow {
    int    severity;
    long   served;
    double fifoMean, pqMean;        // mean wait in ticks
    long   fifoMax, pqMax;          // longest wait in ticks
};
struct TriageResult {
    int                    ticks;
    std::vector<TriageRow> rows;
    double fifoOverall, pqOverall;  // mean wait over everyone served
};

std::vector<TreeRow>   trees(const std::vector<int> &sizes);
std::vector<SearchRow> search(const std::vector<int> &sizes, int lookupsPerRun, int runs);
TriageResult           triage(int ticks, unsigned seed);

// plain-text report of all three experiments
std::string textReport(const std::vector<TreeRow> &t, const std::vector<SearchRow> &s, const TriageResult &q);

} // namespace bench

#endif
