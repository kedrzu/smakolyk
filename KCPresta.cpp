#include "KCPresta.h"
using namespace Presta;

KCPresta::KCPresta(const QSettings& settings, Prestashop *presta, KCFirma *kcFirma, QObject *parent) :
    mPresta(presta),
    mKCFirma(kcFirma),
    mPSWebService(presta->getWebService()),
    mUploadFinished(true),
    mProduktyUpload(settings.value("KC-Presta/produkty_upload").toUInt()),
    mConfigKatRootSprzedaz(settings.value("kategorie/root_sprzedaz", "1").toUInt()),
    mConfigKatRootKatalog(settings.value("kategorie/root_sprzedaz", "2").toUInt())
{
    mStatusyZamowien[OCZEKUJE] = settings.value("zamowienia/oczekuje").toUInt();
    mStatusyZamowien[W_REALIZACJI] = settings.value("zamowienia/w_realizacji").toUInt();
    mStatusyZamowien[DO_ODBIORU] = settings.value("zamowienia/do_odbioru").toUInt();
    mStatusyZamowien[WYSLANE] = settings.value("zamowienia/wyslane").toUInt();
    mStatusyZamowien[ZREALIZOWANE] = settings.value("zamowienia/zrealizowane").toUInt();
    mStatusyZamowien[ANULOWANE] = settings.value("zamowienia/anulowane").toUInt();
}

void KCPresta::aktualizujKategorie()
{
    StackTraceBegin();
    // najpierw ściągamy kategorie z internetu
    emit debug("KCPresta::aktualizujKategorie()");
    PSWebService::Options opt;
    opt.resource = "categories";
    opt.display = "[id,id_parent,active,name,link_rewrite,meta_title,meta_description,meta_keywords,description]";
    QDomDocument result = mPSWebService->syncGet(opt);
    QDomElement prestashop = result.firstChildElement("prestashop");
    if(!prestashop.isNull()) {
        QDomElement categories = prestashop.firstChildElement("categories");
        if(!categories.isNull()) {
            // czyścimy starą mapę
            mKatNadrzedne.clear();
            // pobieramy kategorie
            QDomNodeList cats = categories.elementsByTagName("category");
            for(int i=0; i<cats.size(); ++i) {
                QDomNode cat = cats.at(i);
                if(!cat.isNull() && cat.hasChildNodes()) {
                    unsigned id = cat.firstChildElement("id").firstChild().toCDATASection().nodeValue().toUInt();
                    unsigned nadrzedna = cat.firstChildElement("id_parent").firstChild().toCDATASection().nodeValue().toUInt();
                    mKatNadrzedne[id] = nadrzedna;
                }
            }
        } else {
            // TODO łapanie błedów
        }
    }
    emit debug(QString::fromUtf8("Pobrano ").append(QString::number(mKatNadrzedne.size())).append(" kategorii."));
    StackTraceEnd("void KCPresta::aktualizujKategorie()");
}

bool KCPresta::dodajProdukty()
{
    emit debug("bool KCPresta::dodajProdukty()");
    return mKCFirma->produkty(mProdukty, mProduktyUpload);
}

bool KCPresta::dodajProdukty(uint ile)
{
    emit debug("bool KCPresta::dodajProdukty(uint ile)");
    return mKCFirma->produkty(mProdukty, ile);
}

SpecificPrice KCPresta::getSpecificPrice(const Produkt &produkt)
{
    SpecificPrice sp;
    sp.id_product = produkt.id;
    sp.reduction_type = SpecificPrice::AMOUNT;
    sp.price = produkt.cenaKC;
    // TODO zrobić sygnalizację błędu jeżeli cenaPresta > cenaKC
    sp.reduction = produkt.cenaKC - produkt.cenaPresta;
    return sp;
}

