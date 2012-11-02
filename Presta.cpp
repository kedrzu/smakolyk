#include "Presta.h"
#include <QMap>

#include <QDebug>
#include <Qt>

using namespace Presta;

Prestashop::Prestashop(const QSettings& settings, PSWebService *pswebService, QObject *parent) :
    QObject(parent),
    mPSWebService(pswebService)
{
    mLangId = settings.value("prestashop/lang").toUInt();
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
    //productElem.appendChild(buildXMLElement(doc, "quantity", QString::number(product.quantity)));
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

QNetworkReply *Prestashop::edit(const Product &product)
{
    StackTraceBegin();
    QDomDocument doc = toXML(product);
    //qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.id = product.id;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("idRef", QVariant(product.idRef));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::edit(const Product &product)");
}

QNetworkReply *Prestashop::add(const Product &product)
{
    StackTraceBegin();
    QDomDocument doc = toXML(product);
    //qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.resource = "products";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("idRef", QVariant(product.idRef));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::add(const Product &product)");
}

void Prestashop::syncEdit(const Product &product)
{
    StackTraceBegin();
    QDomDocument doc = toXML(product);
    //qDebug() << doc.toByteArray();
    PSWebService::Options opt;
    opt.id = product.id;
    opt.resource = "products";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
    StackTraceEnd("void Prestashop::syncEdit(const Product &product)");
}

unsigned Prestashop::syncAdd(const Product &product)
{
    StackTraceBegin();
    QDomDocument doc = toXML(product);
    PSWebService::Options opt;
    opt.resource = "products";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
    unsigned id = Product::getId(result);
    return id;
    StackTraceEnd("unsigned Prestashop::syncAdd(const Product &product)");
}

QDomDocument Prestashop::toXML(const Category &category)
{
    QDomDocument doc = getPrestaXML();
    QDomElement categoryElem = doc.createElement("category");
    doc.firstChild().appendChild(categoryElem);

    if(category.id > 0)
        categoryElem.appendChild(buildXMLElement(doc, "id",  QString::number(category.id)));
    if(category.id_parent > 0)
        categoryElem.appendChild(buildXMLElement(doc, "id_parent",  QString::number(category.id_parent)));
    categoryElem.appendChild(buildXMLElement(doc, "active", QString::number(category.active)));
    categoryElem.appendChild(buildXMLElement(doc, "name", category.name, mLangId));
    categoryElem.appendChild(buildXMLElement(doc, "meta_description", category.meta_description, mLangId));
    categoryElem.appendChild(buildXMLElement(doc, "meta_keywords", category.meta_keywords, mLangId));
    categoryElem.appendChild(buildXMLElement(doc, "meta_title", category.meta_title, mLangId));
    categoryElem.appendChild(buildXMLElement(doc, "link_rewrite", category.link_rewrite, mLangId));
    categoryElem.appendChild(buildXMLElement(doc, "description", category.description, mLangId));
    return doc;
}

QNetworkReply *Prestashop::edit(const Category &kategoria)
{
    StackTraceBegin();
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("idRef", QVariant(kategoria.idRef));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::edit(const Category &kategoria)");
}

QNetworkReply *Prestashop::add(const Category &kategoria)
{
    StackTraceBegin();
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.resource = "categories";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("idRef", QVariant(kategoria.idRef));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::add(const Category &kategoria)");
}

QNetworkReply *Prestashop::edit(const SpecificPrice &specificPrice)
{
    StackTraceBegin();
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.id = specificPrice.id;
    opt.resource = "specific_prices";
    QNetworkReply* reply = mPSWebService->put(opt, doc);
    reply->setProperty("id_product", QVariant(specificPrice.id_product));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::edit(const SpecificPrice &specificPrice)");
}

QNetworkReply *Prestashop::add(const SpecificPrice &specificPrice)
{
    StackTraceBegin();
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    QNetworkReply* reply = mPSWebService->post(opt, doc);
    reply->setProperty("id_product", QVariant(specificPrice.id_product));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::add(const SpecificPrice &specificPrice)");
}


QNetworkReply *Prestashop::deleteProduct(uint id)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "products";
    opt.id = id;
    QNetworkReply* reply = mPSWebService->del(opt);
    reply->setProperty("id", QVariant(id));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::deleteProduct(uint id)");
}

