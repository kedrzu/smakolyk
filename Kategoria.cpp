#include "Kategoria.h"

unsigned Kategoria::getId(QDomDocument *doc) {
    QDomElement prestashop = doc->firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement category = prestashop.firstChildElement("category");
        if(!category.isNull()) {
            return category.firstChildElement("id").toCDATASection().nodeValue().toUInt();
        } else return 0;
    } else return 0;
}
