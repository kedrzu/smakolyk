#include "MainWindow.h"
#include "ui_MainWindow.h"
#include <QTime>
#include <ConsoleDialog.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mStart(false),
    mSekundy(0),
    mMaxLogMsgs(1000)
{
    ui->setupUi(this);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(timeout()));

    // odczytujemy konfigurację
    QSettings config("config.ini", QSettings::IniFormat);

    // presta API
    QString configUrl = config.value("prestashop/url").toString();
    QString configKey = config.value("prestashop/key").toString();
    mWebService = new PSWebService(configUrl, configKey);

    // ustawienia
    mInterwalSekundy = config.value("KC-Presta/upload_interwal").toUInt();

    // obiekt komunikacji z Presta
    mPresta = new Presta::Prestashop(config, mWebService, this);

    // obiekt KC-Firmy
    mKCFirma = new KCFirma(config, this);

    // obiekt KC-Presta
    mKCPresta = new KCPresta(config, mPresta, mKCFirma, this);

    // obiekt logowania pracy programu
    mLogger = new Logger(mKCPresta);

    // łączymy KC-Presta z konsolą
    connect(mKCPresta, SIGNAL(warning(QString)), this, SLOT(logWarning(QString)));
    connect(mKCPresta, SIGNAL(notice(QString)), this, SLOT(logNotice(QString)));
    connect(mKCPresta, SIGNAL(debug(QString)), this, SLOT(logDebug(QString)));
    connect(mKCPresta, SIGNAL(error(PSWebService::PrestaError)), this, SLOT(logError(PSWebService::PrestaError)));
    connect(mKCPresta, SIGNAL(error(PSWebService::OtherError)), this, SLOT(logError(PSWebService::OtherError)));

    // ikony zamówień
    mZamowieniaIkony[KCPresta::OCZEKUJE] = QIcon(":/icons/clock.png");
    mZamowieniaIkony[KCPresta::W_REALIZACJI] = QIcon(":/icons/basket.png");
    mZamowieniaIkony[KCPresta::DO_ODBIORU] = QIcon(":/icons/package.png");
    mZamowieniaIkony[KCPresta::WYSLANE] = QIcon(":/icons/lorry.png");
    mZamowieniaIkony[KCPresta::ZREALIZOWANE] = QIcon(":/icons/tick.png");
    mZamowieniaIkony[KCPresta::ANULOWANE] = QIcon(":/icons/cancel.png");

    updateTimer();
    updateGUI();

    // autostart
    if(config.value("KC-Presta/autostart").toUInt())
        start();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete mWebService;
    delete mKCFirma;
    delete mKCPresta;
    delete mLogger;
}

void MainWindow::start()
{
    // wciśnięcie przycisku Start
    if(!mStart) {
        mStart = true;
        updateGUI();
        pobierzZamowienia();
        wyslijTowary();
    }
}

void MainWindow::stop()
{
    if(mStart) {
        mStart = false;
        mSekundy = 0;
        updateTimer();
        updateGUI();
        mTimer.stop();
    }
}

void MainWindow::timeout()
{
    if(mKCPresta->isUploadFinished()) {
        if(mSekundy == 0) {
            mTimer.stop();
            pobierzZamowienia();
            wyslijTowary();
        } else if(mSekundy > 0) {
            --mSekundy;
            updateTimer();
        }
    } else {
        mTimer.stop();
    }
}

void MainWindow::wyslijTowary()
{
    // jeżeli towary aktualnie nie są wysyłane, to wysyłamy
    if(mKCPresta->isUploadFinished()) {
        ui->timeLeft->setText(QString::fromUtf8("WYSYŁAM TOWARY"));
        mKCPresta->dodajProdukty();
        if(mKCPresta->buforProduktow().empty()) {
            logNotice(QString::fromUtf8("Brak towarów do wysłania"));
        } else {
            logNotice(QString::fromUtf8("Wysyłam towary"));
            mKCPresta->aktualizujKategorie();
            uint liczbaProduktow = mKCPresta->buforProduktow().size();
            do {
                mKCPresta->uploadProdukty();
                QEventLoop loop;
                if(!mKCPresta->isUploadFinished()) {
                    QObject::connect(mKCPresta, SIGNAL(uploadFinished()), &loop, SLOT(quit()));
                    loop.exec();
                };
                if(mKCPresta->buforProduktow().size() > 0) {
                    logWarning(QString::fromUtf8("Nie udało się wysłać ").append(QString::number(mKCPresta->buforProduktow().size())).append(QString::fromUtf8(" towarów")));
                }
                mKCPresta->dodajProdukty();
                liczbaProduktow += mKCPresta->buforProduktow().size();
                logDebug(QString::fromUtf8("Wysłano ").append(QString::number(liczbaProduktow)).append(QString::fromUtf8(" towarów")));
            } while (!mKCPresta->buforProduktow().empty());
            logNotice(QString::fromUtf8("Wysłano ").append(QString::number(liczbaProduktow)).append(QString::fromUtf8(" towarów")));
        }
        if(mStart) {
            mSekundy = mInterwalSekundy;
            mTimer.start(1000);
        }
        updateTimer();
    }
}

