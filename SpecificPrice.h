#ifndef SPECIFICPRICE_H
#define SPECIFICPRICE_H

#include <QString>

struct SpecificPrice
{
    enum ReductionType {
        PERCENTAGE, AMOUNT
    };

    unsigned id;
    unsigned id_product;
    unsigned id_shop;
    unsigned id_cart;
    unsigned id_currency;
    unsigned id_country;
    unsigned id_group;
    unsigned id_customer;
    float price;
    float reduction;
    unsigned from_quantity;
    QString from;
    QString to;
    ReductionType reduction_type;

    SpecificPrice();
};

#endif // SPECIFICPRICE_H
