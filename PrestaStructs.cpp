#include "PrestaStructs.h"
using namespace Presta;

Product::Product() :
    id(0),
    active(0),
    show_price(0),
    available_for_order(0),
    id_category_default(0),
    out_of_stock(2),
    quantity(0),
    price(0)
{}

unsigned Product::getId(QDomDocument &doc) {
    QDomElement prestashop = doc.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement product = prestashop.firstChildElement("product");
        if(!product.isNull()) {
            return product.firstChildElement("id").toCDATASection().nodeValue().toUInt();
        } else return 0;
    } else return 0;
}

SpecificPrice::SpecificPrice() :
    id(0),
    id_product(0),
    id_shop(0),
    id_cart(0),
    id_currency(0),
    id_country(0),
    id_group(0),
    id_customer(0),
    price(0),
    reduction(0),
    from_quantity(0),
    from("0000-00-00 00:00:00"),
    to("0000-00-00 00:00:00"),
    reduction_type(AMOUNT)
{}
