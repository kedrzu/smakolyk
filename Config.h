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
    QString key; /*!< klucz dost�pu do API Presta */
    QString url; /*!< adres URL sklepu */
    uint lang; /*!< numer u�ywanego j�zyka w Presta (polskiego najlepiej :P) */

signals:
    
public slots:
    
};

#endif // CONFIG_H
