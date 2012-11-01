#include "KCFirma.h"
#include <QDebug>

KCFirma::KCFirma(QSettings settings, QObject *parent) :
    QObject(parent)
{
    //sciezki docelowo wczytywane z configa
    mKCFirmaPath = settings.value("BazaKCFirmy", "c:\\KCFirma2\\Dane\\SMAKOLYK.dan");
    mKCFirmaDB = QSqlDatabase::addDatabase("QODBC", "KCFirma");
    mKCFirmaDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};"
                               "DBQ=" + mKCFirmaPath);
    mKCFirmaDB.open();

    //////////////////////////

    mKCPosPath   = settings.value("BazaKCPos", "c:\\KCPos\\dane\\kasa.dan");
    mKCPosDB = QSqlDatabase::addDatabase("QODBC", "KCPos");
    mKCPosDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};"
                               "Dbq=" + mKCPosPath);
    mKCPosDB.open();

   //////////////////////////

    if( (!mKCPosDB.isOpen()) || (!mKCFirmaDB.isOpen()) )
        throw BLAD_POLACZENIA_Z_BAZA;

    //wczytawna z configa miejscie w klasyfikacji w ktorym jest zapisany status towaru(czy do sprzedazy?)
    //i generowanie odpowiedniego wzoru do zapytania SQL
    //przykładowa klasyfikacja 006000000007002000000000000000
    // przykladowy wzor:      "006???????????????????????????"

    QString nrKlasyfikacjiSprzedaz = settings.value("nrKlasyfikacjiSprzedaz", QString::number(0));
    QString nrKlasyfikacjiKatalog  = settings.value("nrKlasyfikacjiKatalog",  QString::number(0));
    QString nrKlasyfikacjiWylacz   = settings.value("nrKlasyfikacjiWylacz"  , QString::number(0));
    QString wzorKlasyfikacjiSprzedaz = settings.value("wzorKlasyfikacjiSprzedaz", QString("002"));
    QString wzorKlasyfikacjiKatalog  = settings.value("wzorKlasyfikacjiKatalog" , QString("001"));
    QString wzorKlasyfikacjiWylacz   = settings.value("wzorKlasyfikacjiWylacz"  , QString("000"));

    mWzorzecKlasyfikacjiSprzedaz = mGenerujWzorzecKlasyfikacji(nrKlasyfikacjiSprzedaz.toInt(),wzorKlasyfikacjiSprzedaz);
    mWzorzecKlasyfikacjiKatalog  = mGenerujWzorzecKlasyfikacji(nrKlasyfikacjiKatalog.toInt(), wzorKlasyfikacjiKatalog);
    mWzorzecKlasyfikacjiWylacz   = mGenerujWzorzecKlasyfikacji(nrKlasyfikacjiWylacz.toInt(),  wzorKlasyfikacjiWylacz);


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
    //poczatek transakcji
    if(!mKCPosDB.transaction())
        return false;

    //pozycje z kc posa : status (klasyfikacja), id (PrestaTowary.IDPresta), idKC(Kod)
    //cenaKC (ce_sb), kategriaKC ( KodGrupy), kategoria (PrestaKategorie.IDKategoriaPresta),
    // nazwa ( Nazwa), ean ( KodPaskowy)
    QSqlQuery kcTowaryPos (mKCPosDB);

    kcTowaryPos.exec("SELECT Towary.Kod, Towary.KodPaskowy, Towary.Nazwa, Towary.Klasyfikacja, Towary.ce_sb, Towary.KodGrupy, "
                     "PrestaTowary.IDPresta, PrestaKategorie.IDKategoriaPresta "
                     "FROM (Towary "
                     "LEFT JOIN PrestaTowary "
                     "ON Towary.Kod = PrestaTowary.IDKC )"
                     "LEFT JOIN PrestaKategorie "
                     "ON Towary.KodGrupy = PrestaKategorie.IDGrupyKC "
                     "WHERE ((Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiSprzedaz + " )"
                     "OR (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiKatalog + " ) "
                     "OR (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiWylacz + "AND PrestaTowary.IDPresta <> 0 )) " );


    //////////////////////////
    //Pozycje z KC Firmy cenaPresta(ce_sb2) krotkiOpis ( gramatura, jednostka)


    QVector<unsigned> listaProduktow (ilosc);
    //////////////////////////
    for (int i = 0; i < ilosc && kcTowaryPos.next() ; i++ ){



        //zapytanie do KC firmy
        QSqlQuery kcTowaryFirma(mKCFirmaDB);
        kcTowaryFirma.exec("SELECT Towary.ce_sb2, Towary.Gramatura, Jednostki.Nazwa "
                           "FROM (Towary "
                           "LEFT JOIN Jednostki "
                           "ON Towary.IdJedNorm = Jednostki.Id ) "
                           "WHERE Towary.Kod = '" + kcTowaryPos.value(0).toString() + "'");
        kcTowaryFirma.next();

        Produkt produkt;

        //ustawianie statusu
        QString tmp = kcTowaryPos.value(3).toString();
        if (mPorownajKlasyfikacje(mWzorzecKlasyfikacjiSprzedaz, tmp))
            produkt.status = Produkt::SPRZEDAZ;
        else
            if(mPorownajKlasyfikacje(mWzorzecKlasyfikacjiKatalog, tmp))
                produkt.status = Produkt::KATALOG;
            else
                if (mPorownajKlasyfikacje(mWzorzecKlasyfikacjiWylacz, tmp))
                        produkt.status = Produkt::WYLACZ;

        produkt.id = kcTowaryPos.value(6).toInt();

        produkt.idKC = kcTowaryPos.value(0).toInt();
        listaProduktow << produkt.idKC;

        //cena presta (Z KC firmy)
        produkt.cenaPresta = kcTowaryFirma.value(0).toFloat();

        produkt.cenaKC = kcTowaryPos.value(4).toFloat();

        produkt.kategoria = kcTowaryPos.value(7).toInt();

        produkt.kategoriaKC = kcTowaryPos.value(5).toInt();

        produkt.nazwa = kcTowaryPos.value(2).toString();

        produkt.ean = kcTowaryPos.value(1).toString();

        //krotki opis (z KCfirmy)
        produkt.krotkiOpis = mGenerujOpisT(kcTowaryFirma);
/*
        qDebug()<< produkt.nazwa <<"\t"<<
                   produkt.cenaKC <<"\t"<<
                   produkt.ean << "\t"<<
                   produkt.id<< "\t" <<
                   produkt.idKC << "\t"<<
                   produkt.krotkiOpis << "\t" ;
*/

        produktyMapa[produkt.idKC] = produkt;

    }

   ///usuwanie produktow


    for (int i = 0; i < listaProduktow.size(); i++)
    {
        QSqlQuery usuwaniePos(mKCPosDB);
        usuwaniePos.exec("DELETE FROM Towary"
                         "WHERE Kod =" + QString::number(listaProduktow[i]));
    }

    return mKCPosDB.commit();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void KCFirma::zmianaProduktu(unsigned id, unsigned idKC, float cena) {

    //sprawdzanie czy jest juz w bazie
    QSqlQuery sprawdzanie(mKCPosDB);
    sprawdzanie.exec("SELECT IDPresta "
                     "FROM PrestaTowary "
                     "WHERE IDKC =" +QString::number(idKC));

    QSqlQuery zmiana(mKCPosDB);
    if(sprawdzanie.next()){
        zmiana.exec("UPDATE PrestaTowary "
                    "SET CenaPresta =" + QString::number(cena) + ", IDPresta = "+QString::number(id) + " " +
                    "WHERE IDKC =" +QString::number(idKC));
    }

    else{
        zmiana.exec("INSERT INTO PrestaTowary ( IDKC, IDPresta, CenaPresta) "
                    "VALUES (" + QString::number(idKC) +", " + QString::number(id) + ", " +
                    QString::number(cena)+ " )");
        qDebug()<<zmiana.lastError().text();
    }

}


