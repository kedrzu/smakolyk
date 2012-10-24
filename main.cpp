#include <QtCore/QCoreApplication>
#include "Presta.h"
#include "KCFirma.h"
#include "Logger.h"
#include <iostream>

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Config config;
    config.url = "http://localhost/PrestaShop.PL_1.4.4.1-stable/";
    config.key = "DFHHPRHFWBQ47F55373EUA25FB9QXEFL";
    config.lang = 6;

    PSWebService* webService = new PSWebService(config);
    KCFirma* kcfirma = new KCFirma();
    Presta* presta = new Presta(config, webService, kcfirma);
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
