#include "Presta.h"
#include <QMap>

#include <QDebug>

Presta::Presta(PSWebService *pswebService, KCFirma *kcFirma, QObject *parent) :
    QObject(parent),
    mPSWebService(pswebService),
    mKCFirma(kcFirma),
    mLangId(6)
{
}

QDomDocument* Presta::getPrestaXML() {
    QDomDocument* doc = new QDomDocument;
    QDomElement root = doc->createElement("prestashop");
    root.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    doc->appendChild(root);
    return doc;
}

QDomElement Presta::buildXMLElement(QDomDocument* doc, const QString &name, const QString &value)
{
    QDomElement elem = doc->createElement(name);
    elem.appendChild(doc->createCDATASection(value));
    return elem;
}

QDomElement Presta::buildXMLElement(QDomDocument *doc, const QString& name, const QString &value, int lang) {
    QDomElement elem = doc->createElement(name);
    QDomElement elemLang = doc->createElement("language");
    elem.appendChild(elemLang);
    elemLang.setAttribute("id", lang);
    elemLang.appendChild(doc->createCDATASection(value));
    return elem;
}

QDomDocument* Presta::toXML(const Produkt &produkt) {
    QDomDocument* doc = getPrestaXML();
    QDomElement product = doc->createElement("product");
    doc->firstChild().appendChild(product);

    if(produkt.id > 0)
        product.appendChild(buildXMLElement(doc, "id",  QString::number(produkt.id)));
    product.appendChild(buildXMLElement(doc, "price", QString::number(produkt.cena)));
    product.appendChild(buildXMLElement(doc, "out_of_stock", QString::number(produkt.outOfStock)));
    product.appendChild(buildXMLElement(doc, "quantity", QString::number(produkt.ilosc)));
    product.appendChild(buildXMLElement(doc, "ean13", produkt.ean));
    QString ref = QString::number(produkt.idKC);
    while(ref.length() < 11 ) {
        ref.prepend("0");
    }
    product.appendChild(buildXMLElement(doc, "reference", ref));
    product.appendChild(buildXMLElement(doc, "active", produkt.status > 0 ? "1" : "0"));
    product.appendChild(buildXMLElement(doc, "show_price", produkt.status > 1 ? "1" : "0"));
    product.appendChild(buildXMLElement(doc, "available_for_order", produkt.status > 2 ? "1" : "0"));
    product.appendChild(buildXMLElement(doc, "name", produkt.nazwa, mLangId));
    product.appendChild(buildXMLElement(doc, "meta_description", produkt.metaOpis, mLangId));
    product.appendChild(buildXMLElement(doc, "meta_keywords", produkt.metaKeywords, mLangId));
    product.appendChild(buildXMLElement(doc, "meta_title", produkt.metaTytul, mLangId));
    product.appendChild(buildXMLElement(doc, "link_rewrite", produkt.przyjaznyUrl, mLangId));
    product.appendChild(buildXMLElement(doc, "available_now", produkt.dostepnyTeraz, mLangId));
    product.appendChild(buildXMLElement(doc, "available_later", produkt.dostepnyPozniej, mLangId));
    product.appendChild(buildXMLElement(doc, "description", produkt.opis, mLangId));
    product.appendChild(buildXMLElement(doc, "description_short", produkt.krotkiOpis, mLangId));
    // dodawanie do kategorii
    if(produkt.kategoria > 0) {
        QDomElement associations = doc->createElement("associations");
        product.appendChild(associations);
        QDomElement categories = doc->createElement("categories");
        associations.appendChild(categories);

        unsigned cat = produkt.kategoria;
        unsigned i = 0;
        do {
            i++;
            QDomElement category = doc->createElement("category");
            categories.appendChild(category);
            category.appendChild(buildXMLElement(doc, "id", QString::number(cat)));

            if(mKatNadrzedne.contains(cat)) {
                cat = mKatNadrzedne[cat];
            } else {
                break;
            }

            if(i>100) throw std::string("chujnia!");
        } while(cat > 0);
    }

    return doc;
}

