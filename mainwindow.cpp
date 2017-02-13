#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "buttonupdown.h"
#include <QCloseEvent>
#include <QLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QFileDialog>

#define PUNCTOR_DISPLAY_TITLE_MAIN "Mini Punctor"
#define PUNCTOR_DISPLAY_TIME_FORMAT "mm : ss . zzz"

#define PUNCTOR_FILE_SUFFIX_TXT "Plain Text (*.txt)(*.txt)"


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
    updateTitle("");

    isPuncturing = false;
    fileModified = false;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showTime(qint64 timeTick)
{
    ui->labelTime->setText(TimeLine::timeStampToString(
                               timeTick, PUNCTOR_DISPLAY_TIME_FORMAT));
}


void MainWindow::updateTitle(QString fileName)
{
    if (!fileName.isEmpty())
    {
        if (fileName.length() > 16)
        {
            int p = fileName.lastIndexOf('/');
            if (p < 0)
                p = fileName.lastIndexOf('\\');
            if (p >= 0)
                fileName = fileName.mid(p + 1);
        }
        fileName.append(" - ");
    }
    this->setWindowTitle(fileName.append(PUNCTOR_DISPLAY_TITLE_MAIN));
}

void MainWindow::updateList()
{
    ui->listTimeline->clear();
    int count = currentList.count();
    QString content;
    for(int i=0; i<count; i++)
    {
        content = TimeLine::timeStampToString(currentList[i].startTime,
                                              currentFile.getTimeFormat());
        content.append("  ").append(currentList[i].content);
        ui->listTimeline->addItem(content);
    }
}

bool MainWindow::sureToExit(bool manualClose)
{
    if (!manualClose && isPuncturing)
    {
        if (QMessageBox::question(this,"Quit MiniPunctor",
                              "The program is puncturing now.\n"
                              "Quit anyway?",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
            return false;
    }

    if (fileModified)
    {
        QMessageBox::StandardButton button =
            QMessageBox::warning(this,"File not saved",
                                 "You have unsaved changes.\n"
                                 "Save it before quit?",
                                 QMessageBox::Yes |
                                 QMessageBox::No |
                                 QMessageBox::Cancel,
                                 QMessageBox::No);
        if (button == QMessageBox::Cancel)
            return false;
        else if (button == QMessageBox::Yes)
        {
            on_actionSave_File_triggered();
            if (fileModified)
                return false;
        }
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

void MainWindow::on_bUDMsec_clicked(int button)
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
    int i = currentList.addItemByTime(currentTime);
    ui->listTimeline->insertItem(i,
                                 TimeLine::timeStampToString(
                                    currentTime, currentFile.getTimeFormat()));
    fileModified = true;
}

void MainWindow::on_buttonStart_clicked()
{
    switch (timerState) {
    case 0: //Stop
        currentTime = 0;
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
    if (currentFile.getFilePath().isEmpty())
    {
        on_actionSave_File_As_triggered();
        return;
    }
    currentFile.save(currentList);
    fileModified = false;
}

void MainWindow::on_actionSave_File_As_triggered()
{
    QString path = QFileDialog::getSaveFileName(
                    this,
                    "Save file as",
                    ".",
                    PUNCTOR_FILE_SUFFIX_TXT);
    if (path.isEmpty())
        return;

    currentFile.saveAs(currentList, path);
    fileModified = false;
    updateTitle(path);
}

void MainWindow::on_actionOpen_File_triggered()
{
    if (fileModified)
    {
        QMessageBox::StandardButton button =
            QMessageBox::warning(this,"File not saved",
                                 "You have unsaved changes.\n"
                                 "Save it before open new file?",
                                 QMessageBox::Yes |
                                 QMessageBox::No |
                                 QMessageBox::Cancel,
                                 QMessageBox::No);
        if (button == QMessageBox::Cancel)
                return;
        else if (button == QMessageBox::Yes)
        {
            on_actionSave_File_triggered();
            if (fileModified)
                return;
        }
    }

    QString path = QFileDialog::getOpenFileName(
                    this,
                    "Open file",
                    ".",
                    PUNCTOR_FILE_SUFFIX_TXT);
    if (path.isEmpty())
        return;

    currentFile.open(path, currentList, true);
    fileModified = false;
    updateList();
    updateTitle(path);
}

void MainWindow::on_actionNew_File_triggered()
{
    if (fileModified)
    {
        QMessageBox::StandardButton button =
            QMessageBox::warning(this,"File not saved",
                                 "You have unsaved changes.\n"
                                 "Save it before create new file?",
                                 QMessageBox::Yes |
                                 QMessageBox::No |
                                 QMessageBox::Cancel,
                                 QMessageBox::No);
        if (button == QMessageBox::Cancel)
                return;
        else if (button == QMessageBox::Yes)
        {
            on_actionSave_File_triggered();
            if (fileModified)
                return;
        }
    }
    currentList.clear();
    ui->listTimeline->clear();
    currentFile.close();
    fileModified = false;
}

void MainWindow::on_buttonListClear_clicked()
{
    if (currentList.count() < 1)
        return;
    if (QMessageBox::question(this,"Clear all entries",
                              "Do you really want to clear the list?\n",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) != QMessageBox::Yes)
            return;
    ui->listTimeline->clear();
    currentList.clear();
    fileModified = true;
}

void MainWindow::on_buttonListRemove_clicked()
{
    QList<int> indexList;
    QList<QListWidgetItem *> itemList = ui->listTimeline->selectedItems();
    if (itemList.count() < 1)
        return;

    QList<QListWidgetItem *>::const_iterator i;
    for(i=itemList.constBegin(); i!=itemList.constEnd(); i++)
        indexList.append(ui->listTimeline->row(*i));

    for(i=itemList.constBegin(); i!=itemList.constEnd(); i++)
    {
        ui->listTimeline->removeItemWidget(*i);
        delete *i;
    }

    QList<int>::const_iterator j;
    for(j=indexList.constEnd() - 1; j>=indexList.constBegin(); j--)
    {
        currentList.removeItem(*j);
    }
    fileModified = true;
}

void MainWindow::on_buttonListInsert_clicked()
{
    TimeTick tick;
    tick.startTime = tick.endTime = currentTime;
    int i;

    QList<QListWidgetItem *> itemList = ui->listTimeline->selectedItems();
    if (itemList.isEmpty())
        i = 0;
    else
        i = ui->listTimeline->row(itemList.last());

    ui->listTimeline->insertItem(i,TimeLine::timeStampToString(
                                     currentTime, currentFile.getTimeFormat()));
    currentList.addItem(tick, i);
    fileModified = true;
}
