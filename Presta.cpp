#include "Presta.h"
#include <QMap>

#include <QDebug>
#include <Qt>

using namespace Presta;

Prestashop::Prestashop(const Config &config, PSWebService *pswebService, QObject *parent) :
    QObject(parent),
    mPSWebService(pswebService),
    mLangId(config.lang)
{
}

QDomDocument Prestashop::getPrestaXML() {
    QDomDocument doc;
    QDomElement root = doc.createElement("prestashop");
    root.setAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    doc.appendChild(root);
    return doc;
}

QDomElement Prestashop::buildXMLElement(QDomDocument& doc, const QString &name, const QString &value)
{
    QDomElement elem = doc.createElement(name);
    elem.appendChild(doc.createCDATASection(value));
    return elem;
}

QDomElement Prestashop::buildXMLElement(QDomDocument &doc, const QString& name, const QString &value, int lang) {
    QDomElement elem = doc.createElement(name);
    QDomElement elemLang = doc.createElement("language");
    elem.appendChild(elemLang);
    elemLang.setAttribute("id", lang);
    elemLang.appendChild(doc.createCDATASection(value));
    return elem;
}

QDomDocument Prestashop::toXML(const Product &product) {
    QDomDocument doc = getPrestaXML();
    QDomElement productElem = doc.createElement("product");
    doc.firstChild().appendChild(productElem);

    if(product.id > 0)
        productElem.appendChild(buildXMLElement(doc, "id",  QString::number(product.id)));
    productElem.appendChild(buildXMLElement(doc, "price", QString::number(product.price)));
    productElem.appendChild(buildXMLElement(doc, "out_of_stock", QString::number(product.out_of_stock)));
    productElem.appendChild(buildXMLElement(doc, "quantity", QString::number(product.quantity)));
    productElem.appendChild(buildXMLElement(doc, "ean13", product.ean));
    productElem.appendChild(buildXMLElement(doc, "reference", product.reference));
    productElem.appendChild(buildXMLElement(doc, "active", QString::number(product.active)));
    productElem.appendChild(buildXMLElement(doc, "show_price", QString::number(product.show_price)));
    productElem.appendChild(buildXMLElement(doc, "available_for_order", QString::number(product.available_for_order)));
    productElem.appendChild(buildXMLElement(doc, "name", product.name, mLangId));
    productElem.appendChild(buildXMLElement(doc, "meta_description", product.meta_description, mLangId));
    productElem.appendChild(buildXMLElement(doc, "meta_keywords", product.meta_keywords, mLangId));
    productElem.appendChild(buildXMLElement(doc, "meta_title", product.meta_title, mLangId));
    productElem.appendChild(buildXMLElement(doc, "link_rewrite", product.link_rewrite, mLangId));
    productElem.appendChild(buildXMLElement(doc, "available_now", product.available_now, mLangId));
    productElem.appendChild(buildXMLElement(doc, "available_later", product.available_later, mLangId));
    productElem.appendChild(buildXMLElement(doc, "description", product.description, mLangId));
    productElem.appendChild(buildXMLElement(doc, "description_short", product.description_short, mLangId));
    // dodawanie do kategorii
    if(product.id_category_default > 0) {
        productElem.appendChild(buildXMLElement(doc, "id_category_default", QString::number(product.id_category_default)));
    }
    if(!product.categories.empty()) {
        QDomElement associations = doc.createElement("associations");
        productElem.appendChild(associations);
        QDomElement categories = doc.createElement("categories");
        associations.appendChild(categories);

        QListIterator<uint> it(product.categories);
        while(it.hasNext()) {
            QDomElement category = doc.createElement("category");
            categories.appendChild(category);
            category.appendChild(buildXMLElement(doc, "id", QString::number(it.next())));
        }
    }

    return doc;
}

QNetworkReply *Prestashop::edit(const Product &product) {
    QDomDocument doc = toXML(product);
    //qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.id = product.id;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("idRef", QVariant(product.idRef));
    return reply;
}

QNetworkReply *Prestashop::add(const Product &product) {
    QDomDocument doc = toXML(product);
    //qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("idRef", QVariant(product.idRef));
    return reply;
}

void Prestashop::syncEdit(const Product &product) {
    QDomDocument doc = toXML(product);
    qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.id = product.id;
    opt.resource = "products";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
}

unsigned Prestashop::syncAdd(const Product &product) {
    QDomDocument doc = toXML(product);
    PSWebService::Options opt;
    opt.resource = "products";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
    unsigned id = Product::getId(result);
    return id;
}

