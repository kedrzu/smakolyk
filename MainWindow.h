#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QListWidgetItem>
#include "KCPresta.h"
#include "Logger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void logError(const PSWebService::PrestaError& err);
    void logError(const PSWebService::OtherError& err);
    void logDebug(const QString& msg);
    void logWarning(const QString& msg);
    void logNotice(const QString& msg);

private slots:
    void start();
    void stop();
    void timeout();
    void wyslijTowary();
    void pobierzZamowienia();
    void koniec();
    void showLog(QListWidgetItem* item);

private:
    void updateTimer();
    void updateGUI();
    void clearLog();

    Ui::MainWindow *ui;
    bool mStart;
    uint mInterwalSekundy;
    uint mSekundy;
    uint mMaxLogMsgs;
    QTimer mTimer;
    QMap<QListWidgetItem*, QString> mConsoleLog;
    PSWebService* mWebService;
    Presta::Prestashop *mPresta;
    KCFirma *mKCFirma;
    KCPresta *mKCPresta;
    Logger *mLogger;
};

#endif // MAINWINDOW_H
