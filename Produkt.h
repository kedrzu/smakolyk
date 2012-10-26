#ifndef PRODUKT_H
#define PRODUKT_H

#include <QString>
#include <QDomDocument>

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
    float cena; /*!< [DODAJ][EDYTUJ] */
    uint kategoria; /*!< [EDYTUJ] Numer grupy w Prestashop  */
    uint kategoriaKC; /*!< [DODAJ][EDYTUJ] Numer grupy w KC-Firma */
    QString nazwa; /*!< [DODAJ][EDYTUJ] Nazwa produktu */
    uint outOfStock; /*!< Zostaw domy�lne */
    uint ilosc; /*!< Zostaw domy�lne */
    QString metaOpis; /*!< Opis grupy do wyszukiwarki. Olej to. */
    QString metaKeywords; /*!< S�owa kluczowe. Olej to. */
    QString metaTytul; /*!< [DODAJ][EDYTUJ] Tytu� do wyszukiwarki. Mo�esz ustawi� taki sam jak nazwa. */
    QString przyjaznyUrl; /*!< [DODAJ][EDYTUJ] Nazw� przer�b na ma�e litery i zamiast spacji daj my�lniki. */
    QString dostepnyTeraz; /*!< Olej. */
    QString dostepnyPozniej; /*!< Olej to. */
    QString opis; /*!< Olej to. */
    QString krotkiOpis; /*!< [DODAJ][EDYTUJ] Wpisz tutaj w�a�ciwo�ci typu gramatura itp. */
    QString ean; /*!< [DODAJ][EDYTUJ] Kod kreskowy */

    static unsigned getId(QDomDocument &doc);
    static float getCena(QDomDocument &doc);

/*!
 \brief

 \fn Produkt
*/
    Produkt() : status(USUN), id(0), idKC(0), cena(0), outOfStock(2), ilosc(0), kategoria(0), kategoriaKC(0) {}
};

#endif // PRODUKT_H
