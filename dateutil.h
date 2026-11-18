// dateutil.h - tiny calendar helpers. A date is an int YYYYMMDD (e.g. 20261117).
#ifndef DATEUTIL_H
#define DATEUTIL_H

#include <cstdio>
#include <ctime>
#include <string>

namespace dateutil {

inline long toDays(int date)                   // days since 1970-01-01
{
    int y = date / 10000, m = date / 100 % 100, d = date % 100;
    y -= m <= 2;
    long era = (y >= 0 ? y : y - 399) / 400;
    unsigned yoe = (unsigned)(y - era * 400);
    unsigned doy = (unsigned)((153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1);
    unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + (long)doe - 719468;
}

inline int fromDays(long z)
{
    z += 719468;
    long era = (z >= 0 ? z : z - 146096) / 146097;
    unsigned doe = (unsigned)(z - era * 146097);
    unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    int y = (int)yoe + (int)era * 400;
    unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    unsigned mp = (5 * doy + 2) / 153;
    int d = (int)(doy - (153 * mp + 2) / 5 + 1);
    int m = (int)(mp < 10 ? mp + 3 : mp - 9);
    y += (m <= 2);
    return y * 10000 + m * 100 + d;
}

inline bool isValid(int date)
{
    if (date < 10000101 || date > 99991231) return false;
    int y = date / 10000, m = date / 100 % 100, d = date % 100;
    if (m < 1 || m > 12 || d < 1) return false;
    static const int dim[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int max = dim[m - 1];
    if (m == 2 && ((y % 4 == 0 && y % 100 != 0) || y % 400 == 0)) max = 29;
    return d <= max;
}

inline int addDays(int date, int n) { return fromDays(toDays(date) + n); }

inline int weekday(int date)                   // 0 = Sunday ... 6 = Saturday
{
    long w = (toDays(date) + 4) % 7;           // 1970-01-01 was a Thursday
    return (int)(w < 0 ? w + 7 : w);
}

inline const char *dayName(int date)
{
    static const char *n[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return n[weekday(date)];
}

inline std::string format(int date)            // "2026-11-17"
{
    char b[16];
    snprintf(b, sizeof b, "%04d-%02d-%02d", date / 10000, date / 100 % 100, date % 100);
    return b;
}

inline bool parse(const std::string &s, int &out)   // accepts "YYYY-MM-DD"
{
    int y, m, d;
    char extra;
    if (sscanf(s.c_str(), "%d-%d-%d%c", &y, &m, &d, &extra) != 3) return false;
    int date = y * 10000 + m * 100 + d;
    if (!isValid(date)) return false;
    out = date;
    return true;
}

inline int today()
{
    time_t t = time(nullptr);
    tm *lt = localtime(&t);
    return (lt->tm_year + 1900) * 10000 + (lt->tm_mon + 1) * 100 + lt->tm_mday;
}

inline std::string hourLabel(int hour)         // 9 -> "09:00"
{
    char b[8];
    snprintf(b, sizeof b, "%02d:00", hour);
    return b;
}

} // namespace dateutil

#endif
