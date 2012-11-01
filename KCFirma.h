#ifndef KCFIRMA_H
#define KCFIRMA_H

#include <QtSql/QtSql>
#include <QObject>
#include <QMap>
#include "Kategoria.h"
#include "Produkt.h"
#include "Config.h"

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
    explicit KCFirma(Config& config, QObject *parent = 0);
    /*!
     \brief Pobranie kategorii z bazy danych

     Funkcja ma zwróciæ dane o kategorii o id w KC-Firmie podanym jako argument.
     Kategoriê zwraca w postaci odpowiedniej struktury danych. W nag³ówku Kategoria.h
     podane jest jakie dane nale¿y uzupe³niæ.
     Je¿eli kategorii nie by³o wczeœniej w Presta, czyli nie ma jej w bazie powi¹zañ, to ustaw id=0, albo
     nie ruszaj, bo domyœlnie jest 0.

     \fn kategoria
     \param idKC
     \return Kategoria
    */
    Kategoria kategoria(unsigned idKC) const;
    /*!
     \brief Pobiera produkty do wys³ania

     Funkcja pobiera z bazy produkty do wys³ania. Produkty dodaje siê do mapy produktyMapa. Nale¿y pamiêtaæ, ¿e
     ta mapa mo¿e zawieraæ produkty, których nie uda³o siê wys³aæ wczeœniej, dlatego nie mo¿na jej czyœciæ
     albo coœ takiego.
     Dodajesz produkty w postaci odpowiedniej struktury danych. W nag³ówku Produkt.h opisane jest jakie
     dane nale¿y uzupe³niæ. Je¿eli produktu nie ma w bazie (bo zosta³ dodany œwie¿o do Presta, to ustaw id=0,
     albo nie ruszaj bo domyœlnie jest 0.
     Je¿eli wyci¹gnie siê ju¿ produkt z bazy i utworzy odpowiedni obiekt klasy Produkt, to dodaje siê go
     w prosty sposób do mapy:
     produktyMapa[idKC] = produkt;
     Po pobraniu produuktu z bazy danych nale¿y go wywaliæ z tabeli produktów do wys³ania. Wyci¹gniête
     produkty bêd¹ przechowywane w programie i w razie np wy³¹czenia programu po prostu doda siê je z powrotem.

     \fn produkty
     \param produktyMapa Do tej mapy nale¿y dodaæ produkty do wys³ania. Indeksem mapy jest id produktu w KC-Firmie.
     \param ilosc Ile produktów ma byæ pobranych z bazy.
     \return bool Zwraca, czy uda³o siê pobraæ dane z bazy.
    */
    bool produkty(QMap<unsigned, Produkt>& produktyMapa, unsigned ilosc);
    /*!
     \brief Zmieniony zosta³ produkt w Prestashop

     Po pomyœlnej aktualizacji produktu w bazie Prestashop, wys³any zostanie sygna³. W tym slocie
     trzeba go odebraæ. Metoda ma zaktualizowaæ powi¹zanie produktu w KC-Firmie z Presta zgodnie z
     podanymi danymi.
     Cenê w sklepie internetowym równie¿ bêdziemy przechowywaæ lokalnie, ze wzglêdów bezpieczeñstwa,
     ¿eby sobie ktoœ samowolnie nie zmieni³ ceny w sklepie. Bo potem przy zamówieniu trzeba sk¹dœ
     wzi¹æ cenê produktu.
     Powi¹zanie w lokalnej bazie nale¿y dodaæ lub zmieniæ, je¿eli ju¿ jest. Np je¿eli w Presta ktoœ
     sobie towar usunie, to przy nastêpnej aktualizacji towaru system nada mu nowe ID. Trzeba wtedy
     zaktualizowaæ powi¹zanie.

     \fn zmianaProduktu
     \param id identyfikator w Prestashop
     \param idKC identyfikator w KC-Firma
     \param cena cena produktu
    */
    void zmianaProduktu(unsigned id, unsigned idKC, float cena);
    /*!
     \brief Zmieniona zosta³a kategoria w Prestashop

     Podobnie jak w przypadku produktu, je¿eli kategoria zosta³a dodana do Presta, to nale¿y
     zaktualizowaæ powi¹zanie zgodnie z podanymi danymi.

     \fn zmianaKategorii
     \param id identyfikator w Prestashop
     \param idKC identyfikator w KC-Firma
    */
    void zmianaKategorii(unsigned id, unsigned idKC);

public:

    enum Blad {
        BLAD_POLACZENIA_Z_BAZA,
        BLAD_ZAPYTANIA_GRUPY
    };

private:

    QString mGenerujWzorzecKlasyfikacji(unsigned pozycja, QString status);

    //Wzorzec z ktorym porównywana jest klasyfikacja w metodzie produkty (do operatora "LIKE")
    QString mWzorzecKlasyfikacjiSprzedaz;
    QString mWzorzecKlasyfikacjiKatalog;
    QString mWzorzecKlasyfikacjiWylacz;

    QString mKCFirmaPath;
    QSqlDatabase mKCFirmaDB;

    QString mKCPosPath;
    QSqlDatabase mKCPosDB;


};

#endif // KCFIRMA_H
