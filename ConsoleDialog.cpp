#include "ConsoleDialog.h"
#include "ui_ConsoleDialog.h"

ConsoleDialog::ConsoleDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConsoleDialog)
{
    ui->setupUi(this);
}

ConsoleDialog::~ConsoleDialog()
{
    delete ui;
}

void ConsoleDialog::setText(const QString &text)
{
    ui->textBrowser->setHtml(text);
}

void ConsoleDialog::on_pushButton_clicked()
{
    close();
}
