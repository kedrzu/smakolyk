#include "KCFirma.h"
#include <QDebug>

KCFirma::KCFirma(Config &config, QObject *parent) :
    QObject(parent)
{

    mKCFirmaPath = "C:\\KCFirma2\\Dane\\SMAKOLYK.dan";
    mKCPosPath   = "C:\\KCPos\\dane\\kasa.dan";

    mKCFirmaDB = QSqlDatabase::addDatabase("QODBC");
    mKCFirmaDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};"
                               "Dbq=" + mKCFirmaPath);

    mKCPosDB = QSqlDatabase::addDatabase("QODBC");
    mKCPosDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};"
                               "Dbq=" + mKCPosPath);

    if( (!mKCPosDB.isOpen()) || (!mKCFirmaDB.isOpen()) )
        throw BLAD_POLACZENIA_Z_BAZA;



}

//////////////////////////////////////////////////////////////////////////////////////////////////////
Kategoria KCFirma::kategoria(unsigned idKC) const {

    Kategoria kat;
    QSqlQuery KCFirmaGrupa (mKCFirmaDB);
    KCFirmaGrupa.exec("SELECT Nazwa, KodGrupy, Status "
                      "FROM Grupy"
                      "WHERE KodGrupy=" + QString::number(idKC));

    if (KCFirmaGrupa.size() != 1)
        throw BLAD_ZAPYTANIA_GRUPY;






     return kat;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void KCFirma::zmianaKategorii(unsigned id, unsigned idKC) {
    qDebug() << "zmiana kategorii \tid=" << id << "\tidKC=" << idKC;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void KCFirma::zmianaProduktu(unsigned id, unsigned idKC, float cena) {
    qDebug() << "zmiana produktu \tid=" << id << "\tidKC=" << idKC << "\tcena=" << cena;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

bool KCFirma::produkty(QMap<unsigned, Produkt> &produktyMapa, unsigned ilosc)
{
    return true;
}
