#include "Logger.h"

#include <QDebug>

Logger::Logger(Presta *presta, QObject *parent) :
    QObject(parent),
    mPresta(presta)
{
    connect(presta, SIGNAL(error(PSWebService::PrestaError)), this, SLOT(logError(PSWebService::PrestaError)));
    connect(presta, SIGNAL(error(PSWebService::OtherError)), this, SLOT(logError(PSWebService::OtherError)));
}

void Logger::logError(PSWebService::PrestaError e)
{
    qDebug() << "NETWORK ERROR - KOD: " << e.code;
    for(int i=0; i<e.msgs.size(); ++i) {
        qDebug() << "PRESTA ERROR: [" << e.msgs.at(i).first << "] " << e.msgs.at(i).second;
    }
}

void Logger::logError(PSWebService::OtherError e)
{
    qDebug() << "NETWORK ERROR: [" << e.code << "] url: " << e.url.toString();
    qDebug() << e.msg;
}
