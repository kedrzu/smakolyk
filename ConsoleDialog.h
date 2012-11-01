#ifndef CONSOLEDIALOG_H
#define CONSOLEDIALOG_H

#include <QDialog>

namespace Ui {
class ConsoleDialog;
}

class ConsoleDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConsoleDialog(QWidget *parent = 0);
    void setText(const QString& text);
    ~ConsoleDialog();
    
private slots:
    void on_pushButton_clicked();

private:
    Ui::ConsoleDialog *ui;
};

#endif // CONSOLEDIALOG_H
