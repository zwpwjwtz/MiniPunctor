#include "tickshiftwindow.h"
#include "ui_tickshiftwindow.h"
#include <QMessageBox>


TickShiftWindow::TickShiftWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TickShiftWindow)
{
    ui->setupUi(this);

    shiftValue = 2000;
}

TickShiftWindow::~TickShiftWindow()
{
    delete ui;
}

void TickShiftWindow::on_buttonOK_clicked()
{
    bool ok;
    if (ui->radioSecond->isChecked())
        shiftValue = ui->textInput->text().toInt(&ok) * 1000;
    else if (ui->radioMinute->isChecked())
        shiftValue = ui->textInput->text().toInt(&ok) * 60000;
    else
        shiftValue = ui->textInput->text().toInt(&ok);

    if (!ok)
        QMessageBox::warning(this,
                             "Invalid input",
                             "Please input a number.");
    else
    {
        accepted = true;
        close();
    }
}

void TickShiftWindow::on_buttonCancel_clicked()
{
    accepted = false;
    close();
}
