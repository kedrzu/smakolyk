#include <QtCore/QCoreApplication>
#include "Presta.h"
#include "KCFirma.h"
#include "Logger.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    PSWebService* webService = new PSWebService("http://localhost/PrestaShop.PL_1.4.4.1-stable/", "DFHHPRHFWBQ47F55373EUA25FB9QXEFL");
    KCFirma* kcfirma = new KCFirma();
    Presta* presta = new Presta(webService, kcfirma);
    Logger* logger = new Logger(presta);

    presta->upload();

//    Produkt produkt;
//    produkt.cena = 199.99;
//    produkt.idKC = 56780;
//    produkt.nazwa = "Jesiotr";
//    produkt.przyjaznyUrl = "jesiotr";
//    produkt.status = Produkt::KATALOG;
//    produkt.kategoria = 7;
//    produkt.id = 10;



//    //presta.edytuj(kategoria);
//    presta.upload();
//    presta.syncEdit(produkt);

return a.exec();
}
