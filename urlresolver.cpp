#include "urlresolver.h"

class Sleeper : public QThread {
public:
    static void usleep(unsigned long usecs){QThread::usleep(usecs);}
    static void msleep(unsigned long msecs){QThread::msleep(msecs);}
    static void sleep(unsigned long secs){QThread::sleep(secs);}
};

QUrl UrlResolver::resolve(const QUrl &origUrl) {
    bool done=false;
    QNetworkAccessManager nam;
    QUrl lastRedirectUrl=origUrl;
    QNetworkRequest req=request(origUrl);
    QNetworkReply *rep=nam.head(req);
    while(!done) {
	while(!rep->isFinished()) {
	    qApp->processEvents();
	    Sleeper::msleep(500);
	    printf("wait for reply to continue address resolving...\n");
	}
	QUrl possibleRedirectUrl = rep->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if(!possibleRedirectUrl.isEmpty() &&
	    lastRedirectUrl!=possibleRedirectUrl) {
	    lastRedirectUrl=possibleRedirectUrl;
	    rep->deleteLater();
	    req=request(possibleRedirectUrl);
	    rep=nam.head(req);
	} else {
	    lastRedirectUrl=rep->url();
	    rep->deleteLater();
	    done=true;
	}
    }
    return lastRedirectUrl;
}

QNetworkRequest UrlResolver::request(const QUrl &url) {
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", "Wget/1.14 (linux-gnu)");
    return req;
}
