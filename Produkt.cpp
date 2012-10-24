#include "Produkt.h"

unsigned Produkt::getId(QDomDocument *doc) {
    QDomElement prestashop = doc->firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement product = prestashop.firstChildElement("product");
        if(!product.isNull()) {
            return product.firstChildElement("id").toCDATASection().nodeValue().toUInt();
        } else return 0;
    } else return 0;
}

float Produkt::getCena(QDomDocument *doc) {
    QDomElement prestashop = doc->firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement product = prestashop.firstChildElement("product");
        if(!product.isNull()) {
            return product.firstChildElement("price").toCDATASection().nodeValue().toFloat();
        } else return 0;
    } else return 0;
}
