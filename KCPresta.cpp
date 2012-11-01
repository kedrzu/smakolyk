#include "KCPresta.h"
using namespace Presta;

KCPresta::KCPresta(const QSettings& settings, Prestashop *presta, KCFirma *kcFirma, QObject *parent) :
    mPresta(presta),
    mKCFirma(kcFirma),
    mPSWebService(presta->getWebService()),
    mUploadFinished(true),
    mProduktyUpload(settings.value("KC-Presta/produkty_upload").toUInt())
{
}

void KCPresta::aktualizujKategorie()
{
    // najpierw ściągamy kategorie z internetu
    emit debug("KCPresta::aktualizujKategorie()");
    PSWebService::Options opt;
    opt.resource = "categories";
    opt.display = "[id,id_parent,active,name,link_rewrite,meta_title,meta_description,meta_keywords,description]";
    try {
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
    } catch (PSWebService::PrestaError e) {
        emit error(e);
        throw e;
    } catch (PSWebService::OtherError e) {
        emit error(e);
        throw e;
    }
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

void KCPresta::upload() {

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

    foreach(const Produkt& prod, mProdukty) {
        // jeżeli wystąpi błąd przy dodawaniu kategorii, to nie dodajemy produktu
        bool err = false;
        // kategoria jest produktu nowa, lub była w Presta ale już jej nie ma => dodajemy do Presta
        if(prod.kategoria == 0 || !mKatNadrzedne.contains(prod.kategoria)) {
            Kategoria kat = mKCFirma->kategoria(prod.kategoriaKC);
            kat.id = 0;
            // dodajemy kategorię, jeżeli sie nie uda emitujemy blad i kontynujemy z nastepnym produktem
            try {
                // kategoria dodawana jest synchronicznie
                Category category = kc2presta(kat);
                unsigned id = mPresta->syncAdd(category);
                // aktualizujemy powiązanie
                mKCFirma->zmianaKategorii(id, prod.kategoriaKC);
                mKatNadrzedne[id] = category.id_parent;
            } catch (PSWebService::PrestaError e) {
                emit error(e);
                err = true;
            } catch (PSWebService::OtherError e) {
                emit error(e);
                err = true;
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


    //    // iterujemy po produktach do aktualizacji
    //    QMapIterator<unsigned, Produkt> it(mProdukty);
    //    while(it.hasNext()) {
    //        it.next();
    //        // jeżeli wystąpi błąd przy dodawaniu kategorii, to nie dodajemy produktu
    //        bool err = false;
    //        // kategoria jest produktu nowa, lub była w Presta ale już jej nie ma => dodajemy do Presta
    //        if(it.value().kategoria == 0 || !mKatNadrzedne.contains(it.value().kategoria)) {
    //            Kategoria kat = mKCFirma->kategoria(it.value().kategoriaKC);
    //            kat.id = 0;
    //            // dodajemy kategorię, jeżeli sie nie uda emitujemy blad i kontynujemy z nastepnym produktem
    //            try {
    //                // kategoria dodawana jest synchronicznie
    //                unsigned id = mPresta->syncAdd(kat);
    //                // aktualizujemy powiązanie
    //                emit zmianaKategorii(id, it.value().kategoriaKC);
    //                mKatNadrzedne[id] = kat.nadrzedna;
    //            } catch (PSWebService::PrestaError e) {
    //                emit error(e);
    //                err = true;
    //            } catch (PSWebService::OtherError e) {
    //                emit error(e);
    //                err = true;
    //            }
    //        }
    //        // jeżeli produkt ma już prawidłową kategorię, to dodajemy
    //        if(!err) {

    //            // dodajemy kategorie, do których należy produkt
    //            unsigned cat = it.value().kategoria;
    //            unsigned i = 0;
    //            do {
    //                i++;
    //                it.value().kategorie << cat;

    //                if(mKatNadrzedne.contains(cat)) {
    //                    cat = mKatNadrzedne[cat];
    //                } else {
    //                    break;
    //                }

    //                if(i>100) throw std::string("chujnia!");
    //            } while(cat > 0);

    //            // jeżeli produkt jest w bazie => edytujemy
    //            if(it.value().id > 0) {
    //                QNetworkReply* reply = mPresta->edit(it.value());
    //                if(reply->isFinished()) {
    //                    productEdited(reply);
    //                } else {
    //                    connect(reply, SIGNAL(finished()), this, SLOT(productEdited()));
    //                }
    //            }
    //            // jeżeli nie, dodajemy
    //            else {
    //                QNetworkReply* reply = mPresta->add(it.value());
    //                if(reply->isFinished()) {
    //                    productAdded(reply);
    //                } else {
    //                    connect(reply, SIGNAL(finished()), this, SLOT(productAdded()));
    //                }
    //            }
    //        }
    //        // w przeciwnym wypadku oznaczamy produkt jako błędnie uploadowany
    //        else {
    //            mProduktyError[it.value().idKC] = KATEGORIA;
    //        }
    //    }

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
        // TODO przechwytywanie wyjątków z bazy danych
        mKCFirma->zmianaProduktu(id, idKC, prod.cenaPresta);
        mProdukty.remove(idKC);
    } catch (PSWebService::PrestaError e) {
        e.msg = "productAdded()";
        mProduktyError[idKC] = ADD_ERROR;
        emit error(e);
    } catch (PSWebService::OtherError e) {
        e.msg = "productAdded()";
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
        // TODO przechwytywanie wyjątków z bazy danych
        mKCFirma->zmianaProduktu(id, idKC, prod.cenaPresta);
        mProdukty.remove(idKC);
    } catch (PSWebService::PrestaError e) {
        e.msg = "productEdited()";
        mProduktyError[idKC] = EDIT_ERROR;
        emit error(e);
    } catch (PSWebService::OtherError e) {
        e.msg = "productEdited()";
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
    product.link_rewrite = product.link_rewrite.replace(" ", "-").toLower();

    // dodajemy kategorie, do których należy produkt
    if(mKatNadrzedne.contains(produkt.kategoria)) {
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
    category.link_rewrite = category.link_rewrite.replace(" ", "-").toLower();
    return category;
}