void Presta::edit(const Produkt &produkt) {
    QDomDocument* doc = toXML(produkt);
    //qDebug() << doc->toByteArray();
    PSWebService::Options opt;
    opt.id = produkt.id;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->put(opt, *doc);
    reply->setProperty("idKC", QVariant(produkt.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(productEdited()));
    delete doc;
}

void Presta::add(const Produkt &produkt) {
    QDomDocument* doc = toXML(produkt);
    //qDebug() << doc->toByteArray();
    PSWebService::Options opt;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->post(opt, *doc);
    reply->setProperty("idKC", QVariant(produkt.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(productAdded()));
    delete doc;
}

void Presta::syncEdit(const Produkt &produkt) {
    QDomDocument* doc = toXML(produkt);
    qDebug() << doc->toByteArray();
    PSWebService::Options opt;
    opt.id = produkt.id;
    opt.resource = "products";
    QDomDocument* result = mPSWebService->syncPut(opt, *doc);
    qDebug() << result->toByteArray();
    delete doc;
}

unsigned Presta::syncAdd(const Produkt &produkt) {
    QDomDocument* doc = toXML(produkt);
    PSWebService::Options opt;
    opt.resource = "products";
    QDomDocument* result = mPSWebService->syncPost(opt, *doc);
    unsigned id = Produkt::getId(result);
    delete result;
    delete doc;
    return id;
}

QDomDocument* Presta::toXML(const Kategoria &kategoria) {
    QDomDocument* doc = getPrestaXML();
    QDomElement category = doc->createElement("category");
    doc->firstChild().appendChild(category);

    if(kategoria.id > 0)
        category.appendChild(buildXMLElement(doc, "id",  QString::number(kategoria.id)));
    if(kategoria.nadrzedna > 0)
        category.appendChild(buildXMLElement(doc, "id_parent",  QString::number(kategoria.nadrzedna)));
    category.appendChild(buildXMLElement(doc, "active", kategoria.status > 0 ? "1" : "0"));
    category.appendChild(buildXMLElement(doc, "name", kategoria.nazwa, mLangId));
    category.appendChild(buildXMLElement(doc, "meta_description", kategoria.metaOpis, mLangId));
    category.appendChild(buildXMLElement(doc, "meta_keywords", kategoria.metaKeywords, mLangId));
    category.appendChild(buildXMLElement(doc, "meta_title", kategoria.metaTytul, mLangId));
    category.appendChild(buildXMLElement(doc, "link_rewrite", kategoria.przyjaznyUrl, mLangId));
    category.appendChild(buildXMLElement(doc, "description", kategoria.opis, mLangId));
    return doc;
}

void Presta::edit(const Kategoria &kategoria) {
    QDomDocument* doc = toXML(kategoria);
    //qDebug() << doc->toByteArray();
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->put(opt, *doc);
    reply->setProperty("idKC", QVariant(kategoria.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(categoryEdited()));
    delete doc;
}

void Presta::add(const Kategoria &kategoria) {
    QDomDocument* doc = toXML(kategoria);
    //qDebug() << doc->toByteArray();
    PSWebService::Options opt;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->post(opt, *doc);
    reply->setProperty("idKC", QVariant(kategoria.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(categoryAdded()));
    delete doc;
}

void Presta::syncEdit(const Kategoria &kategoria) {
    QDomDocument* doc = toXML(kategoria);
    qDebug() << doc->toByteArray();
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QDomDocument* result = mPSWebService->syncPut(opt, *doc);
    qDebug() << result->toByteArray();
    delete doc;
}

unsigned Presta::syncAdd(const Kategoria &kategoria) {
    QDomDocument* doc = toXML(kategoria);
    qDebug() << doc->toByteArray();
    PSWebService::Options opt;
    opt.resource = "categories";
    QDomDocument* result = mPSWebService->syncPost(opt, *doc);
    unsigned id = Kategoria::getId(result);
    delete result;
    delete doc;
    return id;
}

void Presta::upload() {

    // najpierw œci¹gamy kategorie z internetu
    PSWebService::Options opt;
    opt.resource = "categories";
    opt.display = "[id,id_parent,active,name,link_rewrite,meta_title,meta_description,meta_keywords,description]";
    QDomDocument* result = mPSWebService->syncGet(opt);
    QDomElement prestashop = result->firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement categories = prestashop.firstChildElement("categories");
        if(!categories.isNull()) {
            // czyœcimy star¹ mapê
            mKatNadrzedne.clear();
            // pobieramy kategorie
            QDomNodeList cats = categories.elementsByTagName("category");
            for(int i=0; i<cats.size(); ++i) {
                QDomNode cat = cats.at(i);
                if(!cat.isNull() && cat.hasChildNodes()) {
                    unsigned id = cat.firstChildElement("id").firstChild().toCDATASection().nodeValue().toUInt();
                    unsigned nadrzedna = cat.firstChildElement("id_parent").firstChild().toCDATASection().nodeValue().toUInt();
                    mKatNadrzedne[id] = nadrzedna;
                }
            }
        } else {
            // TODO ³apanie b³edów
        }
    }

    // iterujemy po produktach do aktualizacji
    QMapIterator<unsigned, Produkt> it(mProdukty);
    while(it.hasNext()) {
        it.next();
        // je¿eli wyst¹pi b³¹d przy dodawaniu kategorii, to nie dodajemy produktu
        bool err = false;
        // kategoria jest produktu nowa, lub by³a w Presta ale ju¿ jej nie ma => dodajemy do Presta
        if(it.value().kategoria == 0 || !mKatNadrzedne.contains(it.value().kategoria)) {
            Kategoria kat = mKCFirma->kategoria(it.value().kategoriaKC);
            kat.id = 0;
            // dodajemy kategoriê, je¿eli sie nie uda emitujemy blad i kontynujemy z nastepnym produktem
            try {
                unsigned id = syncAdd(kat);
                emit zmianaKategorii(id, it.value().kategoriaKC);
                mKatNadrzedne[id] = 1;
            } catch (PSWebService::PrestaError e) {
                emit error(e);
                err = true;
            } catch (PSWebService::OtherError e) {
                emit error(e);
                err = true;
            }
        }
        // je¿eli produkt ma ju¿ prawid³ow¹ kategoriê, to dodajemy
        if(!err) {
            // je¿eli produkt jest w bazie => edytujemy
            if(it.value().id > 0) {
                edit(it.value());
            }
            // je¿eli nie, dodajemy
            else {
                add(it.value());
            }
        }
    }

}

void Presta::categoryAdded()
{

}

void Presta::categoryEdited()
{

}

void Presta::productAdded()
{
    QNetworkReply* reply = (QNetworkReply*)QObject::sender();
    unsigned idKC = reply->property("idKC").toUInt();
    try {
        QDomDocument *doc = PSWebService::readReply(reply);
        unsigned id = Produkt::getId(doc);
        float cena = Produkt::getCena(doc);
        mProduktyUploaded.push_back(idKC);
        emit zmianaProduktu(id, idKC, cena);
    } catch (PSWebService::PrestaError e) {
        mProduktyError << idKC;
        emit error(e);
    } catch (PSWebService::OtherError e) {
        mProduktyError << idKC;
        emit error(e);
    }
    reply->deleteLater();

    // je¿eli wszystkie operacje zosta³y wykonane emitujemy odpowiedni sygna³
    if(mProduktyUploaded.size() + mProduktyError.size() == mProdukty.size()) {
        foreach(unsigned i, mProduktyUploaded) {
            mProdukty.remove(i);
        }
        mProduktyUploaded.clear();
        mProduktyError.clear();
        emit uploadFinished();
    }
}

void Presta::productEdited()
{

}
