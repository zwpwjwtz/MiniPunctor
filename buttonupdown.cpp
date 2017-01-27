#include "buttonupdown.h"
#include "ui_buttonupdown.h"
#include <QSize>
#include <QLabel>
#include <QPoint>

buttonUpDown::buttonUpDown(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::buttonUpDown)
{
    ui->setupUi(this);
}

buttonUpDown::~buttonUpDown()
{
    delete ui;
}

QString buttonUpDown::getText()
{
    return ui->labelText->text();
}

void buttonUpDown::setText(QString text)
{
    ui->labelText->setText(text);
}

void buttonUpDown::resizeEvent(QResizeEvent* size)
{
    QSize textSize = ui->labelText->size();
    int newWidth = size->size().width();
    int textPosX = ui->labelText->pos().x();
    textSize.setWidth(newWidth - textPosX - 10);
    ui->labelText->resize(textSize);
}

void buttonUpDown::on_buttonUp_clicked()
{
    emit clicked(0);
}

void buttonUpDown::on_buttonDown_clicked()
{
    emit clicked(1);
}
