// person.cpp
#include "person.h"
#include <sstream>

// '|' is the field separator in data files, so it must not appear inside a field.
static std::string clean(std::string s)
{
    for (char &c : s)
        if (c == '|' || c == '\n' || c == '\r')
            c = '/';
    return s;
}

Person::Person(int id, const std::string &name, int age)
    : id_(id), name_(clean(name)), age_(age) {}

Patient::Patient(int id, const std::string &name, int age,
                 const std::string &complaint, int severity)
    : Person(id, name, age), complaint_(clean(complaint)), severity_(severity) {}

std::string Patient::describe() const
{
    std::ostringstream o;
    o << "[Patient #" << id_ << "] " << name_ << ", age " << age_
      << ", complaint: " << complaint_ << ", severity " << severity_ << "/5";
    return o.str();
}

std::string Patient::serialize() const
{
    std::ostringstream o;
    o << id_ << '|' << name_ << '|' << age_ << '|' << complaint_ << '|' << severity_;
    return o.str();
}

Doctor::Doctor(int id, const std::string &name, int age,
               const std::string &specialization)
    : Person(id, name, age), specialization_(clean(specialization)) {}

std::string Doctor::describe() const
{
    std::ostringstream o;
    o << "[Doctor #" << id_ << "] Dr. " << name_ << ", age " << age_
      << ", specialization: " << specialization_;
    return o.str();
}

std::string Doctor::serialize() const
{
    std::ostringstream o;
    o << id_ << '|' << name_ << '|' << age_ << '|' << specialization_;
    return o.str();
}
