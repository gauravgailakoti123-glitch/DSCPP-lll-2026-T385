/* pq.h - Priority queue (binary max-heap) for emergency triage.
 * Pure C. Highest severity is served first; equal severity -> earliest arrival. */
#ifndef PQ_H
#define PQ_H

#ifdef __cplusplus
extern "C" {
#endif

#define PQ_NAME_LEN 64

typedef struct {
    int           patient_id;
    int           severity;        /* 1 (minor) .. 5 (critical) */
    unsigned long seq;             /* arrival order, used to break ties */
    char          name[PQ_NAME_LEN];
} TriageEntry;

typedef struct {
    TriageEntry  *data;
    int           size;
    int           capacity;
    unsigned long next_seq;
} PriorityQueue;

int  pq_init(PriorityQueue *pq, int initial_capacity);   /* 0 on success */
void pq_free(PriorityQueue *pq);
int  pq_push(PriorityQueue *pq, int patient_id, int severity, const char *name); /* 0 ok */
int  pq_pop(PriorityQueue *pq, TriageEntry *out);        /* 0 ok, -1 if empty */
int  pq_peek(const PriorityQueue *pq, TriageEntry *out); /* 0 ok, -1 if empty */
int  pq_size(const PriorityQueue *pq);
int  pq_is_empty(const PriorityQueue *pq);

#ifdef __cplusplus
}
#endif
#endif
