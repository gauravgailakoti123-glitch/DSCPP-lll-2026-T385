/* pq.c - binary max-heap priority queue. Push/pop are O(log n). */
#include "pq.h"
#include <stdlib.h>
#include <string.h>

/* returns 1 if a must be served before b */
static int higher(const TriageEntry *a, const TriageEntry *b)
{
    if (a->severity != b->severity)
        return a->severity > b->severity;
    return a->seq < b->seq;
}

static void swap_entries(TriageEntry *a, TriageEntry *b)
{
    TriageEntry t = *a;
    *a = *b;
    *b = t;
}

static void sift_up(PriorityQueue *pq, int i)
{
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (!higher(&pq->data[i], &pq->data[parent]))
            break;
        swap_entries(&pq->data[i], &pq->data[parent]);
        i = parent;
    }
}

static void sift_down(PriorityQueue *pq, int i)
{
    for (;;) {
        int l = 2 * i + 1, r = 2 * i + 2, best = i;
        if (l < pq->size && higher(&pq->data[l], &pq->data[best])) best = l;
        if (r < pq->size && higher(&pq->data[r], &pq->data[best])) best = r;
        if (best == i)
            break;
        swap_entries(&pq->data[i], &pq->data[best]);
        i = best;
    }
}

int pq_init(PriorityQueue *pq, int initial_capacity)
{
    if (initial_capacity < 1)
        initial_capacity = 16;
    pq->data = (TriageEntry *)malloc((size_t)initial_capacity * sizeof(TriageEntry));
    if (!pq->data)
        return -1;
    pq->size = 0;
    pq->capacity = initial_capacity;
    pq->next_seq = 0;
    return 0;
}

void pq_free(PriorityQueue *pq)
{
    free(pq->data);
    pq->data = NULL;
    pq->size = pq->capacity = 0;
}

int pq_push(PriorityQueue *pq, int patient_id, int severity, const char *name)
{
    if (pq->size == pq->capacity) {
        int new_cap = pq->capacity * 2;
        TriageEntry *p = (TriageEntry *)realloc(pq->data, (size_t)new_cap * sizeof(TriageEntry));
        if (!p)
            return -1;
        pq->data = p;
        pq->capacity = new_cap;
    }
    TriageEntry *e = &pq->data[pq->size];
    e->patient_id = patient_id;
    e->severity = severity;
    e->seq = pq->next_seq++;
    strncpy(e->name, name ? name : "", PQ_NAME_LEN - 1);
    e->name[PQ_NAME_LEN - 1] = '\0';
    pq->size++;
    sift_up(pq, pq->size - 1);
    return 0;
}

int pq_pop(PriorityQueue *pq, TriageEntry *out)
{
    if (pq->size == 0)
        return -1;
    if (out)
        *out = pq->data[0];
    pq->size--;
    if (pq->size > 0) {
        pq->data[0] = pq->data[pq->size];
        sift_down(pq, 0);
    }
    return 0;
}

int pq_peek(const PriorityQueue *pq, TriageEntry *out)
{
    if (pq->size == 0)
        return -1;
    if (out)
        *out = pq->data[0];
    return 0;
}

int pq_size(const PriorityQueue *pq)     { return pq->size; }
int pq_is_empty(const PriorityQueue *pq) { return pq->size == 0; }
