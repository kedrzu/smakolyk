#ifndef PRESTA_H
#define PRESTA_H

#include <QObject>
#include "PSWebService.h"
#include <QVector>
#include <QMap>
#include "Kategoria.h"
#include "Produkt.h"
#include "Zamowienie.h"
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
     \brief Rodzaj błędu przy uploadzie produktu

     \enum ProduktError
    */
    enum ProduktError {
        ADD_ERROR,  /*!< Nie udało się dodać produktu */
        EDIT_ERROR, /*!< Nie udało się edytować produktu */
        KATEGORIA /*!< Nie udało się uploadować kategorii do której należy produkt */
    };

    Presta(const Config &config, PSWebService* pswebService, KCFirma* kcFirma, QObject *parent = 0);
    /*!
     \brief Żądanie pobrania listy kategorii z Prestashop.

    */
    void aktualizujKategorie();
    /*!
     \brief Zwraca bufor produktów do uploadu.

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
     \brief Dodaje do bufora uploadu produkty w liczbie określonej w ustawieniach.

     \return bool
    */
    bool dodajProdukty();
    /*!
     \brief Dodaje do bufora uploadu produkty w liczbie określonej w parametrze

     \param ile Ile produktów dodać do bufora
     \return bool
    */
    bool dodajProdukty(uint ile);
    /*!
     \brief Zwraca, czy upload został zakończony.

     \return bool
    */
    bool isFinished() const { return mFinished; }
    /*!
     \brief Zwraca listę produktów, których nie udało się uploadować wraz z rodzajem błędu.

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
     \brief Synchroniczna edycja zamówienia.

     \param zamowienie
    */
    void syncEdit(const Zamowienie& zamowienie);
    /*!
     \brief Zamienia produkt na postać XML

     \param produkt
     \return QDomDocument
    */
    QDomDocument toXML(const Produkt& produkt);
    /*!
     \brief Zamienia kategorię na postać XML

     \param kategoria
     \return QDomDocument
    */
    QDomDocument toXML(const Kategoria& kategoria);
    /*!
     \brief Zamienia zamówienie na postać XML

     \param zamowienie
     \return QDomDocument
    */
    QDomDocument toXML(const Zamowienie& zamowienie);
    /*!
     \brief Przekształca XML do zamówienia.

     \param doc
     \return Zamowienie
    */
    Zamowienie zamowienie(QDomDocument& doc) const;
    /*!
     \brief Pobiera z Presta zamówienie o podanym ID.

     \param id
     \return Zamowienie
    */
    Zamowienie zamowienie(uint id);
    /*!
     \brief Pobiera z presta zamówienia według podanego filtra.

     \param filter
     \return QList<Zamowienie>
    */
    QList<Zamowienie> zamowienie(QString filter);


signals:
    /*!
     \brief Żądanie aktualizacji powiązania produktu

     \param id Id produktu w Presta
     \param idKC Id produktu w KC-Firmie
     \param cena Cena produktu w sprzedaży przez internet
    */
    void zmianaProduktu(unsigned id, unsigned idKC, float cena);
    /*!
     \brief Żądanie aktualizacji powiązania kategorii

     \param id Id kategorii w Presta
     \param idKC Id kategorii w KC-Firmie
    */
    void zmianaKategorii(unsigned id, unsigned idKC);
    /*!
     \brief Sygnał emitowany w momencia zakończenia uploadu.

    */
    void uploadFinished();
    /*!
     \brief Wystąpienie błędu komunikacji z Presta z odpowiednimi danymi.

     \param err
    */
    void error(PSWebService::PrestaError err);
    /*!
     \brief Wystąpienie innego błędu (nie z Presta)

     \param err
    */
    void error(PSWebService::OtherError err);
    /*!
     \brief Informacja do debugu programu.

     \param msg
    */
    void debug(QString msg);
    /*!
     \brief Ostrzeżenie o problemach w działaniu programu

     \param msg
    */
    void warning(QString msg);
    /*!
     \brief Powiadomienie o działaniu programu.

     \param msg
    */
    void notice(QString msg);

public slots:
    /*!
     \brief Wywołanie slotu powoduje uploadowanie produktów w buforze

    */
    void upload();

protected slots:
    void productAdded();
    void productEdited();
    void categoryAdded();
    void categoryEdited();
    
protected:
    void edit(const Produkt& produkt);
    void add(const Produkt& produkt);
    void edit(const Kategoria& kategoria);
    void add(const Kategoria& kategoria);
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
