/**
 * config.hpp
 *
 * Project-wide configuration settings.
 */

#pragma once
/* ------------------------------ Configuration ----------------------------- */

// #define WSR_NOASSERT // Discards assertions for debug builds (Always disabled for release).
// #define WSR_NOPROFILE // Disables profiling data.
// #define WSR_LOGCRITICAL // Only logs messages with a severity >= LogSeverity::CRITICAL
// #define WSR_LOGERROR // Only logs messages with a severity >= LogSeverity::LOG_ERROR.
// #define WSR_LOGINFO // Only logs messages with a severity >= LogSeverity::LOG_INFO
#define WSR_LOGDEBUG // Only logs messages with a severity >= LogSeverity::LOG_DEBUG (Enable all)

/* ----------------------------- Implementation ----------------------------- */

#ifdef WSR_LOGINFO
#define WSR_LCINFO 1
#else
#define WSR_LCINFO 0
#endif

#ifdef WSR_LOGERROR
#define WSR_LCERROR 1
#else
#define WSR_LCERROR 0
#endif

#ifdef WSR_LOGDEBUG
#define WSR_LCDEBUG 1
#else
#define WSR_LCDEBUG 0
#endif

#ifdef WSR_LOGCRITICAL
#define WSR_LCCRITICAL 1
#else
#define WSR_LCCRITICAL 0
#endif

#define WSR_LCC WSR_LCINFO + WSR_LCERROR + WSR_LCDEBUG + WSR_LCCRITICAL

static_assert(WSR_LCC <= 1); // Only one logging configuration can be active at any time.
