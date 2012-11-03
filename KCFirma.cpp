#include "KCFirma.h"
#include <QDebug>

KCFirma::KCFirma(QSettings &settings, QObject *parent) :
    QObject(parent)
{
    //sciezki docelowo wczytywane z configa,
    mKCFirmaPath = settings.value(QString("KC-Firma/db"), QString("c:\\KCFirma2\\Dane\\SMAKOLYK.dan") ).toString();

    mKCFirmaDB = QSqlDatabase::addDatabase("QODBC", "KCFirma");
    mKCFirmaDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};FIL={MS Access};"
                               "DBQ=" + mKCFirmaPath);
    mKCFirmaDB.open();

    //////////////////////////

    mKCPosPath   = settings.value("BazaKCPos", QString("c:\\KCPos\\dane\\kasa.dan")).toString();
    mKCPosDB = QSqlDatabase::addDatabase("QODBC", "KCPos");
    mKCPosDB.setDatabaseName("DRIVER={Microsoft Access Driver (*.mdb)};"
                             "Dbq=" + mKCPosPath);
    mKCPosDB.open();

    //////////////////////////

    if( (!mKCPosDB.isOpen()) || (!mKCFirmaDB.isOpen()) ) {
        Exception e;
        e.type = QString::fromUtf8("Bląd otwierania bazy danych");
        throw e;
    }

    //wczytawna z configa miejscie w klasyfikacji w ktorym jest zapisany status towaru(czy do sprzedazy?)
    //i generowanie odpowiedniego wzoru do zapytania SQL
    //przykładowa klasyfikacja 006000000007002000000000000000
    // przykladowy wzor:      "006???????????????????????????"

    QString nrKlasyfikacjiSprzedaz = settings.value("nrKlasyfikacjiSprzedaz", QString::number(0)).toString();
    QString nrKlasyfikacjiKatalog  = settings.value("nrKlasyfikacjiKatalog",  QString::number(0)).toString();
    QString nrKlasyfikacjiWylacz   = settings.value("nrKlasyfikacjiWylacz"  , QString::number(0)).toString();
    QString wzorKlasyfikacjiSprzedaz = settings.value("wzorKlasyfikacjiSprzedaz", QString("002")).toString();
    QString wzorKlasyfikacjiKatalog  = settings.value("wzorKlasyfikacjiKatalog" , QString("001")).toString();
    QString wzorKlasyfikacjiWylacz   = settings.value("wzorKlasyfikacjiWylacz"  , QString("000")).toString();

    mWzorzecKlasyfikacjiSprzedaz = mGenerujWzorzecKlasyfikacji(nrKlasyfikacjiSprzedaz.toInt(),wzorKlasyfikacjiSprzedaz);
    mWzorzecKlasyfikacjiKatalog  = mGenerujWzorzecKlasyfikacji(nrKlasyfikacjiKatalog.toInt(), wzorKlasyfikacjiKatalog);
    mWzorzecKlasyfikacjiWylacz   = mGenerujWzorzecKlasyfikacji(nrKlasyfikacjiWylacz.toInt(),  wzorKlasyfikacjiWylacz);

    qDebug() << mPorownajKlasyfikacje(mWzorzecKlasyfikacjiSprzedaz,mWzorzecKlasyfikacjiSprzedaz);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
