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

QNetworkReply *PSWebService::get(const Options &options)
{
    //if(options.resource.empty()) {
    //    throw std::runtime_error("Podano złe parametry.");
    //} else {
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
}

QDomDocument PSWebService::syncGet(const Options &options)
{
    return readReply(syncReply(get(options)));
}

QNetworkReply *PSWebService::post(const Options &options, const QDomDocument &xml)
{
    QNetworkRequest req(QUrl(mUrl + "/api/" + options.resource));
    req.setHeader(QNetworkRequest::ContentTypeHeader,QVariant("application/x-www-form-urlencoded"));
    QUrl params;
    params.addQueryItem("xml", xml.toString());
    return mManager->post(req, params.encodedQuery());
}

QDomDocument PSWebService::syncPost(const Options& options, const QDomDocument &xml)
{
    return readReply(syncReply(post(options, xml)));
}

QNetworkReply *PSWebService::put(const Options &options, const QDomDocument &xml)
{
    //if(options.resource.empty() || options.id < 0) {
    //    throw std::runtime_error("Podano złe parametry.");
    //} else {
    QUrl url(mUrl + "/api/" + options.resource + "/" + QString::number(options.id));

    return mManager->put(QNetworkRequest(url), xml.toByteArray());
    //}
}

QDomDocument PSWebService::syncPut(const Options& options, const QDomDocument &xml)
{
    return readReply(syncReply(put(options, xml)));
}

void PSWebService::authentication(QNetworkReply *reply, QAuthenticator *authenticator)
{
    authenticator->setUser(mKey);
}

QDomDocument PSWebService::readReply(QNetworkReply *reply) {
    QByteArray read = reply->readAll();
    QNetworkReply::NetworkError error = reply->error();
    reply->deleteLater();
    QDomDocument doc;
    bool parsed = doc.setContent(read);
    // pomyślnie odebrano XML
    if(error == QNetworkReply::NoError && parsed) {
        return doc;
    }
    // wystąpił błąd i zwrócony został komunikat błędu w XML
    else if(parsed) {
        PrestaError exception;
        StackTrace(exception, "QDomDocument PSWebService::readReply(QNetworkReply *reply)");
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
        OtherError exception;
        StackTrace(exception, "QDomDocument PSWebService::readReply(QNetworkReply *reply)");
        exception.code = error;
        exception.url = reply->url();
        exception.httpResponse = QString(read);
        throw exception;
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

QString PSWebService::OtherError::toHtml()
{
    QString html = "<table cellspacing='5'>";
    html.append(QString::fromUtf8("<tr><td><b>Wiadomość: </b></td><td>")+msg+"</td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>Kod błędu sieciowego:</b></td><td>")+QString::number(code)+"</td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>URL:</b></td><td>")+url.toString()+"</td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>Odpowiedź HTTP: </b></td><td>")+httpResponse+"</td></tr>");
    html.append("</table>\n");
    html.append(stackToHtml());
    return html;
}

QString PSWebService::OtherError::toString()
{
    QString str;
    str.append(QString::fromUtf8("Wiadomość:\t")+msg+"\n");
    str.append(QString::fromUtf8("Kod błędu sieciowego:\t")+QString::number(code)+"\n");
    str.append(QString::fromUtf8("URL:\t")+url.toString()+"\n");
    str.append(QString::fromUtf8("Odpowiedź HTTP: \n")+httpResponse + "\n");
    str.append(stackToString());
    return str;
}

QString PSWebService::PrestaError::toHtml()
{
    QString html = "<table cellspacing='5'>";
    html.append(QString::fromUtf8("<tr><td><b>Wiadomość: </b></td><td>")+msg+"</td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>Kod błędu sieciowego:</b></td><td>")+QString::number(code)+"</td></tr>");
    html.append(QString::fromUtf8("<tr><td><b>URL:</b></td><td>")+url.toString()+"</td></tr>");
    html.append("</table>\n\n");
    html.append("<table cellspacing='5'>");
    html.append(QString::fromUtf8("<tr><td colspan=2>Błędy Prestashop</td></tr>"));
    for(int i=0; i<prestaMsgs.size(); ++i) {
        html.append(QString::fromUtf8("<tr><td><b>Kod:</b></td><td>")+QString::number(prestaMsgs.at(i).first)+"</td></tr>");
        html.append(QString::fromUtf8("<tr><td><b>Komunikat:</b></td><td>")+prestaMsgs.at(i).second+"</td></tr>");
    }
    html.append("</table>\n");
    html.append(stackToHtml());
    return html;
}

QString PSWebService::PrestaError::toString()
{
    QString str;
    str.append(QString::fromUtf8("Wiadomość:\t")+msg+"\n");
    str.append(QString::fromUtf8("Kod błędu sieciowego:\t")+QString::number(code)+"\n");
    str.append(QString::fromUtf8("URL:\t")+url.toString()+"\n\n");
    str.append(QString::fromUtf8("Błędy Prestashop: \n"));
    for(int i=0; i<prestaMsgs.size(); ++i) {
        str.append(QString::fromUtf8("Kod:\t")+QString::number(prestaMsgs.at(i).first)+"\n");
        str.append(QString::fromUtf8("Komunikat:\t")+prestaMsgs.at(i).second+"\n");
    }

    str.append(stackToString());
    return str;
}
