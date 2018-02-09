#include "searchwindow.h"
#include "ui_searchwindow.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <cstdio>

SearchWindow::SearchWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchWindow)
{
    ui->setupUi(this);

    lastSearched = -1;

    connect(this,
            SIGNAL(searchTimeline(QString, int&)),
            parent,
            SLOT(onTimelineSearch(QString, int&)));
    connect(this,
            SIGNAL(replaceTimeline(QString, QString, int&)),
            parent,
            SLOT(onTimelineReplace(QString, QString, int&)));
    connect(this,
            SIGNAL(replaceTimelineAll(QString, QString, int&)),
            parent,
            SLOT(onTimelineReplaceAll(QString, QString, int&)));
}

SearchWindow::~SearchWindow()
{
    delete ui;
}

bool SearchWindow::checkInput()
{
    if (ui->textFind->toPlainText().isEmpty())
    {
        QMessageBox::warning(this,
                             "Warning",
                             "Please input the string to search!");
        return false;
    }

    /*
    if (replace && ui->textReplace->toPlainText().isEmpty())
    {
        QMessageBox::warning(this,
                             "Warning",
                             "Please input the string to replace with!");
        return false;
    }
    */
    return true;
}

void SearchWindow::on_checkBox_clicked()
{
    ui->textReplace->setEnabled(ui->checkBox->isChecked());
}

void SearchWindow::on_buttonExit_clicked()
{
    close();
}

void SearchWindow::on_buttonFind_clicked()
{
    if (!checkInput())
        return;

    emit searchTimeline(ui->textFind->toPlainText(), lastSearched);
}

void SearchWindow::on_buttonReplace_clicked()
{
    if (!(ui->checkBox->isChecked() && checkInput()))
        return;

    emit replaceTimeline(ui->textFind->toPlainText(),
                         ui->textReplace->toPlainText(),
                         lastSearched);
}

void SearchWindow::on_buttonReplaceAll_clicked()
{
    if (!(ui->checkBox->isChecked() && checkInput()))
        return;

    emit replaceTimelineAll(ui->textFind->toPlainText(),
                            ui->textReplace->toPlainText(),
                            lastSearched);
}