//////////////////////////////////////////////////////////////////////////////////////////////////////
void KCFirma::zmianaKategorii(unsigned id, unsigned idKC) {
    //sprawdzanie czy jest juz w bazie IDGrupyKC, IDKategoriaPresta
    QSqlQuery sprawdzanie(mKCPosDB);
    sprawdzanie.exec("SELECT IDKategoriaPresta "
                     "FROM PrestaKategorie "
                     "WHERE IDGrupyKC =" +QString::number(idKC));

    QSqlQuery zmiana(mKCPosDB);

    //jezeli w bazie jest jedna krotka to ...(ms acces nie obsluguje metody size() )
    if(sprawdzanie.next()){
        zmiana.exec("UPDATE PrestaKategorie "
                    "SET IDKategoriaPresta =" + QString::number(id) + " " +
                    "WHERE IDGrupyKC =" +QString::number(idKC));
    }
    else{
        zmiana.exec("INSERT INTO PrestaKategorie ( IDGrupyKC, IDKategoriaPresta) "
                    "VALUES (" + QString::number(idKC) +", " + QString::number(id)+ " )");
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

QString KCFirma::mGenerujWzorzecKlasyfikacji(unsigned pozycja, QString status){

    QString wzorzec;
    wzorzec = "'";
    for(unsigned i = 0; i < 10; i++){
        if (i == pozycja)
            wzorzec += status;
        else
            wzorzec += "___";
    }
    wzorzec += "'";

    return wzorzec;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
bool KCFirma::mPorownajKlasyfikacje (QString wzor, QString klasyfikacja){

    for (int i = 0; i < wzor.size(); i++)
        if(!(wzor[i].toAscii() == klasyfikacja[i].toAscii() || wzor[i].toAscii() == '_'))
            return 0;
    return 1;


}

//////////////////////////////////////////////////////////////////////////////////////////////////////
QString KCFirma::mGenerujOpisT (QSqlQuery &zapytanie){

    QString jednostka = zapytanie.value(2).toString();
    QString gramatura = zapytanie.value(1).toString();

    return (gramatura+ QString (" ") + jednostka);

}

/////////////////////////////////KONIECC//////////////////////////////////////////////////////////////
