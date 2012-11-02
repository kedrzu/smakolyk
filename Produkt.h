#ifndef PRODUKT_H
#define PRODUKT_H

#include <QString>
#include <QDomDocument>
#include <QList>
#include <QDebug>

/*!
 \brief

 \class Produkt Produkt.h "Produkt.h"
*/
struct Produkt {
    /*!
     \brief

     \enum Status
    */
    enum Status {
        USUN = 0,
        WYLACZ = 1,
        KATALOG = 2,
        SPRZEDAZ = 3
    };
    Status status; /*!< [DODAJ][EDYTUJ] Czy produkt jest aktywny, czy nie. */
    uint id; /*!< [EDYTUJ] Id produktu w Prestashop. */
    uint idKC; /*!< [DODAJ][EDYTUJ] Id produktu w KC-Firma. */
    float cenaPresta; /*!< [DODAJ][EDYTUJ] */
    float cenaKC; /*!< [DODAJ][EDYTUJ] */
    uint kategoria; /*!< [EDYTUJ] Numer grupy do sprzedaży w Prestashop  */
    uint kategoriaKatalog;  /*!< Numer grupy katalogowej w Presta*/
    uint kategoriaKC; /*!< [DODAJ][EDYTUJ] Numer grupy w KC-Firma */
    QString nazwa; /*!< [DODAJ][EDYTUJ] Nazwa produktu */
    QString ean; /*!< [DODAJ][EDYTUJ] Kod kreskowy */
    QString opis; /*!< Olej to. */
    QString krotkiOpis; /*!< [DODAJ][EDYTUJ] Wpisz tutaj właściwości typu gramatura itp. */

    void czytaj();
/*!
 \brief

 \fn Produkt
*/
    Produkt() : status(USUN), id(0), idKC(0), cenaPresta(0), kategoria(0), kategoriaKC(0) {}
};

#endif // PRODUKT_H
