#include <QApplication>
#include "KCPresta.h"
#include "KCFirma.h"
#include "Logger.h"
#include <iostream>
#include <QSettings>
#include "MainWindow.h"

#include <QDebug>

using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow main;
    main.show();

    //    for(int j=1; j<5; ++j) {
    //        try {
    //            Order zamowienie = presta->zamowienie(j);

    //            qDebug() << "zamowienie nr " << zamowienie.id;
    //            qDebug() << "data dodania " << zamowienie.dataDodania;
    //            qDebug() << "produkty:";
    //            for(int i=0; i<zamowienie.produkty.size(); ++i) {
    //                qDebug() << "id: " << zamowienie.produkty.at(i).id << " ilosc: " << zamowienie.produkty.at(i).ilosc << "cena :" << zamowienie.produkty.at(i).cena;
    //            }
    //        } catch (PSWebService::OtherError e) {
    //        }
    //    }

//    QList<Order> zamowienia = presta->getOrder(">=[1]");
//    for(int i=0; i<zamowienia.size(); ++i) {
//        const Order& zamowienie = zamowienia.at(i);
//        qDebug() << "";
//        qDebug() << "zamowienie nr " << zamowienie.id;
//        qDebug() << "data dodania " << zamowienie.date_add;
//        qDebug() << "produkty:";
//        for(int i=0; i<zamowienie.produkty.size(); ++i) {
//            qDebug() << " - id: " << zamowienie.produkty.at(i).id << " ilosc: " << zamowienie.produkty.at(i).ilosc << "cena :" << zamowienie.produkty.at(i).cena;
//        }
//    }

    //    Order zamowienie = presta->getOrder(2);
    //    zamowienie.total_products = 1000;
    //    zamowienie.gift = 1;
    //    zamowienie.status = Order::BEZ_ZMIAN;
    //    try {
    //        presta->syncEdit(zamowienie);
    //    } catch (PSWebService::PrestaError e) {
    //        logger->logError(e);
    //    } catch (PSWebService::OtherError e) {
    //        logger->logError(e);
    //    }

    return a.exec();
}
