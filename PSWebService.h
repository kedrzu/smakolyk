#ifndef PSWEBSERVICE_H
#define PSWEBSERVICE_H

#include <string>
#include <QtNetwork/QNetworkAccessManager>
#include <QObject>
#include <QtNetwork/QAuthenticator>
#include <QString>
#include <QUrl>
#include <QtNetwork/QNetworkReply>
#include <QtXml>
#include <QDomDocument>
#include <QVector>
#include <QMap>
#include "Config.h"

class PSWebService : public QObject
{
    Q_OBJECT

public:
    struct Options {
        QString resource;
        int id;
        QMap<QString, QString> filter;
        QString display;
        QString sort;
        int limit;
        Options() : id(-1), limit(-1) {}
    };

    struct PrestaError {
        QNetworkReply::NetworkError code;
        QUrl url;
        QVector<QPair<unsigned, QString> > msgs;
        QString msg;
        PrestaError(const QNetworkReply::NetworkError& code)
            : code(code) {}
        PrestaError() {}
    };

    struct OtherError {
        QNetworkReply::NetworkError code;
        QUrl url;
        QString msg;
        OtherError(QNetworkReply::NetworkError code, const QUrl& url, QString msg) :
            code(code), url(url), msg(msg) {}
        OtherError() {}
    };

    PSWebService(const Config &config);
    ~PSWebService();
    QNetworkReply *get(const Options& options);
    QNetworkReply *post(const Options &options, const QDomDocument& xml);
    QNetworkReply *put(const Options &options, const QDomDocument& xml);
    QDomDocument syncGet(const Options& options);
    QDomDocument syncPost(const Options &options, const QDomDocument& xml);
    QDomDocument syncPut(const Options & options, const QDomDocument& xml);
    static QDomDocument readReply(QNetworkReply* reply);
    static QNetworkReply *syncReply(QNetworkReply* reply);

public slots:
    void authentication(QNetworkReply * reply, QAuthenticator * authenticator);

protected:
    QString mUrl;
    QString mKey;
    QNetworkAccessManager *mManager;
};

#endif // PSWEBSERVICE_H
