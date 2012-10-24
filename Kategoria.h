#ifndef KATEGORIA_H
#define KATEGORIA_H

#include <QString>
#include <QDomDocument>

/*!
 \brief

 \class Kategoria Kategoria.h "Kategoria.h"
*/
struct Kategoria {
    /*!
     \brief

     \enum Status
    */
    enum Status {
        NIEAKTYWNA = 0,
        AKTYWNA = 1
    };

    Status status; /*!< [DODAJ][EDYTUJ] Czy grupa jest aktywna, czy nie. */
    unsigned id; /*!< [EDYTUJ] Id grupy w Prestashop. */
    unsigned idKC; /*!< [DODAJ][EDYTUJ] Id grupy w KC-Firma. */
    unsigned nadrzedna; /*!< Id w Presta grupy nadrz�dnej. Zostaw domy�lne. */
    QString nazwa; /*!< [DODAJ][EDYTUJ] Nazwa grupy */
    QString metaOpis; /*!< Opis grupy do wyszukiwarki. Olej to. */
    QString metaKeywords; /*!< S�owa kluczowe. Olej to. */
    QString metaTytul; /*!< [DODAJ][EDYTUJ] Tytu� do wyszukiwarki. Mo�esz ustawi� taki sam jak nazwa. */
    QString przyjaznyUrl; /*!< [DODAJ][EDYTUJ] Nazw� przer�b na ma�e litery i zamiast spacji daj my�lniki. */
    QString opis; /*!< Olej. */

    static unsigned getId(QDomDocument* doc);
/*!
 \brief

 \fn Kategoria
*/
    Kategoria() : id(0), idKC(0), nadrzedna(1) {}
};

#endif // KATEGORIA_H
