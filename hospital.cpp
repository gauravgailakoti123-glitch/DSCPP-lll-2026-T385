// hospital.cpp
#include "hospital.h"

Hospital::Hospital(const std::string &patientFile, const std::string &doctorFile, int dailySlots)
    : nextPatientId_(1001), nextDoctorId_(2001),
      patientFile_(patientFile), doctorFile_(doctorFile)
{
    pq_init(&triage_, 16);
    cq_init(&appointments_, dailySlots);
}

Hospital::~Hospital()
{
    pq_free(&triage_);
    cq_free(&appointments_);
}

int Hospital::addPatient(const std::string &name, int age, const std::string &complaint, int severity)
{
    int id = nextPatientId_++;
    patients_.push_back(Patient(id, name, age, complaint, severity));
    return id;
}

int Hospital::addDoctor(const std::string &name, int age, const std::string &specialization)
{
    int id = nextDoctorId_++;
    doctors_.push_back(Doctor(id, name, age, specialization));
    return id;
}

const Patient *Hospital::findPatient(int id) const
{
    for (const Patient &p : patients_)
        if (p.getId() == id) return &p;
    return nullptr;
}

const Doctor *Hospital::findDoctor(int id) const
{
    for (const Doctor &d : doctors_)
        if (d.getId() == id) return &d;
    return nullptr;
}

bool Hospital::sendToTriage(int patientId)
{
    const Patient *p = findPatient(patientId);
    if (!p) return false;
    return pq_push(&triage_, p->getId(), p->getSeverity(), p->getName().c_str()) == 0;
}

bool Hospital::serveNextEmergency(TriageEntry &out)
{
    return pq_pop(&triage_, &out) == 0;
}

int Hospital::bookAppointment(int patientId, int doctorId, Appointment &booked)
{
    if (!findPatient(patientId)) return -1;
    if (!findDoctor(doctorId))   return -2;
    if (cq_enqueue(&appointments_, patientId, doctorId, &booked) != 0) return -3;
    return 0;
}

bool Hospital::serveNextAppointment(Appointment &out)
{
    return cq_dequeue(&appointments_, &out) == 0;
}

bool Hospital::save() const
{
    return storage::savePatients(patientFile_, patients_) &&
           storage::saveDoctors(doctorFile_, doctors_);
}

bool Hospital::load(int *skipped)
{
    int s1 = 0, s2 = 0;
    bool okP = storage::loadPatients(patientFile_, patients_, &s1);
    bool okD = storage::loadDoctors(doctorFile_, doctors_, &s2);
    if (skipped) *skipped = s1 + s2;

    // continue id numbering after the highest loaded id
    for (const Patient &p : patients_)
        if (p.getId() >= nextPatientId_) nextPatientId_ = p.getId() + 1;
    for (const Doctor &d : doctors_)
        if (d.getId() >= nextDoctorId_) nextDoctorId_ = d.getId() + 1;
    return okP || okD;
}
