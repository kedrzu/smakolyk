#include "Logger.h"

#include <QDebug>

Logger::Logger(Presta *presta, QObject *parent) :
    QObject(parent),
    mPresta(presta)
{
    connect(presta, SIGNAL(error(QNetworkReply::NetworkError,QByteArray)), this, SLOT(logError(QNetworkReply::NetworkError,QByteArray)));
}

void Logger::logError(QNetworkReply::NetworkError code, QByteArray msg) {
    qDebug() << "ERROR: " << code << " " << msg;
}