Kategoria KCFirma::kategoria(unsigned idKC) const {

    Kategoria kat;
    QSqlQuery kcFirmaGrupa (mKCFirmaDB);
    kcFirmaGrupa.exec("SELECT Grupy.Nazwa, Grupy.KodGrupy "
                      "FROM Grupy "
                      "WHERE KodGrupy='" + QString::number(idKC)+"'");

    QSqlQuery kcPosGrupa (mKCPosDB);
    kcPosGrupa.exec("SELECT PrestaKategorie.IDKategoriaPrestaSprzedaz, PrestaKategorie.IDKategoriaPrestaKatalog "
                      "FROM PrestaKategorie "
                      "WHERE IDGrupyKC=" + QString::number(idKC));

    qDebug() << kcFirmaGrupa.lastError().text();
    qDebug() << kcPosGrupa.lastError().text();

    if(kcFirmaGrupa.next()){
        kat.nazwa = kcFirmaGrupa.value(0).toString();
        kat.idKC  = kcFirmaGrupa.value(1).toInt();
        if(kcPosGrupa.next()){
            kat.id    = kcPosGrupa.value(0).toInt();
            kat.idKatalog = kcPosGrupa.value(1).toInt();
        }
        else
        {
            kat.id = 0;
            kat.idKatalog = 0;
        }
    }
    else {
        Exception e;
        e.type = QString::fromUtf8("Błąd bazy danych");
        e.msg = QString::fromUtf8("Brak Kategorii o podanym idKC");
        throw e;
    }

    if(kcFirmaGrupa.lastError().type() != QSqlError::NoError) {
        Exception e;
        e.type = QString::fromUtf8("Błąd bazy danych");
        e.msg = kcFirmaGrupa.lastError().text();
        throw e;
    }

    if(kcPosGrupa.lastError().type() != QSqlError::NoError) {
        Exception e;
        e.type = QString::fromUtf8("Błąd bazy danych");
        e.msg = kcPosGrupa.lastError().text();
        throw e;
    }

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

    kcTowaryPos.exec("SELECT TOP " + QString::number(ilosc)+" "
                     "Towary.Kod, Towary.KodPaskowy, Towary.Nazwa, Towary.Klasyfikacja, Towary.ce_sb, Towary.KodGrupy, "
                     "PrestaTowary.IDPresta, PrestaKategorie.IDKategoriaPrestaSprzedaz,PrestaKategorie.IDKategoriaPrestaKatalog, Towary.Status "
                     "FROM (Towary "
                     "LEFT JOIN PrestaTowary "
                     "ON Towary.Kod = PrestaTowary.IDKC )"
                     "LEFT JOIN PrestaKategorie "
                     "ON Towary.KodGrupy = PrestaKategorie.IDGrupyKC "
                     "WHERE ((Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiSprzedaz + " AND Towary.Status = 1 )"
                     "OR (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiKatalog + " AND Towary.Status = 1 ) "
                     "OR (Towary.Klasyfikacja LIKE " + mWzorzecKlasyfikacjiWylacz + " AND NOT(PrestaTowary.IDPresta) IS NULL AND Towary.Status = 1 )) " );



    //////////////////////////
    //Pozycje z KC Firmy cenaPresta(ce_sb2) krotkiOpis ( gramatura, jednostka)
    qDebug() << kcTowaryPos.lastError().text();
    qDebug() << kcTowaryPos.lastQuery();

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
        qDebug() << tmp ;
        if(kcTowaryPos.value(9).toBool()){

            if (mPorownajKlasyfikacje(mWzorzecKlasyfikacjiSprzedaz, tmp))
                produkt.status = Produkt::SPRZEDAZ;
            else
                if(mPorownajKlasyfikacje(mWzorzecKlasyfikacjiKatalog, tmp))
                    produkt.status = Produkt::KATALOG;
                else
                    produkt.status = Produkt::WYLACZ;
        }
        else
            produkt.status = Produkt::USUN;

        produkt.id = kcTowaryPos.value(6).toInt();

        produkt.idKC = kcTowaryPos.value(0).toInt();
        listaProduktow << produkt.idKC;

        //cena presta (Z KC firmy)
        produkt.cenaPresta = kcTowaryFirma.value(0).toFloat();

        produkt.cenaKC = kcTowaryPos.value(4).toFloat();

        produkt.kategoria = kcTowaryPos.value(7).toInt();
        produkt.kategoriaKatalog = kcTowaryPos.value(8).toInt();

        produkt.kategoriaKC = kcTowaryPos.value(5).toInt();

        produkt.nazwa = kcTowaryPos.value(2).toString();

        produkt.ean = kcTowaryPos.value(1).toString();

        //krotki opis (z KCfirmy)
        produkt.krotkiOpis = mGenerujOpisT(kcTowaryFirma);

        produkt.czytaj();

        produktyMapa[produkt.idKC] = produkt;

    }

    ///usuwanie produktow


    for (int i = 0; i < listaProduktow.size(); i++)
    {
        QSqlQuery usuwaniePos(mKCPosDB);
        usuwaniePos.exec("DELETE FROM Towary"
                         "WHERE Kod =" + QString::number(listaProduktow[i]));
    }

    bool tmp1 = mKCPosDB.commit();

    if(mKCPosDB.lastError().type() != QSqlError::NoError) {
        Exception e;
        e.type = QString::fromUtf8("Błąd bazy danych");
        e.msg = mKCPosDB.lastError().text();
        throw e;
    }


    return tmp1;
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
        //    qDebug()<<zmiana.lastError().text();
    }
    if(zmiana.lastError().type() != QSqlError::NoError) {
        Exception e;
        e.type = QString::fromUtf8("Błąd bazy danych");
        e.msg = zmiana.lastError().text();
        throw e;
    }

}


