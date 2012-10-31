#ifndef PRESTA_H
#define PRESTA_H

#include <QObject>
#include "PSWebService.h"
#include <QVector>
#include <QMap>
#include "Kategoria.h"
#include "Produkt.h"
#include "Zamowienie.h"
#include "PrestaStructs.h"

/*!
 \brief

 \class Presta Presta.h "Presta.h"
*/
namespace Presta {
class Prestashop : public QObject
{
    Q_OBJECT
public:
    Prestashop(const Config &config, PSWebService* pswebService, QObject *parent = 0);
    QNetworkReply *add(const Product& product);
    QNetworkReply *add(const Category& kategoria);
    QNetworkReply *add(const SpecificPrice& specificPrice);
    QNetworkReply *edit(const Product& product);
    QNetworkReply *edit(const Category& kategoria);
    QNetworkReply *edit(const SpecificPrice& specificPrice);
     /*!
     \brief Przeksztalca XML do zamowienia.

     \param doc
     \return Zamowienie
    */
    Order getOrder(QDomDocument& doc) const;
    /*!
     \brief Pobiera z Presta zamowienie o podanym ID.

     \param id
     \return Zamowienie
    */
    Order getOrder(uint id);
    /*!
     \brief Pobiera z presta zamowienia o podanym stanie.

     \param status
     \return QList<Order>
    */
    QList<Order> getOrders(uint state);
    QList<uint> getSpecificPrice(uint productId);
    /*!
     \brief Synchroniczne dodawanie produktu w Presta.

     \param produkt
     \return unsigned
    */
    unsigned syncAdd(const Product &product);
    /*!
     \brief Synchroniczne dodawanie kategorii.

     \param kategoria
     \return unsigned
    */
    unsigned syncAdd(const Category& kategoria);
    /*!
     \brief Synchroniczne dodawanie ceny specjalnej

     \param specificPrice
     \return unsigned
    */
    void syncAdd(const SpecificPrice& specificPrice);
    /*!
     \brief Synchroniczna edycja produktu w Presta

     \param produkt
    */
    void syncEdit(const Product& product);
    /*!
     \brief Synchroniczna edycja kategorii.

     \param kategoria
    */
    void syncEdit(const Category& kategoria);
    /*!
     \brief Synchroniczna edycja zamowienia.

     \param zamowienie
    */
    void syncEdit(const Order& zamowienie);
    /*!
     \brief Synchroniczna edycja ceny specjalnej.

     \param specificPrice
    */
    void syncEdit(const SpecificPrice& specificPrice);
    /*!
     \brief Zamienia produkt na postac XML

     \param produkt
     \return QDomDocument
    */
    QDomDocument toXML(const Product& product);
    /*!
     \brief Zamienia kategorie na postac XML

     \param kategoria
     \return QDomDocument
    */
    QDomDocument toXML(const Category& category);
    /*!
     \brief Zamienia zamowienie na postac XML

     \param zamowienie
     \return QDomDocument
    */
    QDomDocument toXML(const Order& order);
    /*!
     \brief Zamienia cene specjalna na postac XML

     \param specificPrice
     \return QDomDocument
    */
    QDomDocument toXML(const SpecificPrice& specificPrice);
    
protected:
    static QDomElement buildXMLElement(QDomDocument &doc, const QString &name, const QString &value);
    static QDomElement buildXMLElement(QDomDocument &doc, const QString &name, const QString &value, int lang);
    static QDomDocument getPrestaXML();

    PSWebService *mPSWebService;
    unsigned mLangId;
};
}

#endif // PRESTA_H
