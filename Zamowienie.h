#ifndef ZAMOWIENIE_H
#define ZAMOWIENIE_H

#include <QList>
#include <QString>

class Zamowienie
{
public:
    enum Status {
        BEZ_ZMIAN = 0,
        OCZEKIWANIE_NA_PLATNOSC = 1,
        PLATNOSC_ZAAKCEPTOWANA = 2,
        W_REALIZACJI = 3,
        WYSYLKA = 4,
        ODEBRANE = 5,
        REZYGNACJA = 6,
        ZWROT = 7,
        BRAK_PLATNOSCI = 8,
        NA_ZAMOWIENIE = 9,
        OCZEKIWANIE_NA_PRZELEW = 10,
        GOTOWE_DO_ODBIORU = 13
    };
    struct Produkt {
        uint id;
        uint ilosc;
        float cena;
    };

    Status status;
    uint id;
    QList<Zamowienie::Produkt> produkty;

    uint id_address_delivery;
    uint id_address_invoice;
    uint id_cart;
    uint id_currency;
    uint id_lang;
    uint id_customer;
    uint id_carrier;
    QString module;
    uint invoice_number;
    QString invoice_date;
    uint delivery_number;
    QString delivery_date;
    uint valid;
    QString date_add;
    QString date_upd;
    QString secure_key;
    QString payment;
    uint recyclable;
    uint gift;
    QString gift_message;
    float total_discounts;
    float total_paid;
    float total_paid_real;
    float total_products;
    float total_products_wt;
    float total_shipping;
    float carrier_tax_rate;
    float total_wrapping;
    uint shipping_number;
    float conversion_rate;

    Zamowienie();
};

#endif // ZAMOWIENIE_H
