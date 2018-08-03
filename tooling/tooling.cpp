// https://github.com/KubaO/stackoverflown/tree/master/tooling

#include "tooling.h"

#include <QDebug>
#include <QTime>

namespace tooling {

void showTime(const char *name) {
   auto time = QTime::currentTime().toString("HH:mm:ss.zzz");
   if (name)
      qDebug() << time << name;
   else
      qDebug() << time;
}

bool isAncestorOf(QObject *ancestor, QObject *obj) {
   while (obj && obj != ancestor) obj = obj->parent();
   return obj && obj == ancestor;
}

}  // namespace tooling
