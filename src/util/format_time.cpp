#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

// formats a time

string format_time(const unsigned long& time) {
   time_t a = time;
   const tm* t = gmtime(&a);
   char buf[128];

   strftime(buf, 128, "%Y-%m-%d %H:%M:%S", t);

   return string(buf);
}

string format_elapsed(double seconds) {
   const unsigned minute = 60;
   const unsigned hour = 60 * minute;
   const unsigned day = 24 * hour;

   unsigned days = unsigned(seconds / day);
   unsigned remaining = unsigned(seconds) % day;
   unsigned hours = remaining / hour;
   remaining %= hour;
   unsigned minutes = remaining / minute;
   remaining %= minute;

   ostringstream s;
   if (days) {
      s << days << "d ";
   }

   s << hours << ":" << setw(2) << setfill('0') << minutes
     << ":" << setw(2) << setfill('0') << remaining;

   return s.str();
}
