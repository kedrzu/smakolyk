#include "KCFirma.h"
#include <QDebug>

KCFirma::KCFirma(Config &config, QObject *parent) :
    QObject(parent)
{
    //sciezki docelowo wczytywane z configa

    mKCFirmaPath = "c:\\KCFirma2\\Dane\\SMAKOLYK.dan";
    mKCFirmaDB = QSqlDatabase::addDatabase("QODBC", "KCFirma");
    mKCFirmaDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};"
                               "DBQ=" + mKCFirmaPath);
    mKCFirmaDB.open();

    //////////////////////////

    mKCPosPath   = "c:\\KCPos\\dane\\kasa.dan";
    mKCPosDB = QSqlDatabase::addDatabase("QODBC", "KCPos");
    mKCPosDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};"
                               "Dbq=" + mKCPosPath);
    mKCPosDB.open();

   //////////////////////////

    if( (!mKCPosDB.isOpen()) || (!mKCFirmaDB.isOpen()) )
        throw BLAD_POLACZENIA_Z_BAZA;

    qDebug() << "Blad bazy KCFirmy:\t" << mKCPosDB.lastError().text();
    qDebug() << "Blad bazy KCPosa:\t"  << mKCFirmaDB.lastError().text();



    //wczytawna z configa miejscie w klasyfikacji w ktorym jest zapisany status towaru(czy do sprzedazy?)
    //i generowanie odpowiedniego wzoru do zapytania SQL
    //przykładowa klasyfikacja 006000000007002000000000000000
    // przykladowy wzor:      "006???????????????????????????"
    mWzorzecKlasyfikacjiSprzedaz = mGenerujWzorzecKlasyfikacji(0, QString("006"));
    mWzorzecKlasyfikacjiKatalog  = mGenerujWzorzecKlasyfikacji(0, QString("015"));
    mWzorzecKlasyfikacjiWylacz   = mGenerujWzorzecKlasyfikacji(0, QString("015"));

    QMap<unsigned, Produkt>  a;
    produkty(a, 10);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
Kategoria KCFirma::kategoria(unsigned idKC) const {

    Kategoria kat;
    QSqlQuery kcFirmaGrupa (mKCFirmaDB);
    kcFirmaGrupa.exec("SELECT Nazwa, KodGrupy "
                      "FROM Grupy "
                      "WHERE KodGrupy='" + QString::number(idKC)+"'");


    qDebug()<< "Blad zapytania:\t" << kcFirmaGrupa.lastError().text();

    kcFirmaGrupa.next();
    kat.nazwa = kcFirmaGrupa.value(0).toString();
    kat.idKC = kcFirmaGrupa.value(1).toInt();

    if (!kat.idKC)
        throw BLAD_ZAPYTANIA_GRUPY;

    return kat;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

bool KCFirma::produkty(QMap<unsigned, Produkt> &produktyMapa, unsigned ilosc)
{
    //pozycje z kc posa : status (klasyfikacja), id (PrestaTowary.IDPresta), idKC(Kod)
    //cenaKC (ce_sb), kategriaKC ( KodGrupy), kategoria (PrestaKategorie.IDKategoriaPresta),
    // nazwa ( Nazwa), ean ( KodPaskowy)

    QSqlQuery kcTowaryPos (mKCPosDB);
    kcTowaryPos.exec("SELECT Towary.Kod, Towary.KodPaskowy, Towary.Nazwa, Towary.Klasyfikacja "
                     "FROM Towary "
                     "WHERE Towary.Kod < 100");
                     //"WHERE (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiSprzedaz + " )");


    /*
    kcTowaryPos.exec("SELECT Towary.Kod, Towary.KodPaskowy, Towary.Nazwa, Towary.Klasyfikacja, Towary.ce_sb, Towary.KodGrupy, "
                     "PrestaTowary.IDPresta, PrestaKategorie.IDKategoriaPresta "
                     "FROM (Towary "
                     "LEFT JOIN PrestaTowary "
                     "ON Towary.Kod = PrestaTowary.IDKC )"
                     "LEFT JOIN PrestaKategorie "
                     "ON Towary.KodGrupy = PrestaKategorie.IDGrupyKC "
                     "WHERE (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiSprzedaz + " )");
                     /*
                     "OR (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiKatalog + " ) "
                     "OR (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiKatalog + " )) " );
                 */

    qDebug() << "Blad zapytania:\t" << kcTowaryPos.lastError().text();

    while (kcTowaryPos.next()){
        qDebug() << kcTowaryPos.value(0).toString() << kcTowaryPos.value(2).toString();
    }

    //Pozycje z KC Firmy cenaPresta(ce_


    return true;
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

QString KCFirma::mGenerujWzorzecKlasyfikacji(unsigned pozycja, QString status){

    QString wzorzec;
    wzorzec = "'";
    for(unsigned i = 0; i < 10; i++){
        if (i == pozycja)
            wzorzec += status;
        else
            wzorzec += "???";
    }
    wzorzec += "'";

    return wzorzec;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
