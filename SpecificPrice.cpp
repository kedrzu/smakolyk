#include "SpecificPrice.h"

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
{
}
