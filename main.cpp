// main.cpp - menu-driven console interface
#include "hospital.h"
#include "benchlib.h"
#include "specialty.h"
#include <cstdio>
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

// date as YYYY-MM-DD; an empty answer means today
static int readDate(const Hospital &h, const string &prompt)
{
    for (;;) {
        string s = readLine(prompt + " (YYYY-MM-DD, Enter = today " + dateutil::format(h.today()) + "): ");
        if (s.empty()) return h.today();
        int d;
        if (dateutil::parse(s, d)) return d;
        cout << "  Not a valid date.\n";
    }
}

static string patientLabel(const Hospital &h, int id)
{
    const Patient *p = h.findPatient(id);
    return to_string(id) + (p ? " " + p->getName() : string());
}

static string doctorLabel(const Hospital &h, int id)
{
    const Doctor *d = h.findDoctor(id);
    return to_string(id) + (d ? " Dr. " + d->getName() + " (" + d->getSpecialization() + ")" : string());
}

static void printBooking(const Hospital &h, const Booking &b)
{
    cout << "  " << dateutil::format(b.date) << " (" << dateutil::dayName(b.date) << ") "
         << dateutil::hourLabel(b.hour) << "  |  " << doctorLabel(h, b.doctorId)
         << "  |  patient " << patientLabel(h, b.patientId) << "\n";
}

// ------------------------------------------------------------- registration

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
    const Patient *p = h.findPatient(id);           // hash index
    if (p) cout << "  " << p->describe() << "\n";
    else   cout << "  No patient with ID " << id << ".\n";
}

static void listSorted(const Hospital &h)
{
    vector<const Patient *> v = h.patientsSortedById();   // in-order walk of the AVL tree
    cout << "--- Patients sorted by ID (AVL in-order, " << v.size() << ") ---\n";
    for (const Patient *p : v) cout << "  " << p->describe() << "\n";
}

// ------------------------------------------------------------------ triage

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

// ------------------------------------------------- schedules / appointments

static void autoAssign(Hospital &h)
{
    int pid = readInt("Patient ID: ", 1, 999999);
    const Patient *p = h.findPatient(pid);
    if (!p) { cout << "  No patient with ID " << pid << ".\n"; return; }
    int from = readDate(h, "Earliest date");
    AutoAssignment a;
    switch (h.autoAssign(pid, from, a)) {
    case 0:
        cout << "  Complaint \"" << p->getComplaint() << "\" -> needs " << a.needed << ".\n";
        if (a.fallback)
            cout << "  No " << a.needed << " doctor available, so a " << a.specialty << " doctor was assigned.\n";
        cout << "  Booked:\n";
        printBooking(h, a.booking);
        h.save();
        break;
    case -2: cout << "  No doctor for " << specialty::forComplaint(p->getComplaint()) << " (or General Medicine) is registered.\n"; break;
    case -3: cout << "  No free slot in the next " << Hospital::LOOKAHEAD_DAYS << " days.\n"; break;
    default: cout << "  Could not book.\n"; break;
    }
}

static void bookManual(Hospital &h)
{
    int pid = readInt("Patient ID: ", 1, 999999);
    int did = readInt("Doctor ID: ", 1, 999999);
    int date = readDate(h, "Date");
    int hour = readInt("Hour (9-16, one-hour slots): ", 9, 16);
    Booking b;
    switch (h.bookAppointment(pid, did, date, hour, b)) {
    case 0:  cout << "  Booked:\n"; printBooking(h, b); h.save(); break;
    case -1: cout << "  No patient with ID " << pid << ".\n"; break;
    case -2: cout << "  No doctor with ID " << did << ".\n"; break;
    case -3: cout << "  That slot is already taken.\n"; break;
    case -4: cout << "  Not bookable: date is in the past, a Sunday (closed), or hour is outside 09-16.\n"; break;
    default: cout << "  That patient already has an appointment at that time.\n"; break;
    }
}

static void showSchedule(const Hospital &h)
{
    int did = readInt("Doctor ID: ", 1, 999999);
    const Doctor *d = h.findDoctor(did);
    if (!d) { cout << "  No doctor with ID " << did << ".\n"; return; }
    int date = readDate(h, "Date");
    cout << "  Dr. " << d->getName() << " (" << d->getSpecialization() << ") on "
         << dateutil::format(date) << " " << dateutil::dayName(date) << "\n";
    if (!Hospital::isWorkingDay(date)) { cout << "  Closed on Sundays.\n"; return; }
    vector<int> slots = h.daySchedule(did, date);
    for (int i = 0; i < Hospital::DAILY_SLOTS; i++) {
        cout << "    " << dateutil::hourLabel(Hospital::FIRST_HOUR + i) << "  ";
        if (slots[(size_t)i]) cout << "patient " << patientLabel(h, slots[(size_t)i]) << "\n";
        else cout << "free\n";
    }
}

static void showBookings(const Hospital &h)
{
    vector<Booking> v = h.bookings();
    cout << "--- Appointments (" << v.size() << ") ---\n";
    for (const Booking &b : v) printBooking(h, b);
}

