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
    KCFirma* kcfirma = new KCFirma(config);
    Presta* presta = new Presta(config, webService, kcfirma);
    Logger* logger = new Logger(presta);

    //    for(int j=1; j<5; ++j) {
    //        try {
    //            Zamowienie zamowienie = presta->zamowienie(j);

    //            qDebug() << "zamowienie nr " << zamowienie.id;
    //            qDebug() << "data dodania " << zamowienie.dataDodania;
    //            qDebug() << "produkty:";
    //            for(int i=0; i<zamowienie.produkty.size(); ++i) {
    //                qDebug() << "id: " << zamowienie.produkty.at(i).id << " ilosc: " << zamowienie.produkty.at(i).ilosc << "cena :" << zamowienie.produkty.at(i).cena;
    //            }
    //        } catch (PSWebService::OtherError e) {
    //        }
    //    }

    QList<Zamowienie> zamowienia = presta->zamowienie(">=[1]");
    for(int i=0; i<zamowienia.size(); ++i) {
        const Zamowienie& zamowienie = zamowienia.at(i);
        qDebug() << "";
        qDebug() << "zamowienie nr " << zamowienie.id;
        qDebug() << "data dodania " << zamowienie.date_add;
        qDebug() << "produkty:";
        for(int i=0; i<zamowienie.produkty.size(); ++i) {
            qDebug() << " - id: " << zamowienie.produkty.at(i).id << " ilosc: " << zamowienie.produkty.at(i).ilosc << "cena :" << zamowienie.produkty.at(i).cena;
        }
    }

    Zamowienie zamowienie = presta->zamowienie(2);
    zamowienie.total_products = 1000;
    zamowienie.gift = 1;
    zamowienie.status = Zamowienie::BEZ_ZMIAN;
    try {
        presta->syncEdit(zamowienie);
    } catch (PSWebService::PrestaError e) {
        logger->logError(e);
    } catch (PSWebService::OtherError e) {
        logger->logError(e);
    }


    return a.exec();
}
