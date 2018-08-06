// https://github.com/KubaO/stackoverflown/tree/master/tooling

#include "backport.h"

namespace tooling {

QTimerImpl::QTimerImpl(QObject *parent) : Q_QTimer(parent) {}

QTimerImpl::~QTimerImpl() {}

bool QSaveFileImpl::commit() {
   if (this->error() != QFile::NoError) return false;
   close();
   return true;
}

}  // namespace tooling