QNetworkReply *Prestashop::deleteCategory(uint id)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "categories";
    opt.id = id;
    QNetworkReply* reply = mPSWebService->del(opt);
    reply->setProperty("id", QVariant(id));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::deleteCategory(uint id)");
}

QNetworkReply *Prestashop::deleteSpecificPrice(uint id)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    opt.id = id;
    QNetworkReply* reply = mPSWebService->del(opt);
    reply->setProperty("id", QVariant(id));
    return reply;
    StackTraceEnd("QNetworkReply *Prestashop::deleteSpecificPrice(uint id)");
}

void Prestashop::syncDeleteProduct(uint id)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "products";
    opt.id = id;
    mPSWebService->syncDel(opt);
    StackTraceEnd("QNetworkReply *Prestashop::syncDeleteProduct(uint id)");
}

void Prestashop::syncDeleteCategory(uint id)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "categories";
    opt.id = id;
    mPSWebService->syncDel(opt);
    StackTraceEnd("QNetworkReply *Prestashop::syncDeleteCategory(uint id)");
}

void Prestashop::syncDeleteSpecificPrice(uint id)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    opt.id = id;
    mPSWebService->syncDel(opt);
    StackTraceEnd("QNetworkReply *Prestashop::syncDeleteSpecificPrice(uint id)");
}

void Prestashop::syncEdit(const Category &kategoria)
{
    StackTraceBegin();
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.id = kategoria.id;
    opt.resource = "categories";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
    StackTraceEnd("void Prestashop::syncEdit(const Category &kategoria)");
}

void Prestashop::syncEdit(const Order &zamowienie)
{
    StackTraceBegin();
    QDomDocument doc = toXML(zamowienie);
    PSWebService::Options opt;
    opt.id = zamowienie.id;
    opt.resource = "orders";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
    StackTraceEnd("void Prestashop::syncEdit(const Order &zamowienie)");
}

void Prestashop::syncEdit(const SpecificPrice &specificPrice)
{
    StackTraceBegin();
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.id = specificPrice.id;
    opt.resource = "specific_prices";
    QDomDocument result = mPSWebService->syncPut(opt, doc);
    StackTraceEnd("void Prestashop::syncEdit(const SpecificPrice &specificPrice)");
}

unsigned Prestashop::syncAdd(const Category &kategoria)
{
    StackTraceBegin();
    QDomDocument doc = toXML(kategoria);
    PSWebService::Options opt;
    opt.resource = "categories";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
    unsigned id = Category::getId(result);
    return id;
    StackTraceEnd("unsigned Prestashop::syncAdd(const Category &kategoria)");
}

void Prestashop::syncAdd(const SpecificPrice &specificPrice)
{
    StackTraceBegin();
    QDomDocument doc = toXML(specificPrice);
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    QDomDocument result = mPSWebService->syncPost(opt, doc);
    StackTraceEnd("void Prestashop::syncAdd(const SpecificPrice &specificPrice)");
}