void KCPresta::uploadProdukty() {

    // ustawiamy flagę
    mUploadFinished = false;
    emit debug("void KCPresta::upload() {");

    // kasujemy bufor błędnie uploadowanych produktów
    mProduktyError.clear();

    if(mProdukty.empty()) {
        mUploadFinished = true;
        emit uploadFinished();
        emit debug("emit KCPresta::uploadFinished()");
        return;
    }

    foreach(const Produkt& produkt, mProdukty) {
        // jeżeli wystąpi błąd przy dodawaniu kategorii, to nie dodajemy produktu
        bool err = false;
        Produkt prod = produkt;
        // kategoria produktu jest nowa, lub była w Presta ale już jej nie ma => dodajemy do Presta
        bool dodajKatSprz = (prod.kategoria == 0 || !mKatNadrzedne.contains(prod.kategoria)) && prod.status != Produkt::KATALOG;
        bool dodajKatKata = prod.kategoriaKatalog == 0 || !mKatNadrzedne.contains(prod.kategoriaKatalog);
        if(dodajKatSprz || dodajKatKata) {
            Kategoria kat = mKCFirma->kategoria(prod.kategoriaKC);
            kat.id = 0;
            // dodajemy kategorię, jeżeli sie nie uda emitujemy blad i kontynujemy z nastepnym produktem
            uint idSprz = 0;
            try {
                // kategoria sprzedaży dodawana jest synchronicznie
                Category categorySprzedaz = kc2presta(kat);
                categorySprzedaz.id_parent = mConfigKatRootSprzedaz;
                idSprz = mPresta->syncAdd(categorySprzedaz);
                mKatNadrzedne[idSprz] = categorySprzedaz.id_parent;
            } catch (Exception& e) {
                StackTrace(e, "void KCPresta::uploadProdukty()");
                emit error(e);
                err = true;
            }
            uint idKat = 0;
            if(!err) {
                try {
                    // kategoria katalogu dodawana jest synchronicznie
                    Category categoryKatalog = kc2presta(kat);
                    categoryKatalog.id_parent = mConfigKatRootKatalog;
                    idKat = mPresta->syncAdd(categoryKatalog);
                    mKatNadrzedne[idKat] = categoryKatalog.id_parent;
                } catch (Exception& e) {
                    StackTrace(e, "void KCPresta::uploadProdukty()");
                    emit error(e);
                    err = true;
                }
            }
            // TODO Przydałoby sie zrobić kasowanie kategorii, jeżeli nie udało się poprawnie wysłać obu
            if(!err) {
                // aktualizujemy powiązanie
                mKCFirma->zmianaKategorii(idSprz, idKat, prod.kategoriaKC);
            }
        }
        // jeżeli produkt ma już prawidłową kategorię, to dodajemy
        if(!err) {
            // dodajemy produkt
            Presta::Product product = kc2presta(prod);
            // jeżeli produkt jest w bazie => edytujemy
            if(prod.id > 0) {
                QNetworkReply* reply = mPresta->edit(product);
                if(reply->isFinished()) {
                    productEdited(reply);
                } else {
                    connect(reply, SIGNAL(finished()), this, SLOT(productEdited()));
                }
            }
            // jeżeli nie, dodajemy
            else {
                QNetworkReply* reply = mPresta->add(product);
                if(reply->isFinished()) {
                    productAdded(reply);
                } else {
                    connect(reply, SIGNAL(finished()), this, SLOT(productAdded()));
                }
            }

        }
        // w przeciwnym wypadku oznaczamy produkt jako błędnie uploadowany
        else {
            mProduktyError[prod.idKC] = KATEGORIA;
        }
    }
}

void KCPresta::categoryAdded()
{

}

void KCPresta::categoryEdited()
{

}

void KCPresta::productAdded()
{
    productAdded((QNetworkReply*)QObject::sender());
}

void KCPresta::productEdited()
{
    productEdited((QNetworkReply*)QObject::sender());
}

void KCPresta::specificPriceEdited()
{

}

void KCPresta::specificPriceAdded()
{

}


void KCPresta::categoryAdded(QNetworkReply *reply)
{

}

void KCPresta::categoryEdited(QNetworkReply *reply)
{

}

void KCPresta::productAdded(QNetworkReply *reply)
{
    unsigned idKC = reply->property("idRef").toUInt();
    try {
        QDomDocument doc = PSWebService::readReply(reply);
        unsigned id = Product::getId(doc);
        // uploadujemy cenę specjalną produktu
        const Produkt& prod = mProdukty.value(idKC);
        Presta::SpecificPrice sp = getSpecificPrice(prod);
        mPresta->syncAdd(sp);
        try {
            mKCFirma->zmianaProduktu(id, idKC, prod.cenaPresta);
            mProdukty.remove(idKC);
        } catch (Exception& e) {
            // jeżeli został zwrócony jakiś inny wyjątek, np z bazy danych, to należy produkt usunąć z presta
            mPresta->syncDeleteProduct(id);
            throw;
        }
    } catch (Exception& e) {
        // jeżeli został zwrócony jakiś inny wyjątek, np z bazy danych, to należy produkt usunąć z presta
        StackTrace(e, "void KCPresta::productAdded(QNetworkReply *reply)");
        mProduktyError[idKC] = ADD_ERROR;
        emit error(e);
    }
    reply->deleteLater();

    // jeżeli wszystkie operacje zostały wykonane emitujemy odpowiedni sygnał
    checkFinished();
}

