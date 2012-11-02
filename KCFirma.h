#ifndef KCFIRMA_H
#define KCFIRMA_H

#include <QtSql/QtSql>
#include <QObject>
#include <QMap>
#include <QVector>
#include "Kategoria.h"
#include "Produkt.h"
#include "Exception.h"

/*!
 \brief

 \class KCFirma KCFirma.h "KCFirma.h"
*/
class KCFirma : public QObject
{
    Q_OBJECT
public:
    /*!
     \brief

     \fn KCFirma
     \param
     \param config Obiekt konfiguracyjny
     \param parent
    */
    explicit KCFirma(QSettings &settings, QObject *parent = 0);
    /*!
     \brief Pobranie kategorii z bazy danych

     Funkcja ma zwrócić dane o kategorii o id w KC-Firmie podanym jako argument.
     Kategorię zwraca w postaci odpowiedniej struktury danych. W nagłówku Kategoria.h
     podane jest jakie dane należy uzupełnić.
     Jeżeli kategorii nie było wcześniej w Presta, czyli nie ma jej w bazie powiązań, to ustaw id=0, albo
     nie ruszaj, bo domyślnie jest 0.

     \fn kategoria
     \param idKC
     \return Kategoria
    */
    Kategoria kategoria(unsigned idKC) const;
    /*!
     \brief Pobiera produkty do wysłania

     Funkcja pobiera z bazy produkty do wysłania. Produkty dodaje się do mapy produktyMapa. Należy pamiętać, że
     ta mapa może zawierać produkty, których nie udało się wysłać wcześniej, dlatego nie można jej czyścić
     albo coś takiego.
     Dodajesz produkty w postaci odpowiedniej struktury danych. W nagłówku Produkt.h opisane jest jakie
     dane należy uzupełnić. Jeżeli produktu nie ma w bazie (bo został dodany świeżo do Presta, to ustaw id=0,
     albo nie ruszaj bo domyślnie jest 0.
     Jeżeli wyciągnie się już produkt z bazy i utworzy odpowiedni obiekt klasy Produkt, to dodaje się go
     w prosty sposób do mapy:
     produktyMapa[idKC] = produkt;
     Po pobraniu produuktu z bazy danych należy go wywalić z tabeli produktów do wysłania. Wyciągnięte
     produkty będą przechowywane w programie i w razie np wyłączenia programu po prostu doda się je z powrotem.

     \fn produkty
     \param produktyMapa Do tej mapy należy dodać produkty do wysłania. Indeksem mapy jest id produktu w KC-Firmie.
     \param ilosc Ile produktów ma być pobranych z bazy.
     \return bool Zwraca, czy udało się pobrać dane z bazy.
    */
    bool produkty(QMap<unsigned, Produkt>& produktyMapa, unsigned ilosc);
    /*!
     \brief Zmieniony został produkt w Prestashop

     Po pomyślnej aktualizacji produktu w bazie Prestashop, wysłany zostanie sygnał. W tym slocie
     trzeba go odebrać. Metoda ma zaktualizować powiązanie produktu w KC-Firmie z Presta zgodnie z
     podanymi danymi.
     Cenę w sklepie internetowym również będziemy przechowywać lokalnie, ze względów bezpieczeństwa,
     żeby sobie ktoś samowolnie nie zmienił ceny w sklepie. Bo potem przy zamówieniu trzeba skądś
     wziąć cenę produktu.
     Powiązanie w lokalnej bazie należy dodać lub zmienić, jeżeli już jest. Np jeżeli w Presta ktoś
     sobie towar usunie, to przy następnej aktualizacji towaru system nada mu nowe ID. Trzeba wtedy
     zaktualizować powiązanie.

     \fn zmianaProduktu
     \param id identyfikator w Prestashop
     \param idKC identyfikator w KC-Firma
     \param cena cena produktu
    */
    void zmianaProduktu(unsigned id, unsigned idKC, float cena);
    /*!
     \brief Zmieniona została kategoria w Prestashop

     Podobnie jak w przypadku produktu, jeżeli kategoria została dodana do Presta, to należy
     zaktualizować powiązanie zgodnie z podanymi danymi.

     \fn zmianaKategorii
     \param id identyfikator w Prestashop
     \param idKC identyfikator w KC-Firma
    */
    void zmianaKategorii(unsigned idSprzedaz, unsigned idKatalog, unsigned idKC);

public:

    enum Blad {
        BLAD_POLACZENIA_Z_BAZA,
        BLAD_ZAPYTANIA_GRUPY
    };

private:

    Exception exception;

    QString mGenerujWzorzecKlasyfikacji(unsigned pozycja, QString status);
    bool mPorownajKlasyfikacje (QString wzor, QString klasyfikacja);
    QString mGenerujOpisT (QSqlQuery &zapytanie);
    void mCzyscBaze();

    //Wzorzec z ktorym porównywana jest klasyfikacja w metodzie produkty (do operatora "LIKE")
    QString mWzorzecKlasyfikacjiSprzedaz;
    QString mWzorzecKlasyfikacjiKatalog;
    QString mWzorzecKlasyfikacjiWylacz;

    QString mKCFirmaPath;
    QSqlDatabase mKCFirmaDB;

    QString mKCPosPath;
    QSqlDatabase mKCPosDB;


};

/*

  Wymagany format bazy danych KC Pos
  Dodatkowe tabele:
       PrestaTowary : IDGrupyKC, IDKategoriaPresta
       PrestaKategorie: IDKC, IDPresta, CenaPresta
       PrestaZamowienia: ID, IDZamowienia, IDTowaru, KodKreskowy, CenaB
  */

#endif // KCFIRMA_H
