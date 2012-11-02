#ifndef STRUCTS_H
#define STRUCTS_H

#include <QString>
#include <QDomDocument>
#include <QList>

namespace Presta {

struct Product {

    uint id;
    uint idRef;
    uint active;
    uint show_price;
    uint available_for_order;
    uint id_category_default;
    uint out_of_stock;
    //uint quantity;
    float price;
    QString reference;
    QString name;
    QString ean;
    QString description;
    QString description_short;
    QString meta_description;
    QString meta_keywords;
    QString meta_title;
    QString link_rewrite;
    QString available_now;
    QString available_later;
    QList<uint> categories;

    Product();
    static unsigned getId(QDomDocument &doc);
};

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

struct Order
{
    struct OrderRow {
        uint id;
        uint quantity;
        float price;
    };

    uint id;
    uint current_state;
    uint id_address_delivery;
    uint id_address_invoice;
    uint id_cart;
    uint id_currency;
    uint id_lang;
    uint id_customer;
    uint id_carrier;
    uint invoice_number;
    uint delivery_number;
    uint valid;
    uint id_shop_group;
    uint id_shop;
    uint recyclable;
    uint gift;
    uint shipping_number;
    float total_discounts;
    float total_paid;
    float total_paid_real;
    float total_products;
    float total_products_wt;
    float total_shipping;
    float carrier_tax_rate;
    float total_wrapping;
    float conversion_rate;
    float total_discounts_tax_incl;
    float total_discounts_tax_excl;
    float total_paid_tax_incl;
    float total_paid_tax_excl;
    float total_shipping_tax_incl;
    float total_shipping_tax_excl;
    float total_wrapping_tax_incl;
    float total_wrapping_tax_excl;
    QString date_add;
    QString date_upd;
    QString secure_key;
    QString payment;
    QString delivery_date;
    QString module;
    QString gift_message;
    QString invoice_date;
    QString reference;
    QList<Order::OrderRow> order_rows;

    Order();
};

struct OrderHeader {
    uint id;
    uint current_state;
    float total_paid;
    QString reference;
    QString date_add;

    OrderHeader();
};

struct Category {
    uint id;
    uint idRef;
    uint active;
    uint id_parent;
    QString name;
    QString meta_description;
    QString meta_keywords;
    QString meta_title;
    QString link_rewrite;
    QString description;

    static uint getId(QDomDocument& doc);
    Category();
};

}

#endif // STRUCTS_H
