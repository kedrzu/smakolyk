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
    void logError(const Exception& e);

protected:
    KCPresta* mPresta;
};

#endif // LOGGER_H
