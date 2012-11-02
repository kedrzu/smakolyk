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
#include "Exception.h"

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

    struct NetworkError : public Exception {
        QNetworkReply::NetworkError code;
        QUrl url;
        virtual QString toHtml() const;
        virtual QString toString() const;
        QString networkErrorToHtml() const;
        QString networkErrorToString() const;
    };

    struct PrestaError : public NetworkError {
        QVector<QPair<unsigned, QString> > prestaMsgs;
        virtual QString toHtml() const;
        virtual QString toString() const;
    };

    PSWebService(QString url, QString key);
    ~PSWebService();
    QNetworkReply *del(const Options& options);
    QNetworkReply *get(const Options& options);
    QNetworkReply *post(const Options &options, const QDomDocument& xml);
    QNetworkReply *put(const Options &options, const QDomDocument& xml);
    QDomDocument syncDel(const Options& options);
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