Order Prestashop::getOrder(QDomDocument &doc) const {
    QDomElement prestashop = doc.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement orderElem = prestashop.firstChildElement("order");
        if(!orderElem.isNull()) {
            Order order;
            order.id = orderElem.firstChildElement("id").firstChild().toCDATASection().nodeValue().toUInt();
            order.current_state = orderElem.firstChildElement("current_state").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_address_delivery = orderElem.firstChildElement("id_address_delivery").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_address_invoice = orderElem.firstChildElement("id_address_invoice").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_cart = orderElem.firstChildElement("id_cart").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_currency = orderElem.firstChildElement("id_currency").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_lang = orderElem.firstChildElement("id_lang").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_customer = orderElem.firstChildElement("id_customer").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_carrier = orderElem.firstChildElement("id_carrier").firstChild().toCDATASection().nodeValue().toUInt();
            order.module = orderElem.firstChildElement("module").firstChild().toCDATASection().nodeValue();
            order.invoice_number = orderElem.firstChildElement("invoice_number").firstChild().toCDATASection().nodeValue().toUInt();
            order.invoice_date = orderElem.firstChildElement("invoice_date").firstChild().toCDATASection().nodeValue();
            order.delivery_number = orderElem.firstChildElement("delivery_number").firstChild().toCDATASection().nodeValue().toUInt();
            order.delivery_date = orderElem.firstChildElement("delivery_date").firstChild().toCDATASection().nodeValue();
            order.valid = orderElem.firstChildElement("valid").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_shop_group = orderElem.firstChildElement("id_shop_group").firstChild().toCDATASection().nodeValue().toUInt();
            order.id_shop = orderElem.firstChildElement("id_shop").firstChild().toCDATASection().nodeValue().toUInt();
            order.date_add = orderElem.firstChildElement("date_add").firstChild().toCDATASection().nodeValue();
            order.date_upd = orderElem.firstChildElement("date_upd").firstChild().toCDATASection().nodeValue();
            order.secure_key = orderElem.firstChildElement("secure_key").firstChild().toCDATASection().nodeValue();
            order.payment = orderElem.firstChildElement("payment").firstChild().toCDATASection().nodeValue();
            order.recyclable = orderElem.firstChildElement("recyclable").firstChild().toCDATASection().nodeValue().toUInt();
            order.gift = orderElem.firstChildElement("gift").firstChild().toCDATASection().nodeValue().toUInt();
            order.gift_message = orderElem.firstChildElement("gift_message").firstChild().toCDATASection().nodeValue();
            order.reference = orderElem.firstChildElement("reference").firstChild().toCDATASection().nodeValue();
            order.total_discounts = orderElem.firstChildElement("total_discounts").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_paid = orderElem.firstChildElement("total_paid").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_paid_real = orderElem.firstChildElement("total_paid_real").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_products = orderElem.firstChildElement("total_products").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_products_wt = orderElem.firstChildElement("total_products_wt").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_shipping = orderElem.firstChildElement("total_shipping").firstChild().toCDATASection().nodeValue().toFloat();
            order.carrier_tax_rate = orderElem.firstChildElement("carrier_tax_rate").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_wrapping = orderElem.firstChildElement("carrier_tax_rate").firstChild().toCDATASection().nodeValue().toFloat();
            order.shipping_number = orderElem.firstChildElement("shipping_number").firstChild().toCDATASection().nodeValue().toUInt();
            order.conversion_rate = orderElem.firstChildElement("conversion_rate").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_discounts_tax_incl = orderElem.firstChildElement("total_discounts_tax_incl").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_discounts_tax_excl = orderElem.firstChildElement("total_discounts_tax_excl").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_paid_tax_incl = orderElem.firstChildElement("total_paid_tax_incl").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_paid_tax_excl = orderElem.firstChildElement("total_paid_tax_excl").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_shipping_tax_incl = orderElem.firstChildElement("total_shipping_tax_incl").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_shipping_tax_excl = orderElem.firstChildElement("total_shipping_tax_excl").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_wrapping_tax_incl = orderElem.firstChildElement("total_wrapping_tax_incl").firstChild().toCDATASection().nodeValue().toFloat();
            order.total_wrapping_tax_excl = orderElem.firstChildElement("total_wrapping_tax_excl").firstChild().toCDATASection().nodeValue().toFloat();

            // produkty
            QDomElement associations = orderElem.firstChildElement("associations");
            QDomNodeList order_rows = associations.firstChildElement("order_rows").elementsByTagName("order_row");
            for(int i=0; i<order_rows.size(); ++i) {
                QDomElement order_rowElem = order_rows.at(i).toElement();
                Order::OrderRow order_row;
                order_row.id = order_rowElem.firstChildElement("product_id").firstChild().toCDATASection().nodeValue().toUInt();
                order_row.price = order_rowElem.firstChildElement("product_price").firstChild().toCDATASection().nodeValue().toFloat();
                order_row.quantity = order_rowElem.firstChildElement("product_quantity").firstChild().toCDATASection().nodeValue().toUInt();
                order.order_rows << order_row;
            }

            // zwracamy zamówienie
            return order;
        } else {
            throw QString("chujnia");
        }
    } else {
        throw QString("chujnia");
    }
}

