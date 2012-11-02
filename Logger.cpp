#include "Logger.h"

#include <QDebug>

Logger::Logger(KCPresta *presta, QObject *parent) :
    QObject(parent),
    mPresta(presta)
{
    connect(presta, SIGNAL(error(Exception)), this, SLOT(logError(Exception)));
}

void Logger::logError(const Exception &e)
{
    qDebug() << e.toString();
}
