#ifndef URLRESOLVER_H
#define URLRESOLVER_H

#include <QtCore>
#include <QtNetwork>

class UrlResolver {
public:
    static QUrl resolve(const QUrl &origUrl);
private:
    static QNetworkRequest request(const QUrl &url);
};

#endif // URLRESOLVER_H
