// specialty.h - decides which kind of doctor a complaint needs
#ifndef SPECIALTY_H
#define SPECIALTY_H

#include "person.h"
#include <string>
#include <vector>

namespace specialty {

// Canonical specialty names, e.g. "Cardiology", "Neurology", "General Medicine".
const std::vector<std::string> &all();

// Best specialty for the complaint text (keyword scoring). "General Medicine" if nothing matches.
std::string forComplaint(const std::string &complaint);

// Ordered list of specialties to try for this patient:
//   [Pediatrics if age <= 12], the complaint's specialty, General Medicine (no duplicates).
std::vector<std::string> candidatesFor(const Patient &p);

// True if the doctor's free-text specialization belongs to the canonical specialty
// ("cardiologist" / "Cardiology" / "heart" -> Cardiology, "dentist" -> Dentistry ...).
bool doctorHas(const Doctor &d, const std::string &canonical);

} // namespace specialty

#endif
