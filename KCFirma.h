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

     Funkcja ma zwr�ci� dane o kategorii o id w KC-Firmie podanym jako argument.
     Kategori� zwraca w postaci odpowiedniej struktury danych. W nag��wku Kategoria.h
     podane jest jakie dane nale�y uzupe�ni�.
     Je�eli kategorii nie by�o wcze�niej w Presta, czyli nie ma jej w bazie powi�za�, to ustaw id=0, albo
     nie ruszaj, bo domy�lnie jest 0.

     \fn kategoria
     \param idKC
     \return Kategoria
    */
    Kategoria kategoria(unsigned idKC) const;
    /*!
     \brief Pobiera produkty do wys�ania

     Funkcja pobiera z bazy produkty do wys�ania. Produkty dodaje si� do mapy produktyMapa. Nale�y pami�ta�, �e
     ta mapa mo�e zawiera� produkty, kt�rych nie uda�o si� wys�a� wcze�niej, dlatego nie mo�na jej czy�ci�
     albo co� takiego.
     Dodajesz produkty w postaci odpowiedniej struktury danych. W nag��wku Produkt.h opisane jest jakie
     dane nale�y uzupe�ni�. Je�eli produktu nie ma w bazie (bo zosta� dodany �wie�o do Presta, to ustaw id=0,
     albo nie ruszaj bo domy�lnie jest 0.
     Je�eli wyci�gnie si� ju� produkt z bazy i utworzy odpowiedni obiekt klasy Produkt, to dodaje si� go
     w prosty spos�b do mapy:
     produktyMapa[idKC] = produkt;
     Po pobraniu produuktu z bazy danych nale�y go wywali� z tabeli produkt�w do wys�ania. Wyci�gni�te
     produkty b�d� przechowywane w programie i w razie np wy��czenia programu po prostu doda si� je z powrotem.

     \fn produkty
     \param produktyMapa Do tej mapy nale�y doda� produkty do wys�ania. Indeksem mapy jest id produktu w KC-Firmie.
     \param ilosc Ile produkt�w ma by� pobranych z bazy.
     \return bool Zwraca, czy uda�o si� pobra� dane z bazy.
    */
    bool produkty(QMap<unsigned, Produkt>& produktyMapa, unsigned ilosc);
    /*!
     \brief Zmieniony zosta� produkt w Prestashop

     Po pomy�lnej aktualizacji produktu w bazie Prestashop, wys�any zostanie sygna�. W tym slocie
     trzeba go odebra�. Metoda ma zaktualizowa� powi�zanie produktu w KC-Firmie z Presta zgodnie z
     podanymi danymi.
     Cen� w sklepie internetowym r�wnie� b�dziemy przechowywa� lokalnie, ze wzgl�d�w bezpiecze�stwa,
     �eby sobie kto� samowolnie nie zmieni� ceny w sklepie. Bo potem przy zam�wieniu trzeba sk�d�
     wzi�� cen� produktu.
     Powi�zanie w lokalnej bazie nale�y doda� lub zmieni�, je�eli ju� jest. Np je�eli w Presta kto�
     sobie towar usunie, to przy nast�pnej aktualizacji towaru system nada mu nowe ID. Trzeba wtedy
     zaktualizowa� powi�zanie.

     \fn zmianaProduktu
     \param id identyfikator w Prestashop
     \param idKC identyfikator w KC-Firma
     \param cena cena produktu
    */
    void zmianaProduktu(unsigned id, unsigned idKC, float cena);
    /*!
     \brief Zmieniona zosta�a kategoria w Prestashop

     Podobnie jak w przypadku produktu, je�eli kategoria zosta�a dodana do Presta, to nale�y
     zaktualizowa� powi�zanie zgodnie z podanymi danymi.

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

    //Wzorzec z ktorym por�wnywana jest klasyfikacja w metodzie produkty (do operatora "LIKE")
    QString mWzorzecKlasyfikacjiSprzedaz;
    QString mWzorzecKlasyfikacjiKatalog;
    QString mWzorzecKlasyfikacjiWylacz;

    QString mKCFirmaPath;
    QSqlDatabase mKCFirmaDB;

    QString mKCPosPath;
    QSqlDatabase mKCPosDB;


};

#endif // KCFIRMA_H
