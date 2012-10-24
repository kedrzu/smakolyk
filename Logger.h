#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include "Presta.h"

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(Presta* presta, QObject *parent = 0);
    
signals:
    
public slots:
    void logError(QNetworkReply::NetworkError code, QByteArray msg);

protected:
    Presta* mPresta;
};

#endif // LOGGER_H
