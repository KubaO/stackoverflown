/****************************************************************************
**
** Copyright (C) 2017 Kuba Ober
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the "Software"),
** to deal in the Software without restriction, including without limitation
** the rights to use, copy, modify, merge, publish, distribute, sublicense,
** and/or sell copies of the Software, and to permit persons to whom the
** Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included
** in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
** OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
**  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
** THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
** FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
** DEALINGS IN THE SOFTWARE.
**
****************************************************************************/

#include "networkreplycache.h"

#include <utils/networkaccessmanager.h>

#include <QEventLoop>

namespace StackOverflow {
namespace Internal {

CachedReply::CachedReply(QNetworkReply *reply, int timeout, qint64 maxSize, QObject *parent) :
    QObject(parent),
    timeout(timeout),
    maxSize(maxSize),
    reply(reply)
{
    Q_ASSERT(timeout > 0);
    QTimer::singleShot(timeout, Qt::CoarseTimer, reply, [reply]{
        if (!reply->isFinished())
            reply->abort();
    });
    QObject::connect(reply, &QNetworkReply::finished, this, [this]{
        data = this->reply->readAll();
    });
    QObject::connect(reply, &QIODevice::readyRead, this, [reply, maxSize]{
        if (reply->bytesAvailable() > maxSize)
            reply->abort();
    });
}

void CachedReply::waitForFinished() {
    QEventLoop loop;
    QObject::connect(reply.data(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    if (!reply->isFinished())
        loop.exec();
}

CachedReply * NetworkReplyCache::getReply(const Key &key, const QNetworkRequest &request, int timeout, qint64 maxSize)
{
    auto obj = object(id);
    if (!obj || obj->hasError()) { // we'll re-fetch if there was an error last time
        auto cost = maxSize / 1024;
        if (cost > maxCost())
            return nullptr;
        reply = Utils::NetworkAccessManager::instance()->get(request);
        obj = new CachedReply(reply, timeout, maxSize);
        auto inserted = insert(key, obj, cost);
        Q_ASSERT(inserted);
    }
    return obj;
}

} // namespace Internal
} // namespace StackOverflow