static void cancelBooking(Hospital &h)
{
    int did = readInt("Doctor ID: ", 1, 999999);
    int date = readDate(h, "Date");
    int hour = readInt("Hour (9-16): ", 9, 16);
    if (h.cancelAppointment(date, did, hour)) { cout << "  Cancelled.\n"; h.save(); }
    else cout << "  No such appointment.\n";
}

static void checkIn(Hospital &h)
{
    int pid = readInt("Patient ID arriving for today's appointment: ", 1, 999999);
    Appointment t;
    switch (h.checkIn(pid, t)) {
    case 0:  cout << "  Checked in. Token #" << t.slot_no << " for " << doctorLabel(h, t.doctor_id) << ".\n"; break;
    case -1: cout << "  No appointment today for that patient.\n"; break;
    case -3: cout << "  Token line is full (" << h.tokens().capacity << " tokens).\n"; break;
    default: cout << "  Already checked in.\n"; break;
    }
}

static void serveToken(Hospital &h)
{
    Appointment a;
    if (h.serveNextToken(a))
        cout << "  Next: token #" << a.slot_no << ", patient " << patientLabel(h, a.patient_id)
             << " -> " << doctorLabel(h, a.doctor_id) << "\n";
    else
        cout << "  Nobody is waiting.\n";
}

static void showTokens(const Hospital &h)
{
    const CircularQueue &q = h.tokens();
    cout << "  " << cq_size(&q) << " of " << q.capacity << " tokens in use\n";
    for (int i = 0; i < cq_size(&q); i++) {
        Appointment a;
        cq_get(&q, i, &a);
        cout << "    token #" << a.slot_no << ": patient " << patientLabel(h, a.patient_id)
             << " -> " << doctorLabel(h, a.doctor_id) << "\n";
    }
}

// --------------------------------------------------- structures / benchmark

static void showStats(const Hospital &h)
{
    IndexStats s = h.indexStats();
    cout << "  Patient records: " << s.patients << "\n"
         << "  BST height (unbalanced): " << s.bstHeight << "\n"
         << "  AVL height (balanced):   " << s.avlHeight << "   (" << s.avlRotations << " rotations so far)\n"
         << "  Hash index: " << s.hashSize << " entries in " << s.hashCapacity << " buckets, load "
         << s.hashLoad << ", " << s.hashCollisions << " collisions\n";
    if (s.patients > 0) {
        int probe = h.patients().front().getId();
        cout << "  Lookup demo for ID " << probe << " is done by the hash index; AVL/BST/linear find the same record.\n";
    }
}

static void runBenchmark()
{
    cout << "  Running quick benchmark (a few seconds) ...\n\n";
    vector<bench::TreeRow>   t = bench::trees({100, 1000, 5000});
    vector<bench::SearchRow> s = bench::search({100, 1000, 10000}, 1000, 3);
    bench::TriageResult      q = bench::triage(5000, 7);
    cout << bench::textReport(t, s, q);
    cout << "\n  For the full benchmark run ./bench\n";
}

static void menu()
{
    cout << "\n===== Hospital Patient Management System =====\n"
            "  1. Register patient             11. Show doctor schedule\n"
            "  2. Register doctor              12. Show all appointments\n"
            "  3. List patients & doctors      13. Cancel appointment\n"
            "  4. Search patient by ID (hash)  14. Check in patient (token)\n"
            "  5. Patients sorted by ID (AVL)  15. Serve next token\n"
            "  6. Send patient to triage       16. Show token line\n"
            "  7. Serve next emergency         17. Structure statistics\n"
            "  8. Show triage queue            18. Run benchmark\n"
            "  9. Auto-assign doctor & book    19. Save to files\n"
            " 10. Book appointment manually    20. Load from files\n"
            "  0. Exit\n";
}

int main()
{
    Hospital h;
    int skipped = 0;
    if (h.load(&skipped))
        cout << "Loaded " << h.patients().size() << " patients, " << h.doctors().size()
             << " doctors and " << h.bookings().size() << " appointments from files"
             << (skipped ? " (some bad lines skipped)" : "") << ".\n";

    for (;;) {
        menu();
        int c = readInt("Choice: ", 0, 20);
        switch (c) {
        case 1:  registerPatient(h);   break;
        case 2:  registerDoctor(h);    break;
        case 3:  listAll(h);           break;
        case 4:  searchPatient(h);     break;
        case 5:  listSorted(h);        break;
        case 6:  toTriage(h);          break;
        case 7:  serveEmergency(h);    break;
        case 8:  showTriage(h);        break;
        case 9:  autoAssign(h);        break;
        case 10: bookManual(h);        break;
        case 11: showSchedule(h);      break;
        case 12: showBookings(h);      break;
        case 13: cancelBooking(h);     break;
        case 14: checkIn(h);           break;
        case 15: serveToken(h);        break;
        case 16: showTokens(h);        break;
        case 17: showStats(h);         break;
        case 18: runBenchmark();       break;
        case 19: cout << (h.save() ? "  Saved.\n" : "  Save failed.\n"); break;
        case 20: cout << (h.load(&skipped) ? "  Loaded.\n" : "  No data files found.\n"); break;
        case 0:
            if (h.save()) cout << "Records saved. Goodbye.\n";
            else          cout << "Warning: could not save records. Goodbye.\n";
            return 0;
        }
        if (c == 1 || c == 2) h.save();      // keep files in sync after every registration
    }
}
