// person.h - OOP model: Person (abstract base) -> Patient, Doctor
#ifndef PERSON_H
#define PERSON_H

#include <string>

// Encapsulation: data members are private/protected, accessed via getters.
class Person {
protected:
    int         id_;
    std::string name_;
    int         age_;

public:
    Person(int id, const std::string &name, int age);
    virtual ~Person() {}

    int                getId()   const { return id_; }
    const std::string &getName() const { return name_; }
    int                getAge()  const { return age_; }

    // Polymorphism: each subclass supplies its own behaviour.
    virtual std::string describe()  const = 0;   // one-line display text
    virtual std::string serialize() const = 0;   // one line for the data file
    virtual std::string role()      const = 0;   // "Patient" / "Doctor"
};

class Patient : public Person {
    std::string complaint_;
    int         severity_;   // 1 (minor) .. 5 (critical)

public:
    Patient(int id, const std::string &name, int age,
            const std::string &complaint, int severity);

    const std::string &getComplaint() const { return complaint_; }
    int                getSeverity()  const { return severity_; }

    std::string describe()  const override;
    std::string serialize() const override;
    std::string role()      const override { return "Patient"; }
};

class Doctor : public Person {
    std::string specialization_;

public:
    Doctor(int id, const std::string &name, int age,
           const std::string &specialization);

    const std::string &getSpecialization() const { return specialization_; }

    std::string describe()  const override;
    std::string serialize() const override;
    std::string role()      const override { return "Doctor"; }
};

#endif
