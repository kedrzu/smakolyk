#include "PSWebService.h"
#include <QEventLoop>
#include <QtNetwork/QNetworkRequest>
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QDebug>

using namespace std;

PSWebService::PSWebService(QString url, QString key)
    : mUrl(url), mKey(key)
{
    mManager = new QNetworkAccessManager(this);

    connect(mManager, SIGNAL(authenticationRequired ( QNetworkReply *, QAuthenticator * )),
            this, SLOT(authentication( QNetworkReply *, QAuthenticator * )));
}

PSWebService::~PSWebService()
{
    delete mManager;
}

QNetworkReply *PSWebService::del(const Options &options)
{
    StackTraceBegin();
    if(options.resource.isEmpty() || options.id == 0) {
        Exception e;
        e.type = QString::fromUtf8("Błąd wywołania funkcji");
        e.msg = QString::fromUtf8("Nie podano parametrów ");
        if(options.resource.isEmpty())
            e.msg.append("'resource' ");
        if(options.id == 0)
            e.msg.append("'id' ");
        throw e;
    }
    QUrl url(mUrl + "/api/" + options.resource + "/" + QString::number(options.id));
    // tworzymy zapytanie DELETE
    return mManager->deleteResource( QNetworkRequest( url ) );
    StackTraceEnd("QNetworkReply *PSWebService::del(const Options &options)");
}

QNetworkReply *PSWebService::get(const Options &options)
{
    StackTraceBegin();
    if(options.resource.isEmpty()) {
        Exception e;
        e.type = QString::fromUtf8("Błąd wywołania funkcji");
        e.msg = QString::fromUtf8("Nie podano parametru 'resource'");
        throw e;
    }

    QUrl url;
    if(options.id > 0) {
        url = QUrl(mUrl + "/api/" + options.resource + "/" + QString::number(options.id));
    } else {
        url = QUrl(mUrl + "/api/" + options.resource);
    }
    QMapIterator<QString, QString> it(options.filter);
    while(it.hasNext()) {
        it.next();
        url.addQueryItem( QString("filter[").append(it.key()).append("]"), it.value() );
    }
    if(!options.display.isEmpty()) {
        url.addQueryItem( "display", options.display);
    }
    if(!options.sort.isEmpty()) {
        url.addQueryItem( "sort", options.sort);
    }
    // tworzymy zapytanie GET
    return mManager->get( QNetworkRequest( url ) );
    //}
    StackTraceEnd("QNetworkReply *PSWebService::get(const Options &options)");
}

QDomDocument PSWebService::syncDel(const Options &options)
{
    StackTraceBegin();
    return readReply(syncReply(del(options)));
    StackTraceEnd("QDomDocument PSWebService::syncDel(const Options &options)");
}

QDomDocument PSWebService::syncGet(const Options &options)
{
    StackTraceBegin();
    return readReply(syncReply(get(options)));
    StackTraceEnd("QDomDocument PSWebService::syncGet(const Options &options)");
}

QNetworkReply *PSWebService::post(const Options &options, const QDomDocument &xml)
{
    StackTraceBegin();
    QNetworkRequest req(QUrl(mUrl + "/api/" + options.resource));
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    QUrl params;
    params.addQueryItem("xml", xml.toString());
    return mManager->post(req, params.encodedQuery());
    StackTraceEnd("QNetworkReply *PSWebService::post(const Options &options, const QDomDocument &xml)");
}

QDomDocument PSWebService::syncPost(const Options& options, const QDomDocument &xml)
{
    StackTraceBegin();
    return readReply(syncReply(post(options, xml)));
    StackTraceEnd("QDomDocument PSWebService::syncPost(const Options& options, const QDomDocument &xml)");
}

QNetworkReply *PSWebService::put(const Options &options, const QDomDocument &xml)
{
    StackTraceBegin();
    if(options.resource.isEmpty() || options.id == 0) {
        Exception e;
        e.type = QString::fromUtf8("Błąd wywołania funkcji");
        e.msg = QString::fromUtf8("Nie podano parametrów ");
        if(options.resource.isEmpty())
            e.msg.append("'resource' ");
        if(options.id == 0)
            e.msg.append("'id' ");
        throw e;
    }
    QUrl url(mUrl + "/api/" + options.resource + "/" + QString::number(options.id));

    return mManager->put(QNetworkRequest(url), xml.toByteArray());
    StackTraceEnd("QNetworkReply *PSWebService::put(const Options &options, const QDomDocument &xml)");
}