//////////////////////////////////////////////////////////////////////////////////////////////////////
void KCFirma::zmianaKategorii(unsigned idSprzedaz, unsigned idKatalog, unsigned idKC) {
    //sprawdzanie czy jest juz w bazie IDGrupyKC, IDKategoriaPresta
    QSqlQuery sprawdzanie(mKCPosDB);
    sprawdzanie.exec("SELECT IDKategoriaPrestaSprzedaz,IDKategoriaPrestaKatalog "
                     "FROM PrestaKategorie "
                     "WHERE IDGrupyKC =" +QString::number(idKC));

    QSqlQuery zmiana(mKCPosDB);


    //jezeli w bazie jest jedna krotka to ...(ms acces nie obsluguje metody size() )
    if(sprawdzanie.next()){
        zmiana.exec("UPDATE PrestaKategorie "
                    "SET IDKategoriaPrestaSprzedaz =" + QString::number(idSprzedaz) + " " +
                    "IDKategoriaPrestaKatalog =" +QString::number(idKatalog) +  " " +
                    "WHERE IDGrupyKC =" +QString::number(idKC));
    }
    else{
        zmiana.exec("INSERT INTO PrestaKategorie ( IDGrupyKC, IDKategoriaPrestaSprzedaz ,IDKategoriaPrestaKatalog) "
                    "VALUES (" + QString::number(idKC) +", " + QString::number(idSprzedaz)+ ", " + QString::number(idKatalog)+  " )");
    }
    if(zmiana.lastError().type() != QSqlError::NoError) {
        Exception e;
        e.type = QString::fromUtf8("Błąd bazy danych");
        e.msg = zmiana.lastError().text();
        throw e;
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

    /*
    for (int i = 0; i < wzor.size(); i++)
        if(!(wzor[i].toAscii() == klasyfikacja[i].toAscii() || wzor[i].toAscii() == '_'))
            return 0;
    return 1;
*/
    qDebug() << klasyfikacja.mid(0,3) << wzor.mid(1,3);
    if(klasyfikacja.mid(0,3) == wzor.mid(1,3))
        return 1;
    else
        return 0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////
QString KCFirma::mGenerujOpisT (QSqlQuery &zapytanie){

    QString jednostka = zapytanie.value(2).toString();
    QString gramatura = zapytanie.value(1).toString();

    return (gramatura+ QString (" ") + jednostka);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////

void KCFirma::mCzyscBaze(){

    QSqlQuery czysc(mKCPosDB);
    czysc.exec(QString("DELETE Towary.* "
                       "FROM Towary "
                       "WHERE ( ( NOT EXISTS "
                       "( SELECT PrestaTowary.IDKC, PrestaTowary.IDPresta "
                       "FROM PrestaTowary WHERE Towary.Kod = PrestaTowary.IDKC ) ) "
                       "AND Towary.Klasyfikacja LIKE %1 )").arg(mWzorzecKlasyfikacjiWylacz));



}



//////////////////////////////////////////KONIECC/////////////////////////////////////////////////////
