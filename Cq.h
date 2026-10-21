/* cq.h - Circular queue for fixed daily outpatient appointment slots. Pure C. */
#ifndef CQ_H
#define CQ_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int patient_id;
    int doctor_id;
    int slot_no;       /* 1 .. capacity, the slot number printed on the token */
} Appointment;

typedef struct {
    Appointment *data;
    int          capacity;
    int          front;      /* index of the oldest element */
    int          count;
    int          next_slot;  /* slot number given to the next booking */
} CircularQueue;

int  cq_init(CircularQueue *cq, int capacity);      /* 0 on success */
void cq_free(CircularQueue *cq);
int  cq_enqueue(CircularQueue *cq, int patient_id, int doctor_id, Appointment *booked); /* 0 ok, -1 full */
int  cq_dequeue(CircularQueue *cq, Appointment *out);   /* 0 ok, -1 empty */
int  cq_get(const CircularQueue *cq, int i, Appointment *out); /* i-th from front, 0 ok */
int  cq_size(const CircularQueue *cq);
int  cq_is_full(const CircularQueue *cq);
int  cq_is_empty(const CircularQueue *cq);

#ifdef __cplusplus
}
#endif
#endif
