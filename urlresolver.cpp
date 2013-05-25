#include "urlresolver.h"

QUrl UrlResolver::resolve(const QUrl &origUrl) {
    bool done=false;
    QNetworkAccessManager nam;
    QUrl lastRedirectUrl=origUrl;
    QNetworkRequest req=request(origUrl);
    QNetworkReply *rep=nam.get(req);
    while(!done) {
	while(!rep->isFinished()) {
	    qApp->processEvents();
	}
	QUrl possibleRedirectUrl = rep->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
	if(!possibleRedirectUrl.isEmpty() &&
	    lastRedirectUrl!=possibleRedirectUrl) {
	    lastRedirectUrl=possibleRedirectUrl;
	    rep->deleteLater();
	    req=request(possibleRedirectUrl);
	    rep=nam.get(req);
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
