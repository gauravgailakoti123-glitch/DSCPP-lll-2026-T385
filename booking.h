// booking.h - one booked slot in a doctor's schedule
#ifndef BOOKING_H
#define BOOKING_H

struct Booking {
    int date;        // YYYYMMDD
    int hour;        // start hour, 24h clock (the slot lasts one hour)
    int doctorId;
    int patientId;
};

#endif
