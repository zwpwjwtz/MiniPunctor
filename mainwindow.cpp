#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "helpwindow.h"
#include <QCloseEvent>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QFileDialog>


#define PUNCTOR_TIME_DISPLAY_FORMAT "mm : ss . zzz"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->bUDMin->setText("Min");
    ui->bUDSec->setText("Sec");
    ui->bUDMsec->setText("Msec");

    connect(&timer,
            SIGNAL(timeout()),
            this,
            SLOT(on_timer_timeout()));

    timerState = 0;
    timerInterval = 100;
    timer.setInterval(timerInterval);

    currentTime = 0;
    showTime(currentTime);

    isPuncturing = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString timeTickToString(qint64 timeTick)
{
    static int msec, sec, min;
    msec =  timeTick % 1000;    timeTick /= 1000;
    sec = timeTick % 60;    timeTick /=60;
    min = timeTick % 60;    timeTick /=60;

    QString temp(PUNCTOR_TIME_DISPLAY_FORMAT);
    static char buf[4];
    ::sprintf(buf, "%03d", msec);    temp.replace("zzz", buf);
    ::sprintf(buf, "%02d", sec);     temp.replace("ss", buf);
    ::sprintf(buf, "%02d", min);     temp.replace("mm", buf);

    return temp;
}

void MainWindow::saveFile()
{
    QString path = QFileDialog::getSaveFileName(
                this,
                "Save file as",
                ".",
                "Plain Text(*.txt)"
    );
    if (path.isEmpty())
        return;

    QList<QString> recordList;
    QListWidget* listTimeline = ui->listTimeline;
    for (int i=0; i<listTimeline->count(); i++)
    {
        recordList.append(listTimeline->item(i)->text());
    }

    currentFile.saveAs(recordList, path);
}

void MainWindow::showTime(qint64 timeTick)
{
    ui->labelTime->setText(timeTickToString(timeTick));
}

bool MainWindow::sureToExit(bool manualClose)
{
    if (!manualClose && isPuncturing)
    {
        if (QMessageBox::question(this,"Quit F3",
                              "The program is puncturing now.\n"
                              "Quit anyway?",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
            return false;
    }
    return true;
}


void MainWindow::closeEvent(QCloseEvent* event)
{
    if (sureToExit(false))
    {
        help.close();
    }
    else
        event->ignore();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space)
        on_buttonPunc_clicked();
}

void MainWindow::on_timer_timeout()
{
    currentTime += timerInterval;
    showTime(currentTime);
}

void MainWindow::on_bUDMin_clicked(int button)
{
    if (button)
    {
        currentTime -= 60000;
        if (currentTime < 0)
            currentTime = 0;
    }
    else
        currentTime += 60000;
    showTime(currentTime);
}

void MainWindow::on_bUDSec_clicked(int button)
{
    if (button)
    {
        currentTime -= 1000;
        if (currentTime < 0)
            currentTime = 0;
    }
    else
        currentTime += 1000;
    showTime(currentTime);
}

void MainWindow::on_bUDMSec_clicked(int button)
{
    if (button)
    {
        currentTime -= timerInterval;
        if (currentTime < 0)
            currentTime = 0;
    }
    else
        currentTime += timerInterval;
    showTime(currentTime);
}

void MainWindow::on_buttonPunc_clicked()
{
    ui->listTimeline->addItem(timeTickToString(currentTime));
}

void MainWindow::on_buttonStart_clicked()
{
    switch (timerState) {
    case 0: //Stop
    case 2: //Pause
        timerState = 1;
        isPuncturing = true;
        timer.start();
        ui->buttonStart->setText("Resume");
        break;
    case 1: //Counting
        timerState = 2;
        isPuncturing =false;
        timer.stop();
        ui->buttonStart->setText("Pause");
    default:
        break;
    }
}

void MainWindow::on_buttonStop_clicked()
{
    timerState = 0;
    currentTime = 0;
    isPuncturing = false;
    timer.stop();
    ui->buttonStart->setText("Start");
}

void MainWindow::on_actionAbout_triggered()
{
    help.show();
}

void MainWindow::on_actionExit_triggered()
{
    if (isPuncturing)
    {
        if (!sureToExit(true))
            return;
    }
    this->close();
}

void MainWindow::on_actionSave_File_triggered()
{
    saveFile();
}

void MainWindow::on_actionSave_File_As_triggered()
{
    saveFile();
}
