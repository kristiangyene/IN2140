#include "util.h"

/*
 * This function is not a macro, so we can not use what the advanced.c example
 * uses (the __FILE__, __LINE__, etc) that makes it easy to locate the line in
 * question. We can however use it just like a normal function, which is easier
 * to understand.
 *
 * The function prints a message according to the activated logging level that
 * is controlled by the global logger_level variable. Three defines give the
 * three defined logger levels (you can add more!).
 *
 * The type argument expects a string being either "DEBUG", "INFO", or "OUT",
 * that are defined in three defines for ease of use.
 *
 * The function behaves a little differently depending on the type given when
 * called. A typical line will look like this:
 * [16:26:52][DEBUG] test
 *
 * The color for the type-text (DEBUG, INFO, OUT) depends on the three defines
 * CNRM, CMAG, CYEL (you can change the values to get different colors in your
 * terminal!).
 *
 * You can also look at 'man 2 time' and the associated functions if you want
 * to change the time format in the output.
 *
 * If "OUT" is given as a type in the call, the print happens regardless of
 * the state of the logger_level variable. This is intended as a regular printf
 * with timestamp and formatted like the rest of the output.
 */

int logger_level = LOGGER_LEVEL_DEBUG;

void logger(char* type, char* s) {
    if((strcmp(type, LOGGER_DEBUG) == 0 && logger_level > 1) ||
       (strcmp(type, LOGGER_INFO) == 0 && logger_level > 0)  ||
       (strcmp(type, LOGGER_ERROR) == 0)) {

        char* color;
        if (strcmp(type, LOGGER_DEBUG) == 0) {
            color = CMAG;
        } else if (strcmp(type, LOGGER_INFO) == 0) {
            color = CYEL;
        } else {
            color = CRED;
        }

        time_t rawtime;
        struct tm* timeinfo;
        struct timespec spec;

        time(&rawtime);
        timeinfo = localtime(&rawtime);
        char* datetime = asctime(timeinfo);
        datetime[strlen(datetime) - 6] = 0;
        datetime = &datetime[strlen(datetime) - 8];

        clock_gettime(CLOCK_REALTIME, &spec);

        printf("[%s.%09ld][%s%-5s%s] %s\n", datetime, spec.tv_nsec, color, type, CNRM, s);
    }
}
