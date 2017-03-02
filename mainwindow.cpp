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
#define PUNCTOR_FILE_SUFFIX_LRC "Lyric file (*.lrc)(*.lrc)"
#define PUNCTOR_FILE_SUFFIX_SRT "Subtitle file (*.srt)(*.srt)"


bool Punctor_getSelectedItemIndex(QListWidget* list,
                                  QList<int>& indexList);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->bUDMin->setText("Min");
    ui->bUDSec->setText("Sec");
    ui->bUDMsec->setText("Msec");
    help = NULL;
    tickEditor = NULL;

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
    lastSelectedIndex = 0;
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

QString& MainWindow::tickToItemText(const TimeTick &tick)
{
    static QString content;
    content = TimeLine::timeStampToString(tick.startTime,
                                          currentFile.getTimeFormat());
    content.append("  ").append(tick.content);
    content.replace("\r\n", "↵");
    content.replace('\n', "↵");
    content.replace('\r', "↵");
    return content;
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
    QList<int> indexList;
    Punctor_getSelectedItemIndex(ui->listTimeline, indexList);

    ui->listTimeline->clear();
    int count = currentList.count();
    for(int i=0; i<count; i++)
        ui->listTimeline->addItem(tickToItemText(currentList[i]));

    QList<int>::const_iterator i;
    for(i=indexList.constBegin(); i!=indexList.constEnd(); i++)
    {
        if (*i >= ui->listTimeline->count())
            break;
        ui->listTimeline->item(*i)->setSelected(true);
    }
}

void MainWindow::updateListItem(int index)
{
    QListWidgetItem* item = ui->listTimeline->item(index);
    item->setText(tickToItemText(currentList[index]));
}

void MainWindow::moveSelectedArea(int delta)
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        return;

    QList<int>::iterator i;
    for(i=indexList.begin(); i!=indexList.end(); i++)
    {
        *i += delta;
        if (*i < 0)
            *i = 0;
        else if (*i >= ui->listTimeline->count())
            *i = ui->listTimeline->count() - 1;
    }
    indexList = indexList.toSet().toList();

    ui->listTimeline->clearSelection();
    for(i=indexList.begin(); i!=indexList.end(); i++)
    {
        ui->listTimeline->item(*i)->setSelected(true);
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
        if (help)
            help->close();
        delete help;
        if (tickEditor)
            tickEditor->close();
        delete tickEditor;
    }
    else
        event->ignore();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space)
        on_buttonPunc_clicked();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    ui->buttonPunc->resize(ui->buttonPunc->width(),
                           this->height() - ui->buttonPunc->pos().y() - 40);
    ui->listTimeline->resize(this->width() - ui->listTimeline->pos().x() - 10,
                             this->height() - ui->listTimeline->pos().y() - 40);
    ui->frameListOperation->resize(ui->listTimeline->width(),
                                   ui->frameListOperation->height());
    ui->frameListButtonRight->move(ui->frameListOperation->width() -
                                   ui->frameListButtonRight->width(), 0);
    event->accept();
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
    if (!help)
        help = new HelpWindow;
    help->show();
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
    if (currentFile.save(currentList) != FileContainer::fileOK)
    {
        QMessageBox::critical(this, "Error while saving",
                              "Cannot save file.\nPlease check "
                              "if there is enough space on the disk,\n"
                              "and you have the permission to write file.");
        return;
    }
    fileModified = false;
}

