// storage.cpp
#include "storage.h"
#include <fstream>
#include <sstream>

namespace storage {

static std::vector<std::string> split(const std::string &line)
{
    std::vector<std::string> f;
    std::string cur;
    std::stringstream ss(line);
    while (std::getline(ss, cur, '|'))
        f.push_back(cur);
    return f;
}

static bool toInt(const std::string &s, int &out)
{
    try {
        size_t pos = 0;
        out = std::stoi(s, &pos);
        return pos == s.size();
    } catch (...) {
        return false;
    }
}

bool savePatients(const std::string &path, const std::vector<Patient> &v)
{
    std::ofstream f(path.c_str(), std::ios::trunc);
    if (!f) return false;
    for (const Patient &p : v) f << p.serialize() << '\n';
    return f.good();
}

bool saveDoctors(const std::string &path, const std::vector<Doctor> &v)
{
    std::ofstream f(path.c_str(), std::ios::trunc);
    if (!f) return false;
    for (const Doctor &d : v) f << d.serialize() << '\n';
    return f.good();
}

bool loadPatients(const std::string &path, std::vector<Patient> &out, int *skipped)
{
    std::ifstream f(path.c_str());
    if (!f) return false;               // missing file = nothing to load yet
    out.clear();
    int bad = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::vector<std::string> p = split(line);
        int id, age, sev;
        if (p.size() != 5 || !toInt(p[0], id) || !toInt(p[2], age) ||
            !toInt(p[4], sev) || sev < 1 || sev > 5) { bad++; continue; }
        out.push_back(Patient(id, p[1], age, p[3], sev));
    }
    if (skipped) *skipped = bad;
    return true;
}

bool loadDoctors(const std::string &path, std::vector<Doctor> &out, int *skipped)
{
    std::ifstream f(path.c_str());
    if (!f) return false;
    out.clear();
    int bad = 0;
    std::string line;
    while (std::getline(f, line)) {
        if (line.empty()) continue;
        std::vector<std::string> p = split(line);
        int id, age;
        if (p.size() != 4 || !toInt(p[0], id) || !toInt(p[2], age)) { bad++; continue; }
        out.push_back(Doctor(id, p[1], age, p[3]));
    }
    if (skipped) *skipped = bad;
    return true;
}

} // namespace storage
