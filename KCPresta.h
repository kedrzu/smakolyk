#ifndef KCPRESTA_H
#define KCPRESTA_H

#include <QObject>
#include "Presta.h"
#include "KCFirma.h"

class KCPresta : public QObject
{
    Q_OBJECT
public:
    KCPresta(const QSettings &settings, Presta::Prestashop *presta, KCFirma *kcFirma, QObject *parent = 0);
    /*!
     \brief Rodzaj bledu przy uploadzie produktu

     \enum ProduktError
    */
    enum ProduktError {
        ADD_ERROR,  /*!< Nie udalo sie dodac produktu */
        EDIT_ERROR, /*!< Nie udalo sie edytowac produktu */
        KATEGORIA /*!< Nie udalo sie uploadowac kategorii do ktorej nalezy produkt */
    };
    enum ZamowienieStatus {
        OCZEKUJE,
        W_REALIZACJI,
        DO_ODBIORU,
        WYSLANE,
        ZREALIZOWANE,
        ANULOWANE
    };

    /*!
     \brief Żądanie pobrania listy kategorii z Prestashop.

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
     \brief Zwraca, czy upload zostal zakonczony.

     \return bool
    */
    bool isUploadFinished() const { return mUploadFinished; }
    /*!
     \brief Zwraca liste produktow, ktorych nie udalo sie uploadowac wraz z rodzajem bledu.

     \return const QMap<unsigned, ProduktError>&
    */
    const QMap<unsigned, ProduktError>& produktyBledy() { return mProduktyError; }
    /*!
     \brief Wywołanie powoduje pobranie listy zamówień

    */
    QList<Presta::OrderHeader> pobierzZamowienia();
    ZamowienieStatus statusyZamowien(uint i) const { return mStatusyZamowien.key(i); }
    uint statusyZamowien(ZamowienieStatus status) const { return mStatusyZamowien[status]; }
    static QString statusyZamowienNazwa(ZamowienieStatus status);
signals:
    /*!
     \brief Sygnal emitowany w momencia zakonczenia uploadu.

    */
    void uploadFinished();
    /*!
     \brief Wystapienie bledu

     \param err
    */
    void error(const Exception& err);
    /*!
     \brief Informacja do debugu programu.

     \param msg
    */
    void debug(const QString& msg);
    /*!
     \brief Ostrzezenie o problemach w dzialaniu programu

     \param msg
    */
    void warning(const QString& msg);
    /*!
     \brief Powiadomienie o dzialaniu programu.

     \param msg
    */
    void notice(const QString& msg);

public slots:
    /*!
     \brief Wywolanie slotu powoduje uploadowanie produktow w buforze

    */
    void uploadProdukty();

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
    Presta::SpecificPrice getSpecificPrice(const Produkt& produkt);
    Presta::Product kc2presta(const Produkt &produkt);
    Presta::Category kc2presta(const Kategoria &kategoria);
    Presta::Prestashop *mPresta;
    KCFirma *mKCFirma;
    PSWebService* mPSWebService;
    QMap<uint, uint> mKatNadrzedne;
    QMap<uint, uint> mKatDodaneSprz;
    QMap<uint, uint> mKatDodaneKat;
    QMap<uint, Produkt> mProdukty;
    QMap<uint, ProduktError> mProduktyError;
    bool mUploadFinished;
    uint mProduktyUpload;
    QMap<ZamowienieStatus, uint> mStatusyZamowien;
    uint mConfigKatRootSprzedaz;
    uint mConfigKatRootKatalog;
};

#endif // KCPRESTA_H
