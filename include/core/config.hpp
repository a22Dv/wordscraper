/**
 * config.hpp
 *
 * Project-wide configuration file.
 */

#pragma once

/* --------------------------- Main configuration --------------------------- */

// #define WSC_NOPROFILE // Turns off profiling.
// #define WSC_NOASSERT // Discards assert checks (Discarded on release builds regardless).
#define WSC_NOCATCH // Discards main try-catch block.
#define WSC_LOGGING_ERROR // Enables logging only for logs with a severity level >= ERROR
#define WSC_LOGGING_INFO // Enables all logging types. Severity level >= INFO.


/* --------------------------------- Testing -------------------------------- */

/**
 * Swaps int main() in main.cpp to a test function.
 * Only one of these defines can be active at a time.
 */

#define WSC_TEST1 // N/A
// #define WSC_TEST2 // N/A
// #define WSC_TEST3 // N/A
// #define WSC_TEST4 // N/A
// #define WSC_TEST5 // N/A


/* --------------------------------- Checks --------------------------------- */

#ifdef WSC_TEST1 
#define WSC_TEST1CNT 1
#else
#define WSC_TEST1CNT 0
#endif

#ifdef WSC_TEST2
#define WSC_TEST2CNT 1
#else
#define WSC_TEST2CNT 0
#endif

#ifdef WSC_TEST3 
#define WSC_TEST3CNT 1
#else
#define WSC_TEST3CNT 0
#endif

#ifdef WSC_TEST4 
#define WSC_TEST4CNT 1
#else
#define WSC_TEST4CNT 0
#endif

#ifdef WSC_TEST5
#define WSC_TEST5CNT 1
#else
#define WSC_TEST5CNT 0
#endif

#define WSC_TESTCNT WSC_TEST1CNT + WSC_TEST2CNT + WSC_TEST3CNT + WSC_TEST4CNT + WSC_TEST5CNT
static_assert(WSC_TESTCNT <= 1);

