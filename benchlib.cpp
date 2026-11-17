// benchlib.cpp
#include "benchlib.h"

extern "C" {
#include "avl.h"
#include "bst.h"
#include "hash.h"
#include "pq.h"
}

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <deque>
#include <numeric>
#include <random>

namespace bench {

static const int BASE_ID = 1001;

// ------------------------------------------------------------------ trees

std::vector<TreeRow> trees(const std::vector<int> &sizes)
{
    std::vector<TreeRow> rows;
    for (int n : sizes) {
        for (int pass = 0; pass < 2; pass++) {
            std::vector<int> keys(n);
            std::iota(keys.begin(), keys.end(), BASE_ID);
            if (pass == 1) {
                std::mt19937 rng(12345);
                std::shuffle(keys.begin(), keys.end(), rng);
            }
            BstTree b; AvlTree a;
            bst_init(&b); avl_init(&a);
            for (int k : keys) { bst_insert(&b, k, k); avl_insert(&a, k, k); }

            TreeRow r;
            r.n = n;
            r.order = pass == 0 ? "sorted" : "random";
            r.bstHeight = bst_height(&b);
            r.avlHeight = avl_height(&a);
            r.bstInsertCmp = (double)b.comparisons / n;
            r.avlInsertCmp = (double)a.comparisons / n;
            r.avlRotations = avl_rotations(&a);
            r.avlBalanced = avl_is_balanced(&a) != 0;

            bst_reset_stats(&b); avl_reset_stats(&a);
            int v;
            for (int k : keys) { bst_find(&b, k, &v); avl_find(&a, k, &v); }
            r.bstSearchCmp = (double)b.comparisons / n;
            r.avlSearchCmp = (double)a.comparisons / n;
            rows.push_back(r);
            bst_free(&b); avl_free(&a);
        }
    }
    return rows;
}

// ----------------------------------------------------------------- search

static volatile long g_sink;   // keeps the optimiser from deleting the lookups

std::vector<SearchRow> search(const std::vector<int> &sizes, int lookups, int runs)
{
    using clock = std::chrono::steady_clock;
    std::vector<SearchRow> rows;
    std::mt19937 rng(2026);
    // sparse 6-digit ids (like hospital record numbers): a harder case for hashing than 1001, 1002, ...
    std::vector<int> pool(900000);
    std::iota(pool.begin(), pool.end(), 100000);
    std::shuffle(pool.begin(), pool.end(), rng);

    for (int n : sizes) {
        if (n > (int)pool.size()) n = (int)pool.size();
        std::vector<int> ids(pool.begin(), pool.begin() + n);   // random order = the "register"
        std::vector<int> sorted(ids);
        std::sort(sorted.begin(), sorted.end());

        AvlTree avl; HashTable chain, open;
        avl_init(&avl);
        hash_init(&chain, HASH_CHAINING, 64);
        hash_init(&open, HASH_OPEN, 64);
        for (int i = 0; i < n; i++) {
            avl_insert(&avl, ids[(size_t)i], i);
            hash_put(&chain, ids[(size_t)i], i);
            hash_put(&open, ids[(size_t)i], i);
        }

        std::vector<int> queries(lookups);       // random existing ids
        std::uniform_int_distribution<int> pick(0, n - 1);
        for (int &q : queries) q = ids[(size_t)pick(rng)];

        SearchRow r;
        r.n = n;

        // ---- comparison counts (one counting pass) ----
        long seqC = 0, binC = 0;
        for (int q : queries) {
            for (int i = 0; i < n; i++) { seqC++; if (ids[(size_t)i] == q) break; }
            int lo = 0, hi = n - 1;
            while (lo <= hi) {
                int mid = (lo + hi) / 2;
                binC++;
                if (sorted[(size_t)mid] == q) break;
                if (sorted[(size_t)mid] < q) lo = mid + 1; else hi = mid - 1;
            }
        }
        avl_reset_stats(&avl); hash_reset_stats(&chain); hash_reset_stats(&open);
        int v;
        for (int q : queries) { avl_find(&avl, q, &v); hash_get(&chain, q, &v); hash_get(&open, q, &v); }
        r.seqCmp = (double)seqC / lookups;
        r.binCmp = (double)binC / lookups;
        r.avlCmp = (double)avl.comparisons / lookups;
        r.chainProbes = (double)chain.probes / lookups;
        r.openProbes = (double)open.probes / lookups;

        // ---- timing (plain loops, averaged over several runs) ----
        auto timeIt = [&](auto &&fn) {
            double total = 0;
            for (int run = 0; run < runs; run++) {
                auto t0 = clock::now();
                long acc = 0;
                for (int q : queries) acc += fn(q);
                auto t1 = clock::now();
                g_sink = acc;
                total += std::chrono::duration<double, std::nano>(t1 - t0).count();
            }
            return total / ((double)runs * lookups);
        };
        r.seqNs = timeIt([&](int q) { for (int i = 0; i < n; i++) if (ids[(size_t)i] == q) return (long)i; return -1L; });
        r.binNs = timeIt([&](int q) {
            int lo = 0, hi = n - 1;
            while (lo <= hi) {
                int mid = (lo + hi) / 2;
                if (sorted[(size_t)mid] == q) return (long)mid;
                if (sorted[(size_t)mid] < q) lo = mid + 1; else hi = mid - 1;
            }
            return -1L;
        });
        r.avlNs   = timeIt([&](int q) { int x = -1; avl_find(&avl, q, &x); return (long)x; });
        r.chainNs = timeIt([&](int q) { int x = -1; hash_get(&chain, q, &x); return (long)x; });
        r.openNs  = timeIt([&](int q) { int x = -1; hash_get(&open, q, &x); return (long)x; });
        rows.push_back(r);

        avl_free(&avl); hash_free(&chain); hash_free(&open);
    }
    return rows;
}

// ----------------------------------------------------------------- triage

TriageResult triage(int ticks, unsigned seed)
{
    // Each tick 0-3 patients arrive (mean 1.5); the doctor sees 2 on 70% of ticks, else 1 (mean 1.7).
    // Severity mix: 1:30% 2:25% 3:20% 4:15% 5:10%.
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> arrivals(0, 3), pct(0, 99);
    auto severityOf = [&]() {
        int p = pct(rng);
        return p < 30 ? 1 : p < 55 ? 2 : p < 75 ? 3 : p < 90 ? 4 : 5;
    };

    struct Pat { int severity; int arrival; };
    std::vector<Pat> all;                         // index = patient id
    std::deque<int> fifo;
    PriorityQueue pq;
    pq_init(&pq, 64);

    long served[6] = {0}, servedF[6] = {0}, sumF[6] = {0}, sumP[6] = {0}, maxF[6] = {0}, maxP[6] = {0};
    // the same arrival stream feeds both queues, and both serve the same number of patients per tick
    for (int t = 0; t < ticks; t++) {
        int a = arrivals(rng);
        for (int i = 0; i < a; i++) {
            int id = (int)all.size();
            all.push_back({severityOf(), t});
            fifo.push_back(id);
            pq_push(&pq, id, all.back().severity, "");
        }
        int cap = pct(rng) < 70 ? 2 : 1;
        for (int i = 0; i < cap; i++) {
            if (!fifo.empty()) {
                int id = fifo.front(); fifo.pop_front();
                long w = t - all[(size_t)id].arrival;
                int s = all[(size_t)id].severity;
                sumF[s] += w; if (w > maxF[s]) maxF[s] = w;
                servedF[s]++;
            }
            TriageEntry e;
            if (pq_pop(&pq, &e) == 0) {
                long w = t - all[(size_t)e.patient_id].arrival;
                int s = e.severity;
                sumP[s] += w; if (w > maxP[s]) maxP[s] = w;
                served[s]++;
            }
        }
    }
    pq_free(&pq);

    TriageResult res;
    res.ticks = ticks;

    long totF = 0, totP = 0, nF = 0, nP = 0;
    for (int s = 1; s <= 5; s++) {
        TriageRow r;
        r.severity = s;
        r.served = served[s];
        r.fifoMean = servedF[s] ? (double)sumF[s] / (double)servedF[s] : 0;
        r.pqMean = served[s] ? (double)sumP[s] / (double)served[s] : 0;
        r.fifoMax = maxF[s];
        r.pqMax = maxP[s];
        res.rows.push_back(r);
        totF += sumF[s]; nF += servedF[s];
        totP += sumP[s]; nP += served[s];
    }
    res.fifoOverall = nF ? (double)totF / (double)nF : 0;
    res.pqOverall = nP ? (double)totP / (double)nP : 0;
    return res;
}

// ------------------------------------------------------------------ text

std::string textReport(const std::vector<TreeRow> &t, const std::vector<SearchRow> &s, const TriageResult &q)
{
    std::string o;
    char b[256];

    o += "== 1. BST vs AVL tree (height and comparisons) ==\n";
    snprintf(b, sizeof b, "%7s %-7s | %9s %9s | %12s %12s | %12s %12s | %9s\n",
             "n", "order", "BST h", "AVL h", "BST ins cmp", "AVL ins cmp", "BST find cmp", "AVL find cmp", "rotations");
    o += b;
    for (const TreeRow &r : t) {
        snprintf(b, sizeof b, "%7d %-7s | %9d %9d | %12.1f %12.1f | %12.1f %12.1f | %9lu\n",
                 r.n, r.order.c_str(), r.bstHeight, r.avlHeight, r.bstInsertCmp, r.avlInsertCmp,
                 r.bstSearchCmp, r.avlSearchCmp, r.avlRotations);
        o += b;
    }

    o += "\n== 2. Search technique vs average lookup (comparisons/probes and ns per lookup) ==\n";
    snprintf(b, sizeof b, "%7s | %9s %9s %9s %9s %9s | %9s %9s %9s %9s %9s\n", "n",
             "seq cmp", "bin cmp", "AVL cmp", "chain pr", "open pr", "seq ns", "bin ns", "AVL ns", "chain ns", "open ns");
    o += b;
    for (const SearchRow &r : s) {
        snprintf(b, sizeof b, "%7d | %9.1f %9.1f %9.1f %9.2f %9.2f | %9.0f %9.0f %9.0f %9.0f %9.0f\n", r.n,
                 r.seqCmp, r.binCmp, r.avlCmp, r.chainProbes, r.openProbes,
                 r.seqNs, r.binNs, r.avlNs, r.chainNs, r.openNs);
        o += b;
    }

    snprintf(b, sizeof b, "\n== 3. Priority queue vs FIFO triage (%d ticks, mean wait in ticks) ==\n", q.ticks);
    o += b;
    snprintf(b, sizeof b, "%-9s %8s | %10s %10s | %9s %9s\n", "severity", "served", "FIFO mean", "PQ mean", "FIFO max", "PQ max");
    o += b;
    for (const TriageRow &r : q.rows) {
        snprintf(b, sizeof b, "%-9d %8ld | %10.1f %10.1f | %9ld %9ld\n", r.severity, r.served, r.fifoMean, r.pqMean, r.fifoMax, r.pqMax);
        o += b;
    }
    snprintf(b, sizeof b, "%-9s %8s | %10.1f %10.1f |\n", "overall", "", q.fifoOverall, q.pqOverall);
    o += b;
    return o;
}

} // namespace bench