void KCPresta::productEdited(QNetworkReply *reply)
{
    unsigned idKC = reply->property("idRef").toUInt();
    try {
        QDomDocument doc = PSWebService::readReply(reply);
        unsigned id = Product::getId(doc);
        // uploadujemy cenę specjalną produktu
        const Produkt& prod = mProdukty.value(idKC);
        Presta::SpecificPrice sp = getSpecificPrice(prod);
        QList<uint> spList = mPresta->getSpecificPrice(id);
        // jeżeli produkt ma już zapisaną cenę specjalną to należy ją edytować
        if(spList.size() > 0) {
            sp.id  = spList.at(0);
            mPresta->syncEdit(sp);
            for(int i=1; i<spList.size(); ++i) {
                // TODO usuwanie niepotrzebnych
            }
        } else {
            mPresta->syncAdd(sp);
        }
        mKCFirma->zmianaProduktu(id, idKC, prod.cenaPresta);
        mProdukty.remove(idKC);
    } catch (Exception& e) {
        StackTrace(e, "void KCPresta::productEdited(QNetworkReply *reply)");
        mProduktyError[idKC] = EDIT_ERROR;
        emit error(e);
    }
    reply->deleteLater();

    // jeżeli wszystkie operacje zostały wykonane emitujemy odpowiedni sygnał
    checkFinished();
}

void KCPresta::specificPriceEdited(QNetworkReply *reply)
{

}

void KCPresta::specificPriceAdded(QNetworkReply *reply)
{

}

void KCPresta::checkFinished() {
    if(mProduktyError.size() == mProdukty.size()) {
        mUploadFinished = true;
        emit uploadFinished();
    }
}

Presta::Product KCPresta::kc2presta(const Produkt &produkt)
{
    Presta::Product product;
    product.id = produkt.id;
    product.idRef = produkt.idKC;
    product.active = produkt.status > 0 ? 1 : 0;
    product.show_price = produkt.status > 1 ? 1 : 0;
    product.available_for_order = produkt.status > 2 ? 1 : 0;
    product.id_category_default = produkt.kategoria;
    product.price = produkt.cenaKC;
    product.name = produkt.nazwa;
    product.ean = produkt.ean;
    product.description = produkt.opis;
    product.description_short = produkt.krotkiOpis;
    product.reference = QString::number(produkt.idKC);
    while(product.reference.length() < 11) {
        product.reference.prepend("0");
    }
    product.link_rewrite = produkt.nazwa;
    product.link_rewrite = product.link_rewrite.replace(" ", "-").replace(".", "").replace(",", "_").toLower();
    qDebug()<<product.link_rewrite;
    // dodajemy kategorie, do których należy produkt
    if(produkt.kategoriaKatalog > 0 && mKatNadrzedne.contains(produkt.kategoriaKatalog)) {
        product.id_category_default = produkt.kategoriaKatalog;
        unsigned cat = produkt.kategoriaKatalog;
        unsigned i = 0;
        do {
            i++;
            product.categories << cat;

            if(mKatNadrzedne.contains(cat)) {
                cat = mKatNadrzedne[cat];
            } else {
                break;
            }

            if(i>100) throw std::string("chujnia!");
        } while(cat > 0);
    }
    if(produkt.kategoria > 0 && produkt.status != Produkt::KATALOG && mKatNadrzedne.contains(produkt.kategoria)) {
        product.id_category_default = produkt.kategoria;
        unsigned cat = produkt.kategoria;
        unsigned i = 0;
        do {
            i++;
            product.categories << cat;

            if(mKatNadrzedne.contains(cat)) {
                cat = mKatNadrzedne[cat];
            } else {
                break;
            }

            if(i>100) throw std::string("chujnia!");
        } while(cat > 0);
    }

    return product;
}

Presta::Category KCPresta::kc2presta(const Kategoria &kategoria)
{
    Presta::Category category;
    category.id = kategoria.id;
    category.idRef = kategoria.idKC;
    category.active = 1;
    category.id_parent = 1;
    category.name = kategoria.nazwa;
    category.link_rewrite = kategoria.nazwa;
    category.link_rewrite = category.link_rewrite.replace(" ", "-").replace(",", "_").remove(QRegExp("[^A-Za-z0-9_-]")).toLower();
    return category;
}

QList<Presta::OrderHeader> KCPresta::pobierzZamowienia()
{
    QMap<QString, QString> filter;
    filter["current_state"] = "[" + QString::number(mStatusyZamowien[OCZEKUJE]) + "|" + QString::number(mStatusyZamowien[W_REALIZACJI]) + "|" + QString::number(mStatusyZamowien[DO_ODBIORU]) + "]";
    return mPresta->getOrderHeader(filter);
}

QString KCPresta::statusyZamowienNazwa(ZamowienieStatus status)
{
    QString result;
    switch(status) {
    case KCPresta::OCZEKUJE: result=QString::fromUtf8("Oczekujące"); break;
    case KCPresta::W_REALIZACJI: result=QString::fromUtf8("W realizacji"); break;
    case KCPresta::DO_ODBIORU: result=QString::fromUtf8("Do odbioru"); break;
    case KCPresta::WYSLANE: result=QString::fromUtf8("Wysłane"); break;
    case KCPresta::ZREALIZOWANE: result=QString::fromUtf8("Zrealizowane"); break;
    case KCPresta::ANULOWANE: result=QString::fromUtf8("Anulowane"); break;
    }
    return result;
}