QDomDocument PSWebService::syncPut(const Options& options, const QDomDocument &xml)
{
    StackTraceBegin();
    return readReply(syncReply(put(options, xml)));
    StackTraceEnd("QDomDocument PSWebService::syncPut(const Options& options, const QDomDocument &xml)");
}

void PSWebService::authentication(QNetworkReply *reply, QAuthenticator *authenticator)
{
    authenticator->setUser(mKey);
}

QDomDocument PSWebService::readReply(QNetworkReply *reply)
{
    StackTraceBegin();
    QByteArray read = reply->readAll();
    QNetworkReply::NetworkError error = reply->error();
    reply->deleteLater();
    QDomDocument doc;
    bool parsed = doc.setContent(read);
    // pomyślnie odebrano XML
    if(error == QNetworkReply::NoError && (read.isEmpty() || parsed)) {
        return doc;
    }
    // wystąpił błąd i zwrócony został komunikat błędu w XML
    else if(parsed) {
        PrestaError exception;
        exception.type = QString::fromUtf8("Błąd Prestashop");
        exception.code = error;
        exception.url = reply->url();
        QDomElement prestashop = doc.firstChildElement("prestashop");
        QDomElement errorsElem = prestashop.firstChildElement("errors");
        QDomNodeList errorList = errorsElem.elementsByTagName("error");
        for(int i=0; i<errorList.size(); ++i) {
            QPair<unsigned, QString> pair;
            pair.first = errorList.at(i).firstChildElement("code").firstChild().toCDATASection().nodeValue().toInt();
            pair.second = errorList.at(i).firstChildElement("message").firstChild().toCDATASection().nodeValue();
            exception.prestaMsgs << pair;
        }
        throw exception;
    }
    // nie udało się sparsować odpowiedzi XML
    else {
        NetworkError exception;
        exception.type = QString::fromUtf8("Nie udało się sparsować odpowiedzi XML");
        exception.msg = QString(read);
        exception.code = error;
        exception.url = reply->url();
        throw exception;
    }
    StackTraceEnd("QDomDocument PSWebService::readReply(QNetworkReply *reply)");
}

QNetworkReply *PSWebService::syncReply(QNetworkReply *reply)
{
    StackTraceBegin();
    QEventLoop loop;
    if(!reply->isFinished()) {
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
    }
    return reply;
    StackTraceEnd("QNetworkReply *PSWebService::syncReply(QNetworkReply *reply)");
}

QString PSWebService::NetworkError::toHtml() const
{
    return headerToHtml().append(networkErrorToHtml()).append(stackToHtml());
}

QString PSWebService::NetworkError::toString() const
{
    return headerToString().append(networkErrorToString()).append(stackToString());
}

QString PSWebService::NetworkError::networkErrorToHtml() const
{
    QString html = QString::fromUtf8("<table cellspacing='5'><tr><td colspan=2><b><u>Błąd sieciowy</u></b></td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>Kod błędu:</b></td><td>")+QString::number(code)+"</td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>URL:</b></td><td>")+url.toString()+"</td></tr>");
    html.append("</table>\n");
    return html;
}

QString PSWebService::NetworkError::networkErrorToString()const
{
    QString str = QString::fromUtf8("\tBłąd sieciowy:\n");
    str.append(QString::fromUtf8("Kod błędu:\t")+QString::number(code)+"\n");
    str.append(QString::fromUtf8("URL:\t")+url.toString()+"\n");
    return str;
}

QString PSWebService::PrestaError::toHtml() const
{
    QString html = headerToHtml().append(networkErrorToHtml());
    html.append(QString::fromUtf8("<table cellspacing='5'><tr><td colspan=2><b><u>Błędy Prestashop</u></b></td></tr>"));
    for(int i=0; i<prestaMsgs.size(); ++i) {
        html.append(QString::fromUtf8("<tr><td><b>Kod:</b></td><td>")+QString::number(prestaMsgs.at(i).first)+"</td></tr>");
        html.append(QString::fromUtf8("<tr><td><b>Komunikat:</b></td><td>")+prestaMsgs.at(i).second+"</td></tr>");
    }
    html.append("</table>\n");
    html.append(stackToHtml());
    return html;
}

QString PSWebService::PrestaError::toString() const
{
    QString str = headerToString().append(networkErrorToString());
    str.append(QString::fromUtf8("\tBłędy Prestashop: \n"));
    for(int i=0; i<prestaMsgs.size(); ++i) {
        str.append(QString::fromUtf8("Kod:\t")+QString::number(prestaMsgs.at(i).first)+"\n");
        str.append(QString::fromUtf8("Komunikat:\t")+prestaMsgs.at(i).second+"\n");
    }
    str.append(stackToString());
    return str;
}
