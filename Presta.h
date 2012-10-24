#ifndef PRESTA_H
#define PRESTA_H

#include <QObject>
#include "PSWebService.h"
#include <QVector>
#include <QMap>
#include "Kategoria.h"
#include "Produkt.h"
#include "KCFirma.h"

class Presta : public QObject
{
    Q_OBJECT
public:
    Presta(PSWebService* pswebService, KCFirma* kcFirma, QObject *parent = 0);
    QDomDocument* toXML(const Produkt& produkt);
    void syncEdit(const Produkt& produkt);
    unsigned syncAdd(const Produkt& produkt);
    QDomDocument* toXML(const Kategoria& kategoria);
    void syncEdit(const Kategoria& kategoria);
    unsigned syncAdd(const Kategoria& kategoria);
    QMap<unsigned, Produkt>& produkty() { return mProdukty;}
    Produkt &produkty(unsigned i) { return mProdukty[i];}

signals:
    void zmianaProduktu(unsigned id, unsigned idKC, float cena);
    void zmianaKategorii(unsigned id, unsigned idKC);
    void uploadFinished();
    void error(PSWebService::PrestaError err);
    void error(PSWebService::OtherError err);
    void debug(QString msg);
    void warning(QString msg);
    void notice(QString msg);

public slots:
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

    PSWebService *mPSWebService;
    KCFirma *mKCFirma;
    unsigned mLangId;
    QMap<unsigned, unsigned> mKatNadrzedne;
    QMap<unsigned, Produkt> mProdukty;
    QList<unsigned> mProduktyUploaded, mProduktyError;
};

#endif // PRESTA_H