QDomDocument Prestashop::toXML(const Kategoria &kategoria) {
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

QNetworkReply *Prestashop::edit(const Kategoria &kategoria) {
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("idKC", QVariant(kategoria.idKC));
return reply;
}

QNetworkReply *Prestashop::add(const Kategoria &kategoria) {
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("idKC", QVariant(kategoria.idKC));
return reply;
}

QNetworkReply *Prestashop::edit(const SpecificPrice &specificPrice) {
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.id = specificPrice.id;
    opt.resource = "specific_prices";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("id_product", QVariant(specificPrice.id_product));
return reply;
}

QNetworkReply *Prestashop::add(const SpecificPrice &specificPrice) {
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("id_product", QVariant(specificPrice.id_product));
return reply;
}

void Prestashop::syncEdit(const Kategoria &kategoria) {
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
}

void Prestashop::syncEdit(const Order &zamowienie) {
    QDomDocument doc = toXML(zamowienie);
    PSWebService::Options opt;
    opt.id = zamowienie.id;
    opt.resource = "orders";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
}

void Prestashop::syncEdit(const SpecificPrice &specificPrice) {
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.id = specificPrice.id;
    opt.resource = "specific_prices";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
}

unsigned Prestashop::syncAdd(const Kategoria &kategoria) {
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.resource = "categories";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
    unsigned id = Kategoria::getId(result);
    return id;
}

void Prestashop::syncAdd(const SpecificPrice &specificPrice) {
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
}

Order Prestashop::getOrder(QDomDocument &doc) const {
    QDomElement prestashop = doc.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement order = prestashop.firstChildElement("order");
        if(!order.isNull()) {
            Order zam;
            zam.id = order.firstChildElement("id").firstChild().toCDATASection().nodeValue().toUInt();
            zam.current_state = (Order::Status)order.firstChildElement("current_state").firstChild().toCDATASection().nodeValue().toUInt();

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
                Order::OrderRow produkt;
                produkt.id = order_row.firstChildElement("product_id").firstChild().toCDATASection().nodeValue().toUInt();
                produkt.price = order_row.firstChildElement("product_price").firstChild().toCDATASection().nodeValue().toFloat();
                produkt.quantity = order_row.firstChildElement("product_quantity").firstChild().toCDATASection().nodeValue().toUInt();
                zam.order_rows << produkt;
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

QDomDocument Prestashop::toXML(const Order &zamowienie)
{
    QDomDocument doc = getPrestaXML();
    QDomElement order = doc.createElement("order");
    doc.firstChild().appendChild(order);

    if(zamowienie.id > 0)
        order.appendChild(buildXMLElement(doc, "id",  QString::number(zamowienie.id)));
    if(zamowienie.current_state > 0)
        order.appendChild(buildXMLElement(doc, "current_state", QString::number((uint)zamowienie.current_state)));

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

QDomDocument Prestashop::toXML(const SpecificPrice &specificPrice)
{
    QDomDocument doc = getPrestaXML();
    QDomElement specific_price = doc.createElement("specific_price");
    doc.firstChild().appendChild(specific_price);

    if(specificPrice.id > 0)
        specific_price.appendChild(buildXMLElement(doc, "id",  QString::number(specificPrice.id)));

    specific_price.appendChild(buildXMLElement(doc, "id_product", QString::number(specificPrice.id_product)));
    specific_price.appendChild(buildXMLElement(doc, "id_shop", QString::number(specificPrice.id_shop)));
    specific_price.appendChild(buildXMLElement(doc, "id_cart", QString::number(specificPrice.id_cart)));
    specific_price.appendChild(buildXMLElement(doc, "id_country", QString::number(specificPrice.id_country)));
    specific_price.appendChild(buildXMLElement(doc, "id_currency", QString::number(specificPrice.id_currency)));
    specific_price.appendChild(buildXMLElement(doc, "id_customer", QString::number(specificPrice.id_customer)));
    specific_price.appendChild(buildXMLElement(doc, "id_group", QString::number(specificPrice.id_group)));
    specific_price.appendChild(buildXMLElement(doc, "price", QString::number(specificPrice.price)));
    specific_price.appendChild(buildXMLElement(doc, "reduction", QString::number(specificPrice.reduction)));
    specific_price.appendChild(buildXMLElement(doc, "from_quantity", QString::number(specificPrice.from_quantity)));
    specific_price.appendChild(buildXMLElement(doc, "from", specificPrice.from));
    specific_price.appendChild(buildXMLElement(doc, "to", specificPrice.to));
    switch(specificPrice.reduction_type) {
    case SpecificPrice::PERCENTAGE: specific_price.appendChild(buildXMLElement(doc, "reduction_type", "percentage"));
    case SpecificPrice::AMOUNT: specific_price.appendChild(buildXMLElement(doc, "reduction_type", "amount"));
    }
    qDebug() << doc.toByteArray();
    return doc;
}

Order Prestashop::getOrder(uint id)
{
    try {
        PSWebService::Options opt;
        opt.resource = "orders";
        opt.id = id;
        QDomDocument doc = mPSWebService->syncGet(opt);
        Order order = getOrder(doc);
        return order;
    } catch (PSWebService::PrestaError e) {
        e.msg = "zamowienie(uint id)";
        throw e;
    } catch (PSWebService::OtherError e) {
        e.msg = "zamowienie(uint id)";
        throw e;
    }
}

QList<Order> Prestashop::getOrder(Order::Status status)
{
    try {
        PSWebService::Options opt;
        opt.resource = "orders";
        opt.filter["current_state"] = "[" + QString::number((uint)status)+ "]";
        QDomDocument doc = mPSWebService->syncGet(opt);

        QList<Order> zamowienia;

        QDomElement prestashop = doc.firstChildElement("prestashop");
        if(!prestashop.isNull()) {
            QDomNodeList orders = prestashop.firstChildElement("orders").elementsByTagName("order");
            for(int i=0; i<orders.size(); ++i) {
                uint id = orders.at(i).toElement().attribute("id").toUInt();
                zamowienia << getOrder(id);
            }
            return zamowienia;
        } else {
            throw QString("chujnia");
            // TODO sygnalizacja b³êdu
        }
    } catch (PSWebService::PrestaError e) {
        e.msg = "getZamowienie(Zamowienie::Status status)";
        throw e;
    } catch (PSWebService::OtherError e) {
        e.msg = "getZamowienie(Zamowienie::Status status)";
        throw e;
    }
}

QList<uint> Prestashop::getSpecificPrice(uint productId)
{
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    opt.filter["product_id"] = QString::number(productId);
    mPSWebService->syncGet(opt);
}

