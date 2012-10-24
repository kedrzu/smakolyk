#include "KCFirma.h"
#include <QDebug>

KCFirma::KCFirma(Config & config, QObject *parent) :
    QObject(parent)
{
}

Kategoria KCFirma::kategoria(unsigned idKC) const {
     Kategoria kat;
     return kat;
 }

void KCFirma::zmianaKategorii(unsigned id, unsigned idKC) {
    qDebug() << "zmiana kategorii \tid=" << id << "\tidKC=" << idKC;
}

void KCFirma::zmianaProduktu(unsigned id, unsigned idKC, float cena) {
    qDebug() << "zmiana produktu \tid=" << id << "\tidKC=" << idKC << "\tcena=" << cena;
}
