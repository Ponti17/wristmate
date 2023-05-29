#include <time.h>

void printLocalDate(char* dayName, int* dayNumber, char* month, int* weekNumber) {
  time_t now;
  struct tm timeInfo;
  char buffer[80];

  time(&now);
  localtime_r(&now, &timeInfo);

  strftime(dayName, 10, "%A", &timeInfo);
  strftime(buffer, sizeof(buffer), "%d", &timeInfo);
  *dayNumber = atoi(buffer);
  strftime(month, 10, "%B", &timeInfo);
  *weekNumber = timeInfo.tm_yday / 7 + 1;
}
