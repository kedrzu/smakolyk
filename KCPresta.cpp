#include "KCPresta.h"
using namespace Presta;

KCPresta::KCPresta(const Config &config, PSWebService* pswebService, KCFirma* kcFirma, QObject *parent) :
    Prestashop(config, pswebService, parent),
    mKCFirma(kcFirma),
    mFinished(true),
    mProduktyUpload(5)
{
}

void KCPresta::aktualizujKategorie()
{
    // najpierw œci¹gamy kategorie z internetu
    PSWebService::Options opt;
    opt.resource = "categories";
    opt.display = "[id,id_parent,active,name,link_rewrite,meta_title,meta_description,meta_keywords,description]";
    try {
        QDomDocument result = mPSWebService->syncGet(opt);
        QDomElement prestashop = result.firstChildElement("prestashop");
        if(!prestashop.isNull()) {
            QDomElement categories = prestashop.firstChildElement("categories");
            if(!categories.isNull()) {
                // czyœcimy star¹ mapê
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
                // TODO ³apanie b³edów
            }
        }
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
    return mKCFirma->produkty(mProdukty, mProduktyUpload);
}

bool KCPresta::dodajProdukty(uint ile)
{
    return mKCFirma->produkty(mProdukty, ile);
}

SpecificPrice KCPresta::getSpecificPrice(const Produkt &produkt)
{
    SpecificPrice sp;
    sp.id_product = produkt.id;
    sp.reduction_type = SpecificPrice::AMOUNT;
    sp.price = produkt.cenaKC;
    // TODO zrobiæ sygnalizacjê b³êdu je¿eli cenaPresta > cenaKC
    sp.reduction = produkt.cenaKC - produkt.cenaPresta;
    return sp;
}

void KCPresta::upload() {

    // ustawiamy flagê
    mFinished = false;

    // kasujemy bufor b³êdnie uploadowanych produktów
    mProduktyError.clear();

    foreach(const Produkt& prod, mProdukty) {
        // je¿eli wyst¹pi b³¹d przy dodawaniu kategorii, to nie dodajemy produktu
        bool err = false;
        // kategoria jest produktu nowa, lub by³a w Presta ale ju¿ jej nie ma => dodajemy do Presta
        if(prod.kategoria == 0 || !mKatNadrzedne.contains(prod.kategoria)) {
            Kategoria kat = mKCFirma->kategoria(prod.kategoriaKC);
            kat.id = 0;
            // dodajemy kategoriê, je¿eli sie nie uda emitujemy blad i kontynujemy z nastepnym produktem
            try {
                // kategoria dodawana jest synchronicznie
                Category category = kc2presta(kat);
                unsigned id = syncAdd(category);
                // aktualizujemy powi¹zanie
                emit zmianaKategorii(id, prod.kategoriaKC);
                mKatNadrzedne[id] = category.id_parent;
            } catch (PSWebService::PrestaError e) {
                emit error(e);
                err = true;
            } catch (PSWebService::OtherError e) {
                emit error(e);
                err = true;
            }
        }
        // je¿eli produkt ma ju¿ prawid³ow¹ kategoriê, to dodajemy
        if(!err) {
            // dodajemy produkt
            Presta::Product product = kc2presta(prod);
            // je¿eli produkt jest w bazie => edytujemy
            if(prod.id > 0) {
                QNetworkReply* reply = edit(product);
                if(reply->isFinished()) {
                    productEdited(reply);
                } else {
                    connect(reply, SIGNAL(finished()), this, SLOT(productEdited()));
                }
            }
            // je¿eli nie, dodajemy
            else {
                QNetworkReply* reply = add(product);
                if(reply->isFinished()) {
                    productAdded(reply);
                } else {
                    connect(reply, SIGNAL(finished()), this, SLOT(productAdded()));
                }
            }

        }
        // w przeciwnym wypadku oznaczamy produkt jako b³êdnie uploadowany
        else {
            mProduktyError[prod.idKC] = KATEGORIA;
        }
    }

    //    // iterujemy po produktach do aktualizacji
    //    QMapIterator<unsigned, Produkt> it(mProdukty);
    //    while(it.hasNext()) {
    //        it.next();
    //        // je¿eli wyst¹pi b³¹d przy dodawaniu kategorii, to nie dodajemy produktu
    //        bool err = false;
    //        // kategoria jest produktu nowa, lub by³a w Presta ale ju¿ jej nie ma => dodajemy do Presta
    //        if(it.value().kategoria == 0 || !mKatNadrzedne.contains(it.value().kategoria)) {
    //            Kategoria kat = mKCFirma->kategoria(it.value().kategoriaKC);
    //            kat.id = 0;
    //            // dodajemy kategoriê, je¿eli sie nie uda emitujemy blad i kontynujemy z nastepnym produktem
    //            try {
    //                // kategoria dodawana jest synchronicznie
    //                unsigned id = syncAdd(kat);
    //                // aktualizujemy powi¹zanie
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
    //        // je¿eli produkt ma ju¿ prawid³ow¹ kategoriê, to dodajemy
    //        if(!err) {

    //            // dodajemy kategorie, do których nale¿y produkt
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

    //            // je¿eli produkt jest w bazie => edytujemy
    //            if(it.value().id > 0) {
    //                QNetworkReply* reply = edit(it.value());
    //                if(reply->isFinished()) {
    //                    productEdited(reply);
    //                } else {
    //                    connect(reply, SIGNAL(finished()), this, SLOT(productEdited()));
    //                }
    //            }
    //            // je¿eli nie, dodajemy
    //            else {
    //                QNetworkReply* reply = add(it.value());
    //                if(reply->isFinished()) {
    //                    productAdded(reply);
    //                } else {
    //                    connect(reply, SIGNAL(finished()), this, SLOT(productAdded()));
    //                }
    //            }
    //        }
    //        // w przeciwnym wypadku oznaczamy produkt jako b³êdnie uploadowany
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
        // uploadujemy cenê specjaln¹ produktu
        const Produkt& prod = mProdukty.value(idKC);
        Presta::SpecificPrice sp = getSpecificPrice(prod);
        syncAdd(sp);
        // TODO przechwytywanie wyj¹tków z bazy danych
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

    // je¿eli wszystkie operacje zosta³y wykonane emitujemy odpowiedni sygna³
    checkFinished();
}

void KCPresta::productEdited(QNetworkReply *reply)
{
    unsigned idKC = reply->property("idRef").toUInt();
    try {
        QDomDocument doc = PSWebService::readReply(reply);
        unsigned id = Product::getId(doc);
        // uploadujemy cenê specjaln¹ produktu
        const Produkt& prod = mProdukty.value(idKC);
        Presta::SpecificPrice sp = getSpecificPrice(prod);
        QList<uint> spList = Prestashop::getSpecificPrice(id);
        // je¿eli produkt ma ju¿ zapisan¹ cenê specjaln¹ to nale¿y j¹ edytowaæ
        if(spList.size() > 0) {
            sp.id  = spList.at(0);
            syncEdit(sp);
            for(int i=1; i<spList.size(); ++i) {
                // TODO usuwanie niepotrzebnych
            }
        } else {
            syncAdd(sp);
        }
        // TODO przechwytywanie wyj¹tków z bazy danych
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

    // je¿eli wszystkie operacje zosta³y wykonane emitujemy odpowiedni sygna³
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
        mFinished = true;
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

    // dodajemy kategorie, do których nale¿y produkt
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
