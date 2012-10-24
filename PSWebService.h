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

class PSWebService : public QObject
{
    Q_OBJECT

public:
    struct Options {
        std::string resource;
        int id;
        QMap<std::string, std::string> filter;
        std::string display;
        std::string sort;
        int limit;
        Options() : id(-1), limit(-1) {}
    };

    struct PrestaError {
        QNetworkReply::NetworkError code;
        QVector<QPair<unsigned, QString> > msgs;
        PrestaError(const QNetworkReply::NetworkError& code)
            : code(code) {}
        PrestaError() {}
    };

    struct OtherError {
        QNetworkReply::NetworkError code;
        QString msg;
        OtherError(QNetworkReply::NetworkError code, QString msg) :
            code(code), msg(msg) {}
    };

    PSWebService(std::string url, std::string key);
    ~PSWebService();
    QNetworkReply *get(const Options& options);
    QNetworkReply *post(const Options &options, const QDomDocument& xml);
    QNetworkReply *put(const Options &options, const QDomDocument& xml);
    QDomDocument *syncGet(const Options& options);
    QDomDocument *syncPost(const Options &options, const QDomDocument& xml);
    QDomDocument *syncPut(const Options & options, const QDomDocument& xml);
    static QDomDocument *readReply(QNetworkReply* reply);
    static QNetworkReply *syncReply(QNetworkReply* reply);

public slots:
    void authentication(QNetworkReply * reply, QAuthenticator * authenticator);

protected:
    QString mUrl;
    QString mKey;
    QNetworkAccessManager *mManager;
};

#endif // PSWEBSERVICE_H