void MainWindow::pobierzZamowienia()
{
    ui->timeLeft->setText(QString::fromUtf8("POBIERAM ZAMÓWIENIA"));
    try {
        QList<Presta::OrderHeader> zamowienia = mKCPresta->pobierzZamowienia();
        uint add = 0;
        uint update = 0;
        logDebug("Pobrano " + QString::number(zamowienia.size()) + QString::fromUtf8(" zamówień."));
        for(int i=0; i<zamowienia.size(); ++i) {
            uint id = zamowienia.at(i).id;
            uint state = zamowienia.at(i).current_state;

            if(mZamowieniaWidgetItem.contains(id)) {
                if(mZamowieniaHeaders.value(id).current_state != state) {
                    mZamowieniaWidgetItem.value(id)->setIcon(mZamowieniaIkony.value(mKCPresta->statusyZamowien(state)));
                    ++update;
                }
            } else {
                QListWidgetItem* item = new QListWidgetItem(mZamowieniaIkony.value(mKCPresta->statusyZamowien(state)), zamowienia.at(i).reference + "\t[" + zamowienia.at(i).date_add + "]", ui->listaZamowien);
                mZamowieniaWidgetItem[id] = item;
                ++add;
            }
            mZamowieniaHeaders[id] = zamowienia.at(i);
        }
        if(add > 0)
            logNotice("Pobrano " + QString::number(add) + QString::fromUtf8(" nowych zamówień."));
        if(update > 0)
            logNotice(QString::fromUtf8("Na liście zaktualizowano ") + QString::number(update) + QString::fromUtf8(" zamówień."));
    } catch (PSWebService::PrestaError e) {
        logError(e);
    } catch (PSWebService::OtherError e) {
        logError(e);
    }
    updateTimer();
}

void MainWindow::updateTimer()
{
    if(mKCPresta->isUploadFinished()) {
        if(mStart) {
            uint s = mSekundy % 60;
            uint m = (mSekundy / 60);
            QTime time(0, m, s);
            ui->timeLeft->setText(time.toString("mm:ss").prepend("PRZERWA "));
        } else {
            ui->timeLeft->setText("ZATRZYMANY");
        }
    }
}

void MainWindow::updateGUI()
{
    if(mStart) {
        ui->buttonStart->setEnabled(false);
        ui->buttonStop->setEnabled(true);
    } else {
        ui->buttonStart->setEnabled(true);
        ui->buttonStop->setEnabled(false);
    }
}

void MainWindow::koniec()
{
    close();
}

void MainWindow::logError(const PSWebService::PrestaError &err)
{
    QString msg = QString::fromUtf8("Wystąpił błąd podczas błędu");
    QListWidgetItem* item = new QListWidgetItem(QIcon(":/icons/exclamation.png"), QTime::currentTime().toString("[HH:mm:ss] ")+msg, ui->console);
    QString log = "<table cellspacing='5'>";
    log.append(QString::fromUtf8("<tr><td><b>Typ wiadomości:</b></td><td>Błąd Prestashop</td></tr>"));
    log.append(QString::fromUtf8("<tr><td><b>Wykonywana akcja:</b></td><td>")+err.msg+"</td></tr>");
    log.append(QString::fromUtf8("<tr><td><b>Kod błędu sieciowego:</b></td><td>")+QString::number(err.code)+"</td></tr>");
    log.append(QString::fromUtf8("<tr><td><b>URL:</b></td><td>")+err.url.toString()+"</td></tr>");
    log.append("</table>\n");
    for(int i=0; i<err.msgs.size(); ++i) {
        log.append("<table cellspacing='5'>");
        log.append(QString::fromUtf8("<tr><td><b>Kod błedu Prestashop:</b></td><td>")+QString::number(err.msgs.at(i).first)+"</td></tr>");
        log.append(QString::fromUtf8("<tr><td><b>Komunikat:</b></td><td>")+err.msgs.at(i).second+"</td></tr>");
        log.append("</table>\n");
    }
    mConsoleLog[item] = log;
    clearLog();
}

