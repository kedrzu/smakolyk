#ifndef KATEGORIA_H
#define KATEGORIA_H

#include <QString>
#include <QDomDocument>

/*!
 \brief

 \class Kategoria Kategoria.h "Kategoria.h"
*/
struct Kategoria {

    uint id; /*!< [EDYTUJ] Id grupy do sprzedaży w Prestashop. */
    uint idKatalog;  /*!< Id grupy katalogowej */
    uint idKC; /*!< [DODAJ][EDYTUJ] Id grupy w KC-Firma. */
    QString nazwa; /*!< [DODAJ][EDYTUJ] Nazwa grupy */

    Kategoria() : id(0), idKC(0) {}
};

#endif // KATEGORIA_H
