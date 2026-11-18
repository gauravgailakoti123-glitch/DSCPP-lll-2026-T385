// specialty.cpp - keyword rules for automatic doctor assignment
#include "specialty.h"
#include <cctype>

namespace specialty {

namespace {

struct Rule {
    const char *name;
    std::vector<const char *> keywords;   // matched at the start of a word in the complaint
    std::vector<const char *> aliases;    // matched at the start of a word in a doctor's specialization
};

const std::vector<Rule> &rules()
{
    static const std::vector<Rule> r = {
        {"Cardiology",       {"chest pain", "heart", "palpitation", "cardiac", "angina", "blood pressure", "bp"},
                             {"cardio", "heart"}},
        {"Neurology",        {"stroke", "brain", "seizure", "epilepsy", "migraine", "headache", "paralysis", "numb", "dizz", "faint"},
                             {"neuro", "brain"}},
        {"Dentistry",        {"tooth", "teeth", "dental", "gum", "cavity", "toothache", "mouth ulcer"},
                             {"dent", "oral"}},
        {"ENT",              {"throat", "ear", "nose", "nasal", "sinus", "tonsil", "hearing", "voice"},
                             {"ent", "otolaryn", "ear"}},
        {"Orthopedics",      {"fracture", "bone", "joint", "knee", "back pain", "spine", "sprain", "shoulder", "ligament", "hip"},
                             {"ortho", "bone", "spine"}},
        {"Dermatology",      {"skin", "rash", "acne", "itch", "eczema", "allergy", "hives"},
                             {"derma", "skin"}},
        {"Nephrology",       {"kidney", "urine", "urinary", "stone", "renal", "dialysis"},
                             {"nephro", "uro", "kidney", "renal"}},
        {"Gastroenterology", {"stomach", "abdominal", "abdomen", "vomit", "diarrhea", "diarrhoea", "ulcer", "liver", "acidity", "digestion", "jaundice"},
                             {"gastro", "stomach", "liver", "hepat"}},
        {"Ophthalmology",    {"eye", "vision", "cataract", "blurred"},
                             {"ophthal", "eye", "optic"}},
        {"Gynecology",       {"pregnan", "period", "menstrual", "uterus", "ovar", "pcos"},
                             {"gyn", "obstet", "maternity"}},
        {"Pulmonology",      {"asthma", "breath", "lung", "tuberculosis", "wheez", "pneumonia"},
                             {"pulmo", "lung", "chest", "respir"}},
        {"Psychiatry",       {"anxiety", "depress", "stress", "insomnia", "panic", "mental"},
                             {"psych", "mental"}},
        {"Pediatrics",       {},
                             {"pedia", "paedia", "child"}},
        {"General Medicine", {"fever", "cold", "flu", "weakness", "fatigue", "viral", "infection", "body ache"},
                             {"general", "physician", "medicine", "family", "gp"}},
    };
    return r;
}

std::string lower(const std::string &s)
{
    std::string o = s;
    for (char &c : o) c = (char)tolower((unsigned char)c);
    return o;
}

// " " + text + " " contains " " + needle  -> needle appears at the start of a word
bool startsWord(const std::string &padded, const char *needle)
{
    return padded.find(std::string(" ") + needle) != std::string::npos;
}

std::string pad(const std::string &s)
{
    std::string o = " ";
    for (char c : lower(s)) o += isalnum((unsigned char)c) ? c : ' ';
    return o + " ";
}

} // namespace

const std::vector<std::string> &all()
{
    static std::vector<std::string> names;
    if (names.empty())
        for (const Rule &r : rules()) names.push_back(r.name);
    return names;
}

std::string forComplaint(const std::string &complaint)
{
    std::string text = pad(complaint);
    const Rule *best = nullptr;
    int bestScore = 0;
    for (const Rule &r : rules()) {
        int score = 0;
        for (const char *k : r.keywords)
            if (startsWord(text, k)) score++;
        if (score > bestScore) { bestScore = score; best = &r; }   // ties keep the earlier (more serious) rule
    }
    return best ? best->name : "General Medicine";
}

std::vector<std::string> candidatesFor(const Patient &p)
{
    std::vector<std::string> out;
    auto add = [&](const std::string &s) {
        for (const std::string &x : out) if (x == s) return;
        out.push_back(s);
    };
    if (p.getAge() <= 12) add("Pediatrics");
    add(forComplaint(p.getComplaint()));
    add("General Medicine");
    return out;
}

bool doctorHas(const Doctor &d, const std::string &canonical)
{
    std::string text = pad(d.getSpecialization());
    for (const Rule &r : rules()) {
        if (canonical != r.name) continue;
        for (const char *a : r.aliases)
            if (startsWord(text, a)) return true;
        // the canonical name itself, e.g. "Cardiology"
        if (startsWord(text, lower(r.name).c_str())) return true;
    }
    return false;
}

} // namespace specialty
