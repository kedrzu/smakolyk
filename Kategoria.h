#ifndef KATEGORIA_H
#define KATEGORIA_H

#include <QString>
#include <QDomDocument>

/*!
 \brief

 \class Kategoria Kategoria.h "Kategoria.h"
*/
struct Kategoria {

    unsigned id; /*!< [EDYTUJ] Id grupy w Prestashop. */
    unsigned idKC; /*!< [DODAJ][EDYTUJ] Id grupy w KC-Firma. */
    QString nazwa; /*!< [DODAJ][EDYTUJ] Nazwa grupy */

    Kategoria() : id(0), idKC(0) {}
};

#endif // KATEGORIA_H