QDomDocument Prestashop::toXML(const Order &order)
{
    QDomDocument doc = getPrestaXML();
    QDomElement orderElem = doc.createElement("order");
    doc.firstChild().appendChild(orderElem);

    if(order.id > 0)
        orderElem.appendChild(buildXMLElement(doc, "id",  QString::number(order.id)));
    if(order.current_state > 0)
        orderElem.appendChild(buildXMLElement(doc, "current_state", QString::number((uint)order.current_state)));

    orderElem.appendChild(buildXMLElement(doc, "id_address_delivery", QString::number(order.id_address_delivery)));
    orderElem.appendChild(buildXMLElement(doc, "id_address_invoice", QString::number(order.id_address_invoice)));
    orderElem.appendChild(buildXMLElement(doc, "id_cart", QString::number(order.id_cart)));
    orderElem.appendChild(buildXMLElement(doc, "id_currency", QString::number(order.id_currency)));
    orderElem.appendChild(buildXMLElement(doc, "id_lang", QString::number(order.id_lang)));
    orderElem.appendChild(buildXMLElement(doc, "id_customer", QString::number(order.id_customer)));
    orderElem.appendChild(buildXMLElement(doc, "id_carrier", QString::number(order.id_carrier)));
    orderElem.appendChild(buildXMLElement(doc, "module", order.module));
    orderElem.appendChild(buildXMLElement(doc, "invoice_number", QString::number(order.invoice_number)));
    orderElem.appendChild(buildXMLElement(doc, "invoice_date", order.invoice_date));
    orderElem.appendChild(buildXMLElement(doc, "delivery_number", QString::number(order.delivery_number)));
    orderElem.appendChild(buildXMLElement(doc, "delivery_date", order.delivery_date));
    orderElem.appendChild(buildXMLElement(doc, "valid", QString::number(order.valid)));
    orderElem.appendChild(buildXMLElement(doc, "id_shop_group", QString::number(order.id_shop_group)));
    orderElem.appendChild(buildXMLElement(doc, "id_shop", QString::number(order.id_shop)));
    orderElem.appendChild(buildXMLElement(doc, "date_add", order.date_add));
    orderElem.appendChild(buildXMLElement(doc, "date_upd", order.date_upd));
    orderElem.appendChild(buildXMLElement(doc, "secure_key", order.secure_key));
    orderElem.appendChild(buildXMLElement(doc, "payment", order.payment));
    orderElem.appendChild(buildXMLElement(doc, "recyclable", QString::number(order.recyclable)));
    orderElem.appendChild(buildXMLElement(doc, "gift", QString::number(order.gift)));
    orderElem.appendChild(buildXMLElement(doc, "gift_message", order.gift_message));
    orderElem.appendChild(buildXMLElement(doc, "reference", order.reference));
    orderElem.appendChild(buildXMLElement(doc, "total_discounts", QString::number(order.total_discounts)));
    orderElem.appendChild(buildXMLElement(doc, "total_paid", QString::number(order.total_paid)));
    orderElem.appendChild(buildXMLElement(doc, "total_paid_real", QString::number(order.total_paid_real)));
    orderElem.appendChild(buildXMLElement(doc, "total_products", QString::number(order.total_products)));
    orderElem.appendChild(buildXMLElement(doc, "total_products_wt", QString::number(order.total_products_wt)));
    orderElem.appendChild(buildXMLElement(doc, "total_shipping", QString::number(order.total_shipping)));
    orderElem.appendChild(buildXMLElement(doc, "carrier_tax_rate", QString::number(order.carrier_tax_rate)));
    orderElem.appendChild(buildXMLElement(doc, "total_wrapping", QString::number(order.total_wrapping)));
    orderElem.appendChild(buildXMLElement(doc, "shipping_number", QString::number(order.shipping_number)));
    orderElem.appendChild(buildXMLElement(doc, "conversion_rate", QString::number(order.conversion_rate)));
    orderElem.appendChild(buildXMLElement(doc, "total_discounts_tax_incl", QString::number(order.total_discounts_tax_incl)));
    orderElem.appendChild(buildXMLElement(doc, "total_discounts_tax_excl", QString::number(order.total_discounts_tax_excl)));
    orderElem.appendChild(buildXMLElement(doc, "total_paid_tax_incl", QString::number(order.total_paid_tax_incl)));
    orderElem.appendChild(buildXMLElement(doc, "total_paid_tax_excl", QString::number(order.total_paid_tax_excl)));
    orderElem.appendChild(buildXMLElement(doc, "total_shipping_tax_incl", QString::number(order.total_shipping_tax_incl)));
    orderElem.appendChild(buildXMLElement(doc, "total_shipping_tax_excl", QString::number(order.total_shipping_tax_excl)));
    orderElem.appendChild(buildXMLElement(doc, "total_wrapping_tax_incl", QString::number(order.conversion_rate)));
    orderElem.appendChild(buildXMLElement(doc, "total_wrapping_tax_excl", QString::number(order.conversion_rate)));

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
    //qDebug() << doc.toByteArray();
    return doc;
}

