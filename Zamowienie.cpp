#include "Zamowienie.h"

Zamowienie::Zamowienie() :
    status(BEZ_ZMIAN),
    id(0),
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
    recyclable(0),
    gift(0),
    total_discounts(0),
    total_paid(0),
    total_paid_real(0),
    total_products(0),
    total_products_wt(0),
    total_shipping(0),
    carrier_tax_rate(0),
    total_wrapping(0),
    shipping_number(0),
    conversion_rate(0)

{
}
