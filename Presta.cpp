#include "Presta.h"
#include <QMap>

#include <QDebug>
#include <Qt>

Presta::Presta(const Config &config, PSWebService *pswebService, KCFirma *kcFirma, QObject *parent) :
    QObject(parent),
    mPSWebService(pswebService),
    mKCFirma(kcFirma),
    mLangId(config.lang),
    mFinished(true),
    mProduktyUpload(5)
{
}

QDomDocument Presta::getPrestaXML() {
    QDomDocument doc;
    QDomElement root = doc.createElement("prestashop");
    root.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    doc.appendChild(root);
    return doc;
}

QDomElement Presta::buildXMLElement(QDomDocument& doc, const QString &name, const QString &value)
{
    QDomElement elem = doc.createElement(name);
    elem.appendChild(doc.createCDATASection(value));
    return elem;
}

QDomElement Presta::buildXMLElement(QDomDocument &doc, const QString& name, const QString &value, int lang) {
    QDomElement elem = doc.createElement(name);
    QDomElement elemLang = doc.createElement("language");
    elem.appendChild(elemLang);
    elemLang.setAttribute("id", lang);
    elemLang.appendChild(doc.createCDATASection(value));
    return elem;
}

QDomDocument Presta::toXML(const Produkt &produkt) {
    QDomDocument doc = getPrestaXML();
    QDomElement product = doc.createElement("product");
    doc.firstChild().appendChild(product);

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
        QDomElement associations = doc.createElement("associations");
        product.appendChild(associations);
        QDomElement categories = doc.createElement("categories");
        associations.appendChild(categories);

        unsigned cat = produkt.kategoria;
        unsigned i = 0;
        do {
            i++;
            QDomElement category = doc.createElement("category");
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
    QDomDocument doc = toXML(produkt);
    //qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.id = produkt.id;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("idKC", QVariant(produkt.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(productEdited()), Qt::QueuedConnection);
}

void Presta::add(const Produkt &produkt) {
    QDomDocument doc = toXML(produkt);
    //qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("idKC", QVariant(produkt.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(productAdded()), Qt::QueuedConnection);
}

void Presta::syncEdit(const Produkt &produkt) {
    QDomDocument doc = toXML(produkt);
    qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.id = produkt.id;
    opt.resource = "products";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
}

unsigned Presta::syncAdd(const Produkt &produkt) {
    QDomDocument doc = toXML(produkt);
    PSWebService::Options opt;
    opt.resource = "products";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
    unsigned id = Produkt::getId(result);
    return id;
}

QDomDocument Presta::toXML(const Kategoria &kategoria) {
    QDomDocument doc = getPrestaXML();
    QDomElement category = doc.createElement("category");
    doc.firstChild().appendChild(category);

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
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("idKC", QVariant(kategoria.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(categoryEdited()), Qt::QueuedConnection);
}

void Presta::add(const Kategoria &kategoria) {
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("idKC", QVariant(kategoria.idKC));
    connect(reply, SIGNAL(finished()), this, SLOT(categoryAdded()), Qt::QueuedConnection);
}

void Presta::syncEdit(const Kategoria &kategoria) {
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
}

void Presta::syncEdit(const Zamowienie &zamowienie) {
    QDomDocument doc = toXML(zamowienie);
    PSWebService::Options opt;
    opt.id = zamowienie.id;
    opt.resource = "orders";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
}

unsigned Presta::syncAdd(const Kategoria &kategoria) {
    QDomDocument doc = toXML(kategoria);
    qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.resource = "categories";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
    unsigned id = Kategoria::getId(result);
    return id;
}

void Presta::aktualizujKategorie()
{
    // najpierw œci¹gamy kategorie z internetu
    PSWebService::Options opt;
    opt.resource = "categories";
    opt.display = "[id,id_parent,active,name,link_rewrite,meta_title,meta_description,meta_keywords,description]";
    try {
        QDomDocument result = mPSWebService->syncGet(opt);
        QDomElement prestashop = result.firstChildElement("prestashop");
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
    } catch (PSWebService::PrestaError e) {
        emit error(e);
        throw e;
    } catch (PSWebService::OtherError e) {
        emit error(e);
        throw e;
    }
}

bool Presta::dodajProdukty()
{
    return mKCFirma->produkty(mProdukty, mProduktyUpload);
}

bool Presta::dodajProdukty(uint ile)
{
    return mKCFirma->produkty(mProdukty, ile);
}

void Presta::upload() {

    // ustawiamy flagê
    mFinished = false;

    // kasujemy bufor b³êdnie uploadowanych produktów
    mProduktyError.clear();

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
                // kategoria dodawana jest synchronicznie
                unsigned id = syncAdd(kat);
                // aktualizujemy powi¹zanie
                emit zmianaKategorii(id, it.value().kategoriaKC);
                mKatNadrzedne[id] = kat.nadrzedna;
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
        // w przeciwnym wypadku oznaczamy produkt jako b³êdnie uploadowany
        else {
            mProduktyError[it.value().idKC] = KATEGORIA;
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
        QDomDocument doc = PSWebService::readReply(reply);
        unsigned id = Produkt::getId(doc);
        float cena = Produkt::getCena(doc);
        mProdukty.remove(idKC);
        emit zmianaProduktu(id, idKC, cena);
    } catch (PSWebService::PrestaError e) {
        e.msg = "productAdded";
        mProduktyError[idKC] = ADD_ERROR;
        emit error(e);
    } catch (PSWebService::OtherError e) {
        e.msg = "productAdded";
        mProduktyError[idKC] = ADD_ERROR;
        emit error(e);
    }
    reply->deleteLater();

    // je¿eli wszystkie operacje zosta³y wykonane emitujemy odpowiedni sygna³
    checkFinished();
}

void Presta::productEdited()
{
    QNetworkReply* reply = (QNetworkReply*)QObject::sender();
    unsigned idKC = reply->property("idKC").toUInt();
    try {
        QDomDocument doc = PSWebService::readReply(reply);
        unsigned id = Produkt::getId(doc);
        float cena = Produkt::getCena(doc);
        mProdukty.remove(idKC);
        emit zmianaProduktu(id, idKC, cena);
    } catch (PSWebService::PrestaError e) {
        e.msg = "productEdited";
        mProduktyError[idKC] = EDIT_ERROR;
        emit error(e);
    } catch (PSWebService::OtherError e) {
        e.msg = "productEdited";
        mProduktyError[idKC] = EDIT_ERROR;
        emit error(e);
    }
    reply->deleteLater();

    // je¿eli wszystkie operacje zosta³y wykonane emitujemy odpowiedni sygna³
    checkFinished();
}

void Presta::checkFinished() {
    if(mProduktyError.size() == mProdukty.size()) {
        mFinished = true;
        emit uploadFinished();
    }
}

Zamowienie Presta::getZamowienie(QDomDocument &doc) const {
    QDomElement prestashop = doc.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement order = prestashop.firstChildElement("order");
        if(!order.isNull()) {
            Zamowienie zam;
            zam.id = order.firstChildElement("id").firstChild().toCDATASection().nodeValue().toUInt();
            zam.status = (Zamowienie::Status)order.firstChildElement("current_state").firstChild().toCDATASection().nodeValue().toUInt();

            zam.id_address_delivery = order.firstChildElement("id_address_delivery").firstChild().toCDATASection().nodeValue().toUInt();
            zam.id_address_invoice = order.firstChildElement("id_address_invoice").firstChild().toCDATASection().nodeValue().toUInt();
            zam.id_cart = order.firstChildElement("id_cart").firstChild().toCDATASection().nodeValue().toUInt();
            zam.id_currency = order.firstChildElement("id_currency").firstChild().toCDATASection().nodeValue().toUInt();
            zam.id_lang = order.firstChildElement("id_lang").firstChild().toCDATASection().nodeValue().toUInt();
            zam.id_customer = order.firstChildElement("id_customer").firstChild().toCDATASection().nodeValue().toUInt();
            zam.id_carrier = order.firstChildElement("id_carrier").firstChild().toCDATASection().nodeValue().toUInt();
            zam.module = order.firstChildElement("module").firstChild().toCDATASection().nodeValue();
            zam.invoice_number = order.firstChildElement("invoice_number").firstChild().toCDATASection().nodeValue().toUInt();
            zam.invoice_date = order.firstChildElement("invoice_date").firstChild().toCDATASection().nodeValue();
            zam.delivery_number = order.firstChildElement("delivery_number").firstChild().toCDATASection().nodeValue().toUInt();
            zam.delivery_date = order.firstChildElement("delivery_date").firstChild().toCDATASection().nodeValue();
            zam.valid = order.firstChildElement("valid").firstChild().toCDATASection().nodeValue().toUInt();
            zam.date_add = order.firstChildElement("date_add").firstChild().toCDATASection().nodeValue();
            zam.date_upd = order.firstChildElement("date_upd").firstChild().toCDATASection().nodeValue();
            zam.secure_key = order.firstChildElement("secure_key").firstChild().toCDATASection().nodeValue();
            zam.payment = order.firstChildElement("payment").firstChild().toCDATASection().nodeValue();
            zam.recyclable = order.firstChildElement("recyclable").firstChild().toCDATASection().nodeValue().toUInt();
            zam.gift = order.firstChildElement("gift").firstChild().toCDATASection().nodeValue().toUInt();
            zam.gift_message = order.firstChildElement("gift_message").firstChild().toCDATASection().nodeValue();
            zam.total_discounts = order.firstChildElement("total_discounts").firstChild().toCDATASection().nodeValue().toFloat();
            zam.total_paid = order.firstChildElement("total_paid").firstChild().toCDATASection().nodeValue().toFloat();
            zam.total_paid_real = order.firstChildElement("total_paid_real").firstChild().toCDATASection().nodeValue().toFloat();
            zam.total_products = order.firstChildElement("total_products").firstChild().toCDATASection().nodeValue().toFloat();
            zam.total_products_wt = order.firstChildElement("total_products_wt").firstChild().toCDATASection().nodeValue().toFloat();
            zam.total_shipping = order.firstChildElement("total_shipping").firstChild().toCDATASection().nodeValue().toFloat();
            zam.carrier_tax_rate = order.firstChildElement("carrier_tax_rate").firstChild().toCDATASection().nodeValue().toFloat();
            zam.total_wrapping = order.firstChildElement("carrier_tax_rate").firstChild().toCDATASection().nodeValue().toFloat();
            zam.shipping_number = order.firstChildElement("shipping_number").firstChild().toCDATASection().nodeValue().toUInt();
            zam.conversion_rate = order.firstChildElement("conversion_rate").firstChild().toCDATASection().nodeValue().toFloat();

            // produkty
            QDomElement associations = order.firstChildElement("associations");
            QDomNodeList order_rows = associations.firstChildElement("order_rows").elementsByTagName("order_row");
            for(int i=0; i<order_rows.size(); ++i) {
                QDomElement order_row = order_rows.at(i).toElement();
                Zamowienie::Produkt produkt;
                produkt.id = order_row.firstChildElement("product_id").firstChild().toCDATASection().nodeValue().toUInt();
                produkt.cena = order_row.firstChildElement("product_price").firstChild().toCDATASection().nodeValue().toFloat();
                produkt.ilosc = order_row.firstChildElement("product_quantity").firstChild().toCDATASection().nodeValue().toUInt();
                zam.produkty << produkt;
            }

            // zwracamy zamówienie
            return zam;
        } else {
            throw QString("chujnia");
        }
    } else {
        throw QString("chujnia");
    }
}

QDomDocument Presta::toXML(const Zamowienie &zamowienie)
{
    QDomDocument doc = getPrestaXML();
    QDomElement order = doc.createElement("order");
    doc.firstChild().appendChild(order);

    if(zamowienie.id > 0)
        order.appendChild(buildXMLElement(doc, "id",  QString::number(zamowienie.id)));
    if(zamowienie.status > 0)
        order.appendChild(buildXMLElement(doc, "current_state", QString::number((uint)zamowienie.status)));

    order.appendChild(buildXMLElement(doc, "id_address_delivery", QString::number(zamowienie.id_address_delivery)));
    order.appendChild(buildXMLElement(doc, "id_address_invoice", QString::number(zamowienie.id_address_invoice)));
    order.appendChild(buildXMLElement(doc, "id_cart", QString::number(zamowienie.id_cart)));
    order.appendChild(buildXMLElement(doc, "id_currency", QString::number(zamowienie.id_currency)));
    order.appendChild(buildXMLElement(doc, "id_lang", QString::number(zamowienie.id_lang)));
    order.appendChild(buildXMLElement(doc, "id_customer", QString::number(zamowienie.id_customer)));
    order.appendChild(buildXMLElement(doc, "id_carrier", QString::number(zamowienie.id_carrier)));
    order.appendChild(buildXMLElement(doc, "module", zamowienie.module));
    order.appendChild(buildXMLElement(doc, "invoice_number", QString::number(zamowienie.invoice_number)));
    order.appendChild(buildXMLElement(doc, "invoice_date", zamowienie.invoice_date));
    order.appendChild(buildXMLElement(doc, "delivery_number", QString::number(zamowienie.delivery_number)));
    order.appendChild(buildXMLElement(doc, "delivery_date", zamowienie.delivery_date));
    order.appendChild(buildXMLElement(doc, "valid", QString::number(zamowienie.valid)));
    order.appendChild(buildXMLElement(doc, "date_add", zamowienie.date_add));
    order.appendChild(buildXMLElement(doc, "date_upd", zamowienie.date_upd));
    order.appendChild(buildXMLElement(doc, "secure_key", zamowienie.secure_key));
    order.appendChild(buildXMLElement(doc, "payment", zamowienie.payment));
    order.appendChild(buildXMLElement(doc, "recyclable", QString::number(zamowienie.recyclable)));
    order.appendChild(buildXMLElement(doc, "gift", QString::number(zamowienie.gift)));
    order.appendChild(buildXMLElement(doc, "gift_message", zamowienie.gift_message));
    order.appendChild(buildXMLElement(doc, "total_discounts", QString::number(zamowienie.total_discounts)));
    order.appendChild(buildXMLElement(doc, "total_paid", QString::number(zamowienie.total_paid)));
    order.appendChild(buildXMLElement(doc, "total_paid_real", QString::number(zamowienie.total_paid_real)));
    order.appendChild(buildXMLElement(doc, "total_products", QString::number(zamowienie.total_products)));
    order.appendChild(buildXMLElement(doc, "total_products_wt", QString::number(zamowienie.total_products_wt)));
    order.appendChild(buildXMLElement(doc, "total_shipping", QString::number(zamowienie.total_shipping)));
    order.appendChild(buildXMLElement(doc, "carrier_tax_rate", QString::number(zamowienie.carrier_tax_rate)));
    order.appendChild(buildXMLElement(doc, "total_wrapping", QString::number(zamowienie.total_wrapping)));
    order.appendChild(buildXMLElement(doc, "shipping_number", QString::number(zamowienie.shipping_number)));
    order.appendChild(buildXMLElement(doc, "conversion_rate", QString::number(zamowienie.conversion_rate)));
    return doc;
}

Zamowienie Presta::getZamowienie(uint id)
{
    try {
        PSWebService::Options opt;
        opt.resource = "orders";
        opt.id = id;
        QDomDocument doc = mPSWebService->syncGet(opt);
        Zamowienie order = getZamowienie(doc);
        return order;
    } catch (PSWebService::PrestaError e) {
        e.msg = "zamowienie(uint id)";
        emit error(e);
        throw e;
    } catch (PSWebService::OtherError e) {
        e.msg = "zamowienie(uint id)";
        emit error(e);
        throw e;
    }
}

QList<Zamowienie> Presta::getZamowienie(QString filter)
{
    try {
        PSWebService::Options opt;
        opt.resource = "orders";
        opt.filter["id"] = filter;
        QDomDocument doc = mPSWebService->syncGet(opt);

        QList<Zamowienie> zamowienia;

        QDomElement prestashop = doc.firstChildElement("prestashop");
        if(!prestashop.isNull()) {
            QDomNodeList orders = prestashop.firstChildElement("orders").elementsByTagName("order");
            for(int i=0; i<orders.size(); ++i) {
                uint id = orders.at(i).toElement().attribute("id").toUInt();
                zamowienia << getZamowienie(id);
            }
            return zamowienia;
        } else {
            throw QString("chujnia");
        }
    } catch (PSWebService::PrestaError e) {
        e.msg = "zamowienie(QString filter)";
        emit error(e);
        throw e;
    } catch (PSWebService::OtherError e) {
        e.msg = "zamowienie(QString filter)";
        emit error(e);
        throw e;
    }
}

