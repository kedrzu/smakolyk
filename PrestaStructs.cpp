#include "PrestaStructs.h"
using namespace Presta;

Product::Product() :
    id(0),
    active(0),
    show_price(0),
    available_for_order(0),
    id_category_default(0),
    out_of_stock(2),
    //quantity(0),
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

Order::Order() :
    id(0),
    current_state(0),
    id_address_delivery(0),
    id_address_invoice(0),
    id_cart(0),
    id_currency(0),
    id_lang(0),
    id_customer(0),
    id_carrier(0),
    invoice_number(0),
    delivery_number(0),
    valid(0),
    id_shop_group(0),
    id_shop(0),
    recyclable(0),
    gift(0),
    shipping_number(0),
    total_discounts(0),
    total_paid(0),
    total_paid_real(0),
    total_products(0),
    total_products_wt(0),
    total_shipping(0),
    carrier_tax_rate(0),
    total_wrapping(0),
    conversion_rate(0),
    total_discounts_tax_incl(0),
    total_discounts_tax_excl(0),
    total_paid_tax_incl(0),
    total_paid_tax_excl(0),
    total_shipping_tax_incl(0),
    total_shipping_tax_excl(0),
    total_wrapping_tax_incl(0),
    total_wrapping_tax_excl(0)
{}

OrderHeader::OrderHeader() :
    id(0),
    current_state(0),
    total_paid(0)
{}

Category::Category() :
    id(0),
    idRef(0),
    active(1),
    id_parent(1)
{}

uint Category::getId(QDomDocument &doc) {
    QDomElement prestashop = doc.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement category = prestashop.firstChildElement("category");
        if(!category.isNull()) {
            return category.firstChildElement("id").toCDATASection().nodeValue().toUInt();
        } else return 0;
    } else return 0;
}

