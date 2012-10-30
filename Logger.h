#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include "KCPresta.h"

class Logger : public QObject
{
    Q_OBJECT
public:
    explicit Logger(KCPresta* presta, QObject *parent = 0);
    
signals:
    
public slots:
    void logError(PSWebService::PrestaError e);
    void logError(PSWebService::OtherError e);

protected:
    KCPresta* mPresta;
};

#endif // LOGGER_H