void MainWindow::on_actionSave_File_As_triggered()
{
    QString path = QFileDialog::getSaveFileName(
                    this,
                    "Save file as",
                    ".",
                    QString(PUNCTOR_FILE_SUFFIX_TXT).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_LRC).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_SRT));
    if (path.isEmpty())
        return;

    FileContainer::FileType oldType = currentFile.getFileType();
    if (currentFile.saveAs(currentList, path) != FileContainer::fileOK)
    {
        QMessageBox::critical(this,"Error while saving file as",
                              QString("Cannot save file as ")
                              .append('"').append(path).append('"')
                              .append("\nPlease check "
                              "if there is enough space on the disk,\n"
                              "and you have the permission to write file."));
        currentFile.setFileType(oldType);
        return;
    }
    fileModified = false;
    if (oldType != currentFile.getFileType())
        updateList();
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
                    QString(PUNCTOR_FILE_SUFFIX_TXT).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_LRC).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_SRT));
    if (path.isEmpty())
        return;

    FileContainer::FileErrorNumber openError;
    openError = currentFile.open(path, currentList, false);
    if (openError == FileContainer::openFail)
    {
        QMessageBox::critical(this,
                              "Error while opening",
                              "Cannot open file for reading.\n"
                              "Please check if you have the permission "
                              "to read this file.");
        return;
    }
    else if (openError == FileContainer::recordError)
    {
        if (QMessageBox::warning(this,
                                 "Error while reading file",
                                 "This file contains record with incorrect "
                                 "format.\nDo you really want to open it?",
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No) != QMessageBox::Yes)
            return;
        openError = currentFile.open(path, currentList, true);
        if (openError != FileContainer::fileOK)
        {
            QMessageBox::critical(this,
                                  "Error while opening",
                                  QString("We are not able to parse "
                                  "the content of file \n""")
                                  .append('"').append(path).append('"')
                                  .append("\nThe file may be corrupted."));
             return;
         }
    }

    fileModified = false;
    lastSelectedIndex = 0;
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
    currentFile.close();
    fileModified = false;
    lastSelectedIndex = 0;

    ui->listTimeline->clear();
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
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        return;

    QList<QListWidgetItem *> itemList(ui->listTimeline->selectedItems());
    QList<QListWidgetItem *>::const_iterator i;
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

    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        indexList.append(0);

    ui->listTimeline->insertItem(indexList.last(),
                                 TimeLine::timeStampToString(currentTime,
                                                 currentFile.getTimeFormat()));
    currentList.addItem(tick, indexList.last());
    fileModified = true;
}

void MainWindow::on_buttonListMoveUp_clicked()
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        return;

    std::sort(indexList.begin(), indexList.end());
    int c = 0;
    while (!indexList.isEmpty() && indexList.first() == c)
    {
        c++;
        indexList.removeFirst();
    }

    QList<int>::const_iterator i;
    for(i=indexList.constBegin(); i!=indexList.constEnd(); i++)
        currentList.swapItem(*i - 1, *i);

    updateList();
    moveSelectedArea(-1);
}

void MainWindow::on_buttonListMoveDown_clicked()
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        return;

    std::sort(indexList.begin(), indexList.end());
    int c = currentList.count() - 1;
    while (!indexList.isEmpty() && indexList.last() == c)
    {
        c--;
        indexList.removeLast();
    }

    QList<int>::const_iterator i;
    for(i=indexList.constEnd() - 1; i>=indexList.constBegin(); i--)
        currentList.swapItem(*i, *i + 1);

    updateList();
    moveSelectedArea(1);
}

void MainWindow::on_buttonListEdit_clicked()
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
    {
        QMessageBox::information(this, "No time tick selected",
                                 "Please select one time tick from the list "
                                 "before editing it.");
        return;
    }

    if (!tickEditor)
        tickEditor = new TickEditWindow;

    int i = indexList.first();
    TimeTick tick = currentList[i];
    tickEditor->setTimeTick(tick);
    tickEditor->timerInterval = timerInterval;
    tickEditor->timeFormat = currentFile.getTimeFormat();
    tickEditor->exec();

    tick = tickEditor->getTimeTick();
    if (tickEditor->isModified())
    {
        currentList.updateItem(tick, i);
        updateListItem(i);
    }
}

void MainWindow::on_actionCopy_triggered()
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
    {
        QMessageBox::information(this, "No time tick selected",
                                 "Please select one time tick from the list.");
        return;
    }
    if (indexList.count() > 1)
    {
        QMessageBox::warning(this, "More than one time tick selected",
                             "Please select only one time tick.");
        return;
    }
    lastSelectedIndex = indexList.first();
    isCutting = false;
}

void MainWindow::on_actionCut_triggered()
{
    on_actionCopy_triggered();
    isCutting = true;
}

void MainWindow::on_actionPaste_triggered()
{
    if (lastSelectedIndex < 0)
        return;

    int insertPos = 0;
    QList<int> indexList;
    if (Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        insertPos = indexList.first();

    TimeTick tempItem = currentList[lastSelectedIndex];
    if (isCutting)
    {
        currentList.removeItem(lastSelectedIndex);
        lastSelectedIndex = -1;
    }
    else
    {
        if (insertPos <= lastSelectedIndex)
            lastSelectedIndex++;
    }
    currentList.addItem(tempItem, insertPos);
    fileModified = true;

    updateList();
}

void MainWindow::on_actionDuplicate_triggered()
{
    on_actionCopy_triggered();
    on_actionPaste_triggered();
}

bool Punctor_getSelectedItemIndex(QListWidget* list, QList<int>& indexList)
{
    QList<QListWidgetItem *> itemList(list->selectedItems());
    if (itemList.count() < 1)
        return false;

    QList<QListWidgetItem *>::const_iterator i;
    for(i=itemList.constBegin(); i!=itemList.constEnd(); i++)
        indexList.append(list->row(*i));
    return true;
}
