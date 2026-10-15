// main.cpp - menu-driven console interface
#include "hospital.h"
#include <iostream>
#include <string>

using namespace std;

static string readLine(const string &prompt)
{
    cout << prompt;
    string s;
    if (!getline(cin, s)) { cout << "\n"; exit(0); }   // end of input
    return s;
}

static string readNonEmpty(const string &prompt)
{
    for (;;) {
        string s = readLine(prompt);
        if (!s.empty()) return s;
        cout << "  Value cannot be empty.\n";
    }
}

static int readInt(const string &prompt, int lo, int hi)
{
    for (;;) {
        string s = readLine(prompt);
        try {
            size_t pos = 0;
            int v = stoi(s, &pos);
            if (pos == s.size() && v >= lo && v <= hi) return v;
        } catch (...) {}
        cout << "  Please enter a number between " << lo << " and " << hi << ".\n";
    }
}

static void registerPatient(Hospital &h)
{
    string name = readNonEmpty("Patient name: ");
    int age = readInt("Age (0-120): ", 0, 120);
    string complaint = readNonEmpty("Complaint: ");
    int sev = readInt("Severity (1 = minor ... 5 = critical): ", 1, 5);
    int id = h.addPatient(name, age, complaint, sev);
    cout << "  Registered. Patient ID = " << id << "\n";
}

static void registerDoctor(Hospital &h)
{
    string name = readNonEmpty("Doctor name: ");
    int age = readInt("Age (21-80): ", 21, 80);
    string spec = readNonEmpty("Specialization: ");
    int id = h.addDoctor(name, age, spec);
    cout << "  Registered. Doctor ID = " << id << "\n";
}

static void listAll(const Hospital &h)
{
    cout << "--- Patients (" << h.patients().size() << ") ---\n";
    for (const Patient &p : h.patients()) cout << "  " << p.describe() << "\n";
    cout << "--- Doctors (" << h.doctors().size() << ") ---\n";
    for (const Doctor &d : h.doctors()) cout << "  " << d.describe() << "\n";
}

static void searchPatient(const Hospital &h)
{
    int id = readInt("Patient ID: ", 1, 999999);
    const Patient *p = h.findPatient(id);
    if (p) cout << "  " << p->describe() << "\n";
    else   cout << "  No patient with ID " << id << ".\n";
}

static void toTriage(Hospital &h)
{
    int id = readInt("Patient ID to send to emergency triage: ", 1, 999999);
    if (h.sendToTriage(id))
        cout << "  Added to triage queue (" << pq_size(&h.triage()) << " waiting).\n";
    else
        cout << "  No patient with ID " << id << ".\n";
}

static void serveEmergency(Hospital &h)
{
    TriageEntry e;
    if (h.serveNextEmergency(e))
        cout << "  Now attending: " << e.name << " (ID " << e.patient_id
             << "), severity " << e.severity << "/5\n";
    else
        cout << "  Triage queue is empty.\n";
}

static void showTriage(const Hospital &h)
{
    TriageEntry top;
    if (pq_peek(&h.triage(), &top) != 0) { cout << "  Triage queue is empty.\n"; return; }
    cout << "  " << pq_size(&h.triage()) << " waiting. Next to be served: "
         << top.name << " (severity " << top.severity << "/5)\n";
}

static void book(Hospital &h)
{
    int pid = readInt("Patient ID: ", 1, 999999);
    int did = readInt("Doctor ID: ", 1, 999999);
    Appointment a;
    switch (h.bookAppointment(pid, did, a)) {
    case 0:  cout << "  Booked. Slot #" << a.slot_no << " with doctor " << a.doctor_id << ".\n"; break;
    case -1: cout << "  No patient with ID " << pid << ".\n"; break;
    case -2: cout << "  No doctor with ID " << did << ".\n"; break;
    default: cout << "  All " << Hospital::DAILY_SLOTS << " daily slots are taken.\n"; break;
    }
}

static void serveAppointment(Hospital &h)
{
    Appointment a;
    if (h.serveNextAppointment(a))
        cout << "  Next appointment: slot #" << a.slot_no << ", patient " << a.patient_id
             << ", doctor " << a.doctor_id << "\n";
    else
        cout << "  No appointments booked.\n";
}

static void showAppointments(const Hospital &h)
{
    const CircularQueue &q = h.appointments();
    cout << "  " << cq_size(&q) << " of " << q.capacity << " slots booked\n";
    for (int i = 0; i < cq_size(&q); i++) {
        Appointment a;
        cq_get(&q, i, &a);
        cout << "    slot #" << a.slot_no << ": patient " << a.patient_id
             << " with doctor " << a.doctor_id << "\n";
    }
}

static void menu()
{
    cout << "\n===== Hospital Patient Management System =====\n"
            " 1. Register patient          8. Book appointment\n"
            " 2. Register doctor           9. Serve next appointment\n"
            " 3. List patients & doctors  10. Show appointments\n"
            " 4. Search patient by ID     11. Save to files\n"
            " 5. Send patient to triage   12. Load from files\n"
            " 6. Serve next emergency      0. Exit\n"
            " 7. Show triage queue\n";
}

int main()
{
    Hospital h;
    int skipped = 0;
    if (h.load(&skipped))
        cout << "Loaded " << h.patients().size() << " patients and " << h.doctors().size()
             << " doctors from files" << (skipped ? " (some bad lines skipped)" : "") << ".\n";

    for (;;) {
        menu();
        int c = readInt("Choice: ", 0, 12);
        switch (c) {
        case 1:  registerPatient(h);   break;
        case 2:  registerDoctor(h);    break;
        case 3:  listAll(h);           break;
        case 4:  searchPatient(h);     break;
        case 5:  toTriage(h);          break;
        case 6:  serveEmergency(h);    break;
        case 7:  showTriage(h);        break;
        case 8:  book(h);              break;
        case 9:  serveAppointment(h);  break;
        case 10: showAppointments(h);  break;
        case 11: cout << (h.save() ? "  Saved.\n" : "  Save failed.\n"); break;
        case 12: cout << (h.load(&skipped) ? "  Loaded.\n" : "  No data files found.\n"); break;
        case 0:
            if (h.save()) cout << "Records saved. Goodbye.\n";
            else          cout << "Warning: could not save records. Goodbye.\n";
            return 0;
        }
        if (c >= 1 && c <= 2) h.save();      // keep file in sync after every change
    }
}