Order Prestashop::getOrder(uint id)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "orders";
    opt.id = id;
    QDomDocument doc = mPSWebService->syncGet(opt);
    Order order = getOrder(doc);
    return order;
    StackTraceEnd("Order Prestashop::getOrder(uint id)");
}

QList<OrderHeader> Prestashop::getOrderHeader(const QMap<QString, QString> &filter)
{
    StackTraceBegin();
    PSWebService::Options opt;
    opt.resource = "orders";
    opt.display = "[id,current_state,total_paid,reference,date_add]";
    QMapIterator<QString, QString> it(filter);
    while(it.hasNext()) {
        it.next();
        opt.filter[it.key()] = it.value();
    }
    QDomDocument doc = mPSWebService->syncGet(opt);

    QList<OrderHeader> orders;

    QDomElement prestashop = doc.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomNodeList orderNodeList = prestashop.firstChildElement("orders").elementsByTagName("order");
        for(int i=0; i<orderNodeList.size(); ++i) {
            OrderHeader orderHeader;
            orderHeader.id = orderNodeList.at(i).firstChildElement("id").firstChild().toCDATASection().nodeValue().toUInt();
            orderHeader.current_state = orderNodeList.at(i).firstChildElement("current_state").firstChild().toCDATASection().nodeValue().toUInt();
            orderHeader.total_paid = orderNodeList.at(i).firstChildElement("total_paid").firstChild().toCDATASection().nodeValue().toFloat();
            orderHeader.reference = orderNodeList.at(i).firstChildElement("reference").firstChild().toCDATASection().nodeValue();
            orderHeader.date_add = orderNodeList.at(i).firstChildElement("date_add").firstChild().toCDATASection().nodeValue();
            orders << orderHeader;
        }
        return orders;
    } else {
        throw QString("chujnia");
        // TODO sygnalizacja błędu
    }
    StackTraceEnd("QList<OrderHeader> Prestashop::getOrderHeader(const QMap<QString, QString> &filter)");
}

QList<uint> Prestashop::getSpecificPrice(uint productId)
{
    PSWebService::Options opt;
    opt.resource = "specific_prices";
    opt.filter["product_id"] = QString::number(productId);
    QDomDocument doc = mPSWebService->syncGet(opt);
    QList<uint> result;
    QDomElement prestashop = doc.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomNodeList specific_prices = prestashop.firstChildElement("specific_prices").elementsByTagName("specific_price");
        for(int i=0; i<specific_prices.size(); ++i) {
            result << specific_prices.at(i).toElement().attribute("id").toUInt();
        }
    }
    return result;
}

