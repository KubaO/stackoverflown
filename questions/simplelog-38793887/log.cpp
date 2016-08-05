#include "Log.h"
#include <cstdarg>

Q_GLOBAL_STATIC(Log, log)

Log * Log::instance() { return log; }

void Log::sendMessage(const QString & msg) {
	emit log->newMessage(msg);
}

LOG_H_EXPORT_C void r_printf(const char * format, ...) {
	va_list argList;
	va_start(argList, format);
	auto msg = QString::vasprintf(format, argList);
	va_end(argList);
	Log::sendMessage(msg);
}

