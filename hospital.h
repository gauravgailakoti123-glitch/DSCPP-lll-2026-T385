// hospital.h - OOP layer that ties patients/doctors to the C data structures
#ifndef HOSPITAL_H
#define HOSPITAL_H

#include "person.h"
#include "storage.h"

extern "C" {
#include "pq.h"
#include "cq.h"
}

#include <string>
#include <vector>

class Hospital {
    std::vector<Patient> patients_;
    std::vector<Doctor>  doctors_;
    PriorityQueue        triage_;        // emergency queue (by severity)
    CircularQueue        appointments_;  // fixed daily slots
    int                  nextPatientId_;
    int                  nextDoctorId_;
    std::string          patientFile_, doctorFile_;

public:
    static const int DAILY_SLOTS = 8;

    Hospital(const std::string &patientFile = "patients.txt",
             const std::string &doctorFile  = "doctors.txt",
             int dailySlots = DAILY_SLOTS);
    ~Hospital();
    Hospital(const Hospital &) = delete;
    Hospital &operator=(const Hospital &) = delete;

    // registration (ids are assigned automatically; patients 1001+, doctors 2001+)
    int addPatient(const std::string &name, int age, const std::string &complaint, int severity);
    int addDoctor (const std::string &name, int age, const std::string &specialization);

    const Patient *findPatient(int id) const;   // linear search (Phase-II baseline)
    const Doctor  *findDoctor (int id) const;
    const std::vector<Patient> &patients() const { return patients_; }
    const std::vector<Doctor>  &doctors()  const { return doctors_; }

    // emergency triage (priority queue)
    bool sendToTriage(int patientId);                 // uses the patient's severity
    bool serveNextEmergency(TriageEntry &out);
    const PriorityQueue &triage() const { return triage_; }

    // outpatient appointments (circular queue)
    // returns 0 ok, -1 unknown patient, -2 unknown doctor, -3 all slots taken
    int  bookAppointment(int patientId, int doctorId, Appointment &booked);
    bool serveNextAppointment(Appointment &out);
    const CircularQueue &appointments() const { return appointments_; }

    // persistence
    bool save() const;
    bool load(int *skipped = nullptr);   // false if no data files exist yet
};

#endif
