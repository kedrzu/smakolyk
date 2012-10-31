#ifndef KCPRESTA_H
#define KCPRESTA_H

#include <QObject>
#include "Presta.h"
#include "KCFirma.h"

class KCPresta : public Presta::Prestashop
{
    Q_OBJECT
public:
    KCPresta(const Config &config, PSWebService* pswebService, KCFirma* kcFirma, QObject *parent = 0);
    /*!
     \brief Rodzaj bledu przy uploadzie produktu

     \enum ProduktError
    */
    enum ProduktError {
        ADD_ERROR,  /*!< Nie udalo sie dodac produktu */
        EDIT_ERROR, /*!< Nie udalo sie edytowac produktu */
        KATEGORIA /*!< Nie udalo sie uploadowac kategorii do ktorej nalezy produkt */
    };
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
    Presta::SpecificPrice getSpecificPrice(const Produkt& produkt);

    /*!
     \brief Zwraca, czy upload zostal zakonczony.

     \return bool
    */
    bool isFinished() const { return mFinished; }
    Presta::Product kc2presta(const Produkt &produkt);
    Presta::Category kc2presta(const Kategoria &kategoria);
    /*!
     \brief Zwraca liste produktow, ktorych nie udalo sie uploadowac wraz z rodzajem bledu.

     \return const QMap<unsigned, ProduktError>&
    */
    const QMap<unsigned, ProduktError>& produktyBledy() { return mProduktyError; }

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
    void productAdded(QNetworkReply* reply);
    void productEdited(QNetworkReply* reply);
    void categoryAdded(QNetworkReply* reply);
    void categoryEdited(QNetworkReply* reply);
    void specificPriceAdded(QNetworkReply* reply);
    void specificPriceEdited(QNetworkReply* reply);
    void checkFinished();
    KCFirma *mKCFirma;
    QMap<unsigned, unsigned> mKatNadrzedne;
    QMap<unsigned, Produkt> mProdukty;

    QMap<unsigned, ProduktError> mProduktyError;
    bool mFinished;
    uint mProduktyUpload;
};

#endif // KCPRESTA_H
