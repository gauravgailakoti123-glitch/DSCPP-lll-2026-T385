// storage.h - file-based persistence (text files, one record per line)
#ifndef STORAGE_H
#define STORAGE_H

#include "person.h"
#include <string>
#include <vector>

namespace storage {

// Return true on success. Loading skips malformed lines and reports how many.
bool savePatients(const std::string &path, const std::vector<Patient> &v);
bool saveDoctors (const std::string &path, const std::vector<Doctor>  &v);
bool loadPatients(const std::string &path, std::vector<Patient> &out, int *skipped = nullptr);
bool loadDoctors (const std::string &path, std::vector<Doctor>  &out, int *skipped = nullptr);

} // namespace storage

#endif
