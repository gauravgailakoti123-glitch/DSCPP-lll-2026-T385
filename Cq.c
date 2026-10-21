/* cq.c - fixed-size circular queue (array + modulo wrap-around). All ops O(1). */
#include "cq.h"
#include <stdlib.h>

int cq_init(CircularQueue *cq, int capacity)
{
    if (capacity < 1)
        return -1;
    cq->data = (Appointment *)malloc((size_t)capacity * sizeof(Appointment));
    if (!cq->data)
        return -1;
    cq->capacity = capacity;
    cq->front = 0;
    cq->count = 0;
    cq->next_slot = 1;
    return 0;
}

void cq_free(CircularQueue *cq)
{
    free(cq->data);
    cq->data = NULL;
    cq->capacity = cq->front = cq->count = 0;
}

int cq_enqueue(CircularQueue *cq, int patient_id, int doctor_id, Appointment *booked)
{
    if (cq->count == cq->capacity)
        return -1;                                   /* all slots taken */
    int rear = (cq->front + cq->count) % cq->capacity;
    Appointment *a = &cq->data[rear];
    a->patient_id = patient_id;
    a->doctor_id = doctor_id;
    a->slot_no = cq->next_slot;
    cq->next_slot = cq->next_slot % cq->capacity + 1; /* slot numbers cycle 1..capacity */
    cq->count++;
    if (booked)
        *booked = *a;
    return 0;
}

int cq_dequeue(CircularQueue *cq, Appointment *out)
{
    if (cq->count == 0)
        return -1;
    if (out)
        *out = cq->data[cq->front];
    cq->front = (cq->front + 1) % cq->capacity;
    cq->count--;
    return 0;
}

int cq_get(const CircularQueue *cq, int i, Appointment *out)
{
    if (i < 0 || i >= cq->count)
        return -1;
    *out = cq->data[(cq->front + i) % cq->capacity];
    return 0;
}

int cq_size(const CircularQueue *cq)     { return cq->count; }
int cq_is_full(const CircularQueue *cq)  { return cq->count == cq->capacity; }
int cq_is_empty(const CircularQueue *cq) { return cq->count == 0; }
