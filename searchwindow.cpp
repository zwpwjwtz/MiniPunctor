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
            SLOT(on_timeline_search(QString, int&)));
    connect(this,
            SIGNAL(replaceTimeline(QString, QString, int&)),
            parent,
            SLOT(on_timeline_replace(QString, QString, int&)));
    connect(this,
            SIGNAL(replaceTimelineAll(QString, QString, int&, int&)),
            parent,
            SLOT(on_timeline_replace_all(QString, QString, int&, int&)));
}

SearchWindow::~SearchWindow()
{
    delete ui;
}

bool SearchWindow::checkInput(bool replace)
{
    if (ui->textFind->toPlainText().isEmpty())
    {
        QMessageBox::warning(this,
                             "Warning",
                             "Please input the string to search!");
        return false;
    }

    if (replace && ui->textReplace->toPlainText().isEmpty())
    {
        QMessageBox::warning(this,
                             "Warning",
                             "Please input the string to replace with!");
        return false;
    }
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
    if (checkInput(false))
        return;

    emit searchTimeline(ui->textFind->toPlainText(), lastSearched);
}

void SearchWindow::on_buttonReplace_clicked()
{
    if (!(ui->checkBox->isChecked() && checkInput(true)))
        return;

    emit replaceTimeline(ui->textFind->toPlainText(),
                         ui->textReplace->toPlainText(),
                         lastSearched);
}

void SearchWindow::on_buttonReplaceAll_clicked()
{
    if (!(ui->checkBox->isChecked() && checkInput(true)))
        return;

    int replaceCount = 0;
    emit replaceTimelineAll(ui->textFind->toPlainText(),
                            ui->textReplace->toPlainText(),
                            lastSearched,
                            replaceCount);
    QMessageBox::information(this,
                             "Replace All",
                             QString().sprintf("Finish replacing. "
                                              "%d items replaced.",
                                              replaceCount),
                             QMessageBox::Ok);
}