void MainWindow::logError(const PSWebService::OtherError &err)
{
    QString msg = QString::fromUtf8("Wystąpił błąd podczas błędu");
    QListWidgetItem* item = new QListWidgetItem(QIcon(":/icons/exclamation.png"), QTime::currentTime().toString("[HH:mm:ss] ")+msg, ui->console);
    QString log = "<table cellspacing='5'>";
    log.append(QString::fromUtf8("<tr><td><b>Typ wiadomości:</b></td><td>Nieznany błąd</td></tr>"));
    log.append(QString::fromUtf8("<tr><td><b>Wykonywana akcja:</b></td><td>")+err.msg+"</td></tr>");
    log.append(QString::fromUtf8("<tr><td><b>Kod błędu sieciowego:</b></td><td>")+QString::number(err.code)+"</td></tr>");
    log.append(QString::fromUtf8("<tr><td><b>URL:</b></td><td>")+err.url.toString()+"</td></tr>");
    log.append("</table>\n");
    mConsoleLog[item] = log;

    clearLog();
}

void MainWindow::logNotice(const QString &msg)
{
    new QListWidgetItem (QIcon(":/icons/information.png"), QTime::currentTime().toString("[HH:mm:ss] ")+msg, ui->console);

    clearLog();
}

void MainWindow::logWarning(const QString &msg)
{
    new QListWidgetItem(QIcon(":/icons/error.png"), QTime::currentTime().toString("[HH:mm:ss] ")+msg, ui->console);

    clearLog();
}

void MainWindow::logDebug(const QString &msg)
{
    new QListWidgetItem(QIcon(":/icons/bug.png"), QTime::currentTime().toString("[HH:mm:ss] ")+msg, ui->console);

    clearLog();
}

void MainWindow::showLog(QListWidgetItem *item)
{
    if(mConsoleLog.contains(item)) {
        ConsoleDialog dialog;
        dialog.setText(mConsoleLog.value(item));
        dialog.exec();
    }
}

void MainWindow::clearLog()
{
    ui->console->scrollToBottom();
    while((uint)ui->console->count() > mMaxLogMsgs) {
        QListWidgetItem* item = ui->console->takeItem(1);
        if(item) {
            mConsoleLog.remove(item);
            delete item;
        } else {
            break;
        }
    }
}

void MainWindow::updateZamowienia(KCPresta::ZamowienieStatus status)
{
    QList<QListWidgetItem*> selected = ui->listaZamowien->selectedItems();
    uint update = 0;
    for(int i=0; i<selected.size(); ++i) {
        try {
            uint id = mZamowieniaWidgetItem.key(selected.at(i));
            uint state = mKCPresta->statusyZamowien(status);
            if(mZamowieniaHeaders.value(id).current_state != state) {
                Presta::Order order = mPresta->getOrder(id);
                order.current_state = state;
                mPresta->syncEdit(order);
                mZamowieniaWidgetItem.value(id)->setIcon(mZamowieniaIkony.value(mKCPresta->statusyZamowien(state)));
                Presta::OrderHeader header = mZamowieniaHeaders.value(id);
                header.current_state = state;
                mZamowieniaHeaders[id] = header;
                ++update;
            }

        } catch (PSWebService::PrestaError e) {
            logError(e);
        } catch (PSWebService::OtherError e) {
            logError(e);
        }
    }
    if(update > 0) {
        logNotice(QString::fromUtf8("Zmieniono stan ") + QString::number(update) + QString::fromUtf8(" zamówień na \"") + KCPresta::statusyZamowienNazwa(status) + "\"");
    }
    ui->listaZamowien->clearSelection();
}

void MainWindow::on_buttonDoRealizacji_clicked()
{
    updateZamowienia(KCPresta::W_REALIZACJI);
}


void MainWindow::on_buttonDoOdbioru_clicked()
{
    updateZamowienia(KCPresta::DO_ODBIORU);
}

void MainWindow::on_buttonRezygnacja_clicked()
{
    updateZamowienia(KCPresta::ANULOWANE);
}

void MainWindow::on_buttonZrealizowane_clicked()
{
    updateZamowienia(KCPresta::ZREALIZOWANE);
}
