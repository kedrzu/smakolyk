#include "PSWebService.h"
#include <QEventLoop>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QDebug>

using namespace std;

PSWebService::PSWebService(std::string url, std::string key)
    : mUrl(url.c_str()), mKey(key.c_str())
{
    mManager = new QNetworkAccessManager(this);

    connect(mManager, SIGNAL(authenticationRequired ( QNetworkReply *, QAuthenticator * )),
            this, SLOT(authentication( QNetworkReply *, QAuthenticator * )));
}

PSWebService::~PSWebService()
{
    delete mManager;
}

QNetworkReply *PSWebService::get(const Options &options)
{
    //if(options.resource.empty()) {
    //    throw std::runtime_error("Podano z³e parametry.");
    //} else {
    QUrl url;
    if(options.id > 0) {
        url = QUrl(mUrl + "/api/" + options.resource.c_str() + "/" + QString::number(options.id));
    } else {
        url = QUrl(mUrl + "/api/" + options.resource.c_str());
    }
    QMapIterator<std::string, std::string> it(options.filter);
    while(it.hasNext()) {
        it.next();
        url.addQueryItem( QString("filter[").append(it.key().c_str()).append("]"), QString(it.value().c_str() ) );
    }
    if(!options.display.empty()) {
        url.addQueryItem( "display", QString(options.display.c_str() ));
    }
    if(!options.sort.empty()) {
        url.addQueryItem( "sort", QString(options.sort.c_str() ));
    }
    // tworzymy zapytanie GET
    return mManager->get( QNetworkRequest( url ) );
    //}
}

QDomDocument *PSWebService::syncGet(const Options &options)
{
    return readReply(syncReply(get(options)));
}

QNetworkReply *PSWebService::post(const Options &options, const QDomDocument &xml)
{
    QNetworkRequest req(QUrl(mUrl + "/api/" + options.resource.c_str()));
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    QUrl params;
    params.addQueryItem("xml", xml.toString());
    return mManager->post(req, params.encodedQuery());
}

QDomDocument* PSWebService::syncPost(const Options& options, const QDomDocument &xml)
{
    return readReply(syncReply(post(options, xml)));
}

QNetworkReply *PSWebService::put(const Options &options, const QDomDocument &xml)
{
    //if(options.resource.empty() || options.id < 0) {
    //    throw std::runtime_error("Podano z³e parametry.");
    //} else {
    QUrl url(mUrl + "/api/" + options.resource.c_str() + "/" + QString::number(options.id));

    return mManager->put(QNetworkRequest(url), xml.toByteArray());
    //}
}

QDomDocument* PSWebService::syncPut(const Options& options, const QDomDocument &xml)
{
    return readReply(syncReply(put(options, xml)));
}

void PSWebService::authentication(QNetworkReply *reply, QAuthenticator *authenticator)
{
    authenticator->setUser(mKey);
}

QDomDocument *PSWebService::readReply(QNetworkReply *reply) {
    QByteArray read = reply->readAll();
    QNetworkReply::NetworkError error = reply->error();
    reply->deleteLater();
    QDomDocument* doc = new QDomDocument();
    bool parsed = doc->setContent(read);
    // pomyœlnie odebrano XML
    if(error == QNetworkReply::NoError && parsed) {
        return doc;
    }
    // wyst¹pi³ b³¹d i zwrócony zosta³ komunikat b³êdu w XML
    else if(parsed) {
        PrestaError exception(error);
        QDomElement prestashop = doc->firstChildElement("prestashop");
        QDomElement errorsElem = prestashop.firstChildElement("errors");
        QDomNodeList errorList = errorsElem.elementsByTagName("error");
        for(int i=0; i<errorList.size(); ++i) {
            QPair<unsigned, QString> pair;
            pair.first = errorList.at(i).firstChildElement("code").firstChild().toCDATASection().nodeValue().toInt();
            pair.second = errorList.at(i).firstChildElement("message").firstChild().toCDATASection().nodeValue();
        }
        delete doc;
        throw exception;
    }
    // nie uda³o siê sparsowaæ odpowiedzi XML
    else {
        delete doc;
        throw OtherError(error, "Wyst¹pi³ b³¹d parsowania odpowiedzi XML [url:"+reply->url().toString()+"]");
    }
}

QNetworkReply *PSWebService::syncReply(QNetworkReply *reply) {
    QEventLoop loop;
    if(!reply->isFinished()) {
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }
    return reply;
}
