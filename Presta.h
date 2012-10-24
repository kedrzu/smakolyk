#ifndef PRESTA_H
#define PRESTA_H

#include <QObject>
#include "PSWebService.h"
#include <QVector>
#include <QMap>
#include "Kategoria.h"
#include "Produkt.h"
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
     \brief Rodzaj b��du przy uploadzie produktu

     \enum ProduktError
    */
    enum ProduktError {
        ADD_ERROR,  /*!< Nie uda�o si� doda� produktu */
        EDIT_ERROR, /*!< Nie uda�o si� edytowa� produktu */
        KATEGORIA /*!< Nie uda�o si� uploadowa� kategorii do kt�rej nale�y produkt */
    };

    Presta(const Config &config, PSWebService* pswebService, KCFirma* kcFirma, QObject *parent = 0);
    QDomDocument* toXML(const Produkt& produkt);
    void syncEdit(const Produkt& produkt);
    unsigned syncAdd(const Produkt& produkt);
    QDomDocument* toXML(const Kategoria& kategoria);
    void syncEdit(const Kategoria& kategoria);
    unsigned syncAdd(const Kategoria& kategoria);
    /*!
     \brief Zwraca bufor produkt�w do uploadu.

     \fn produkty
     \return QMap<unsigned, Produkt>
    */
    QMap<unsigned, Produkt>& produkty() { return mProdukty;}
    /*!
     \brief Zwraca produkt z bufora uploadu.

     \fn produkty
     \param i Id produktu w KC-Firmie
     \return Produkt
    */
    Produkt &produkty(unsigned i) { return mProdukty[i];}
    /*!
     \brief Zwraca, czy upload zosta� zako�czony.

     \fn isFinished
     \return bool
    */
    bool isFinished() const { return mFinished; }
    /*!
     \brief ��danie pobrania listy kategorii z Prestashop.

     \fn aktualizujKategorie
    */
    void aktualizujKategorie();
    /*!
     \brief Zwraca list� produkt�w, kt�rych nie uda�o si� uploadowa� wraz z rodzajem b��du.

     \fn produktyBledy
     \return const QMap<unsigned, ProduktError>&
    */
    const QMap<unsigned, ProduktError>& produktyBledy() { return mProduktyError; }
    /*!
     \brief Dodaje do bufora uploadu produkty w liczbie okre�lonej w ustawieniach.

     \fn dodajProdukty
     \return bool
    */
    bool dodajProdukty();
    /*!
     \brief Dodaje do bufora uploadu produkty w liczbie okre�lonej w parametrze

     \fn dodajProdukty
     \param ile Ile produkt�w doda� do bufora
     \return bool
    */
    bool dodajProdukty(uint ile);

signals:
    /*!
     \brief ��danie aktualizacji powi�zania produktu

     \fn zmianaProduktu
     \param id Id produktu w Presta
     \param idKC Id produktu w KC-Firmie
     \param cena Cena produktu w sprzeda�y przez internet
    */
    void zmianaProduktu(unsigned id, unsigned idKC, float cena);
    /*!
     \brief ��danie aktualizacji powi�zania kategorii

     \fn zmianaKategorii
     \param id Id kategorii w Presta
     \param idKC Id kategorii w KC-Firmie
    */
    void zmianaKategorii(unsigned id, unsigned idKC);
    /*!
     \brief Sygna� emitowany w momencia zako�czenia uploadu.

     \fn uploadFinished
    */
    void uploadFinished();
    /*!
     \brief Wyst�pienie b��du komunikacji z Presta z odpowiednimi danymi.

     \fn error
     \param err
    */
    void error(PSWebService::PrestaError err);
    /*!
     \brief Wyst�pienie innego b��du (nie z Presta)

     \fn error
     \param err
    */
    void error(PSWebService::OtherError err);
    /*!
     \brief Informacja do debugu programu.

     \fn debug
     \param msg
    */
    void debug(QString msg);
    /*!
     \brief Ostrze�enie o problemach w dzia�aniu programu

     \fn warning
     \param msg
    */
    void warning(QString msg);
    /*!
     \brief Powiadomienie o dzia�aniu programu.

     \fn notice
     \param msg
    */
    void notice(QString msg);

public slots:
    /*!
     \brief Wywo�anie slotu powoduje uploadowanie produkt�w w buforze

     \fn upload
    */
    void upload();

protected slots:
    void productAdded();
    void productEdited();
    void categoryAdded();
    void categoryEdited();
    
protected:
    static QDomElement buildXMLElement(QDomDocument *doc, const QString &name, const QString &value);
    static QDomElement buildXMLElement(QDomDocument *doc, const QString &name, const QString &value, int lang);
    static QDomDocument* getPrestaXML();
    void edit(const Produkt& produkt);
    void add(const Produkt& produkt);
    void edit(const Kategoria& kategoria);
    void add(const Kategoria& kategoria);
    void checkFinished();

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
