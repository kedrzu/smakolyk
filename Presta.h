#ifndef PRESTA_H
#define PRESTA_H

#include <QObject>
#include "PSWebService.h"
#include <QVector>
#include <QMap>
#include "Kategoria.h"
#include "Produkt.h"
#include "Zamowienie.h"
#include "SpecificPrice.h"
#include "KCFirma.h"

/*!
 \brief

 \class Presta Presta.h "Presta.h"
*/
class Presta : public QObject
{
    Q_OBJECT
public:
    /*!
     \brief Rodzaj bledu przy uploadzie produktu

     \enum ProduktError
    */
    enum ProduktError {
        ADD_ERROR,  /*!< Nie udalo sie dodac produktu */
        EDIT_ERROR, /*!< Nie udalo sie edytowac produktu */
        KATEGORIA /*!< Nie udalo sie uploadowac kategorii do ktorej nalezy produkt */
    };

    Presta(const Config &config, PSWebService* pswebService, KCFirma* kcFirma, QObject *parent = 0);
    /*!
     \brief Ladanie pobrania listy kategorii z Prestashop.

    */
    void aktualizujKategorie();
    /*!
     \brief Zwraca bufor produktow do uploadu.

     \return QMap<unsigned, Produkt>
    */
    QMap<unsigned, Produkt>& buforProduktow() { return mProdukty;}
    /*!
     \brief Zwraca produkt z bufora uploadu.

     \param i Id produktu w KC-Firmie
     \return Produkt
    */
    Produkt &buforProduktow(unsigned i) { return mProdukty[i];}
    /*!
     \brief Dodaje do bufora uploadu produkty w liczbie okreslonej w ustawieniach.

     \return bool
    */
    bool dodajProdukty();
    /*!
     \brief Dodaje do bufora uploadu produkty w liczbie okreslonej w parametrze

     \param ile Ile produktow dodac do bufora
     \return bool
    */
    bool dodajProdukty(uint ile);
    /*!
     \brief Na podstawie produktu zwraca cene specjalna.

     \param product
     \return SpecificPrice
    */
    SpecificPrice getSpecificPrice(const Produkt& product);
    /*!
     \brief Przeksztalca XML do zamowienia.

     \param doc
     \return Zamowienie
    */
    Zamowienie getZamowienie(QDomDocument& doc) const;
    /*!
     \brief Pobiera z Presta zamowienie o podanym ID.

     \param id
     \return Zamowienie
    */
    Zamowienie getZamowienie(uint id);
    /*!
     \brief Pobiera z presta zamowienia o podanym stanie.

     \param status
     \return QList<Zamowienie>
    */
    QList<Zamowienie> getZamowienie(Zamowienie::Status status);
    /*!
     \brief Zwraca, czy upload zostal zakonczony.

     \return bool
    */
    bool isFinished() const { return mFinished; }
    /*!
     \brief Zwraca liste produktow, ktorych nie udalo sie uploadowac wraz z rodzajem bledu.

     \return const QMap<unsigned, ProduktError>&
    */
    const QMap<unsigned, ProduktError>& produktyBledy() { return mProduktyError; }
    /*!
     \brief Synchroniczne dodawanie produktu w Presta.

     \param produkt
     \return unsigned
    */
    unsigned syncAdd(const Produkt& produkt);
    /*!
     \brief Synchroniczne dodawanie kategorii.

     \param kategoria
     \return unsigned
    */
    unsigned syncAdd(const Kategoria& kategoria);
    /*!
     \brief Synchroniczne dodawanie ceny specjalnej

     \param specificPrice
     \return unsigned
    */
    void syncAdd(const SpecificPrice& specificPrice);
    /*!
     \brief Synchroniczna edycja produktu w Presta

     \param produkt
    */
    void syncEdit(const Produkt& produkt);
    /*!
     \brief Synchroniczna edycja kategorii.

     \param kategoria
    */
    void syncEdit(const Kategoria& kategoria);
    /*!
     \brief Synchroniczna edycja zamowienia.

     \param zamowienie
    */
    void syncEdit(const Zamowienie& zamowienie);
    /*!
     \brief Synchroniczna edycja ceny specjalnej.

     \param specificPrice
    */
    void syncEdit(const SpecificPrice& specificPrice);
    /*!
     \brief Zamienia produkt na postac XML

     \param produkt
     \return QDomDocument
    */
    QDomDocument toXML(const Produkt& produkt);
    /*!
     \brief Zamienia kategorie na postac XML

     \param kategoria
     \return QDomDocument
    */
    QDomDocument toXML(const Kategoria& kategoria);
    /*!
     \brief Zamienia zamowienie na postac XML

     \param zamowienie
     \return QDomDocument
    */
    QDomDocument toXML(const Zamowienie& zamowienie);
    /*!
     \brief Zamienia cene specjalna na postac XML

     \param specificPrice
     \return QDomDocument
    */
    QDomDocument toXML(const SpecificPrice& specificPrice);


signals:
    /*!
     \brief Ladanie aktualizacji powiazania produktu

     \param id Id produktu w Presta
     \param idKC Id produktu w KC-Firmie
     \param cena Cena produktu w sprzedazy przez internet
    */
    void zmianaProduktu(unsigned id, unsigned idKC, float cena);
    /*!
     \brief Ladanie aktualizacji powiazania kategorii

     \param id Id kategorii w Presta
     \param idKC Id kategorii w KC-Firmie
    */
    void zmianaKategorii(unsigned id, unsigned idKC);
    /*!
     \brief Sygnal emitowany w momencia zakonczenia uploadu.

    */
    void uploadFinished();
    /*!
     \brief Wystapienie bledu komunikacji z Presta z odpowiednimi danymi.

     \param err
    */
    void error(PSWebService::PrestaError err);
    /*!
     \brief Wystapienie innego bledu (nie z Presta)

     \param err
    */
    void error(PSWebService::OtherError err);
    /*!
     \brief Informacja do debugu programu.

     \param msg
    */
    void debug(QString msg);
    /*!
     \brief Ostrzezenie o problemach w dzialaniu programu

     \param msg
    */
    void warning(QString msg);
    /*!
     \brief Powiadomienie o dzialaniu programu.

     \param msg
    */
    void notice(QString msg);

public slots:
    /*!
     \brief Wywolanie slotu powoduje uploadowanie produktow w buforze

    */
    void upload();

protected slots:
    void productAdded();
    void productEdited();
    void categoryAdded();
    void categoryEdited();
    void specificPriceAdded();
    void specificPriceEdited();
    
protected:
    void edit(const Produkt& produkt);
    void add(const Produkt& produkt);
    void edit(const Kategoria& kategoria);
    void add(const Kategoria& kategoria);
    void edit(const SpecificPrice& specificPrice);
    void add(const SpecificPrice& specificPrice);
    void checkFinished();
    static QDomElement buildXMLElement(QDomDocument &doc, const QString &name, const QString &value);
    static QDomElement buildXMLElement(QDomDocument &doc, const QString &name, const QString &value, int lang);
    static QDomDocument getPrestaXML();

    PSWebService *mPSWebService;
    KCFirma *mKCFirma;
    unsigned mLangId;
    QMap<unsigned, unsigned> mKatNadrzedne;
    QMap<unsigned, Produkt> mProdukty;
    QMap<unsigned, ProduktError> mProduktyError;
    bool mFinished;
    uint mProduktyUpload;
};

#endif // PRESTA_H
