#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
#include <QObject>
class Log : public QObject {
	Q_OBJECT
public:
	/// Indicates that a new message is available to be logged.
	Q_SIGNAL void newMessage(const QString &);
	/// Sends a new message signal from the global singleton. This method is thread-safe.
	static void sendMessage(const QString &);
	/// Returns a global singleton. This method is thread-safe.
	static Log * instance();
};
#define LOG_H_EXPORT_C extern "C"
#else
#define LOG_H_EXPORT_C
#endif

LOG_H_EXPORT_C void r_printf(const char * format, ...);

#endif // LOG_H
