#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>

/*!
 \brief

 \class Config Config.h "Config.h"
*/
class Config : public QObject
{
    Q_OBJECT
public:
    /*!
     \brief

     \fn Config
     \param parent
    */
    explicit Config(QObject *parent = 0);
    QString key; /*!< klucz dostêpu do API Presta */
    QString url; /*!< adres URL sklepu */
    uint lang; /*!< numer u¿ywanego jêzyka w Presta (polskiego najlepiej :P) */

signals:
    
public slots:
    
};

#endif // CONFIG_H
