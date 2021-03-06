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

#define PUNCTOR_FILE_SUFFIX_ALL "All (*.*)(*.*)"
#define PUNCTOR_FILE_SUFFIX_TXT "Plain Text (*.txt)(*.txt)"
#define PUNCTOR_FILE_SUFFIX_LRC "Lyric file (*.lrc)(*.lrc)"
#define PUNCTOR_FILE_SUFFIX_SRT "Subtitle file (*.srt)(*.srt)"
#define PUNCTOR_FILE_SUFFIX_SMI "Windows Media Player Subtitle (*.smi)(*.smi)"

#define PUNCTOR_TIMER_STATE_STOP        0
#define PUNCTOR_TIMER_STATE_COUNTING    1
#define PUNCTOR_TIMER_STATE_PAUSE       2


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
    resize(500, 300);

    help = NULL;
    tickEditor = NULL;
    tickShifter = NULL;
    search = NULL;
    menuTimeline = NULL;

    connect(&timer,
            SIGNAL(timeout()),
            this,
            SLOT(onTimerTimeout()));

    timerState = PUNCTOR_TIMER_STATE_STOP;
    timerInterval = 100;
    timer.setInterval(timerInterval);
    playerSyncShift = 0;

    currentTime = 0;
    showTime(currentTime);
    updateTitle("");

    lastOpeningPath = ".";
    lastSavingPath = ".";

    fileModified = false;
    followTimer = true;
    synchronized = true;
    lastSelectedIndex = -1;
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
    static QString tickContent;
    content = TimeLine::timeStampToString(tick.startTime,
                                          currentFile.getTimeFormat());
    tickContent = tick.content;
    tickContent.replace("\r\n", "\n")
               .replace('\n', "↵")
               .replace('\r', " ")
               .replace("<br>", "↵")
               .replace("&nbsp;", " ");
    return content.append("  ").append(tickContent);
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
    if (!manualClose && timerState == PUNCTOR_TIMER_STATE_COUNTING)
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

int MainWindow::searchText(const QString& str, int lastIndex)
{
    if (lastIndex < 0)
        lastIndex = 0;

    while (++lastIndex < currentList.count())
    {
        if (currentList[lastIndex].content.indexOf(str) >= 0)
            break;
    }

    if (lastIndex < currentList.count())
    {
        ui->listTimeline->clearSelection();
        ui->listTimeline->item(lastIndex)->setSelected(true);
        ui->listTimeline->scrollToItem(ui->listTimeline->item(lastIndex));
        return lastIndex;
    }
    else
        return -1;
}

void MainWindow::shiftSelectedTicks(qint64 value)
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
    {
        QMessageBox::information(this, "No time tick selected",
                                 "Please select one time tick from the list "
                                 "before editing it.");
        return;
    }

    TimeTick tick;
    QList<int>::const_iterator i;
    for(i=indexList.constBegin(); i!=indexList.constEnd(); i++)
    {
        tick = currentList[*i];
        tick.startTime += value;
        tick.endTime += value;
        currentList.updateItem(tick, *i);
    }
    setFileModified(true);
    updateList();
}

void MainWindow::setFileModified(bool modified)
{
    fileModified = modified;
    if (modified && !currentFile.getFilePath().isEmpty())
        updateTitle(currentFile.getFilePath().append(" * "));
    else
        updateTitle(currentFile.getFilePath());
}

void MainWindow::startTimer(bool reset)
{
    if (timerState == PUNCTOR_TIMER_STATE_STOP || reset)
    {
        currentTime = 0;
        if (synchronized)
            currentTime = playerSyncShift;
    }

    timer.start();
    timerState = PUNCTOR_TIMER_STATE_COUNTING;
    ui->buttonStart->setText("Pause");
}

void MainWindow::stopTimer()
{
    timer.stop();
    timerState = PUNCTOR_TIMER_STATE_STOP;
    ui->buttonStart->setText("Start");
}

void MainWindow::pauseTimer()
{
    timer.stop();
    timerState = PUNCTOR_TIMER_STATE_PAUSE;
    ui->buttonStart->setText("Resume");
}

bool MainWindow::playerExists()
{
    if (bindedPlayer.isEmpty())
    {
        QMessageBox::warning(this, "No player binded",
                             "The timer is not binded to any media player.\n"
                             "Please choose a player from \"Player\" menu.");
        return false;
    }
    else
        return true;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (sureToExit(false))
    {
        if (help)
        {
            help->close();
            delete help;
        }
        if (tickEditor)
        {
            tickEditor->close();
            delete tickEditor;
        }
        if (tickShifter)
        {
            tickShifter->close();
            delete tickShifter;
        }
        if (search)
        {
            search->close();
            delete search;
        }
        if (menuTimeline)
            delete menuTimeline;
    }
    else
        event->ignore();
}

void MainWindow::keyReleaseEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space &&
        event->modifiers() == Qt::NoModifier)
        on_buttonPunc_clicked();
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    if (height() < 400)
    {
        ui->framePlayerCtrl->setVisible(false);
        ui->buttonStart->move(10, 140);
        ui->buttonStop->move(120, 140);
        ui->buttonPunc->move(10, 184);       
    }
#ifndef Q_OS_WIN
    else
    {
        ui->framePlayerCtrl->setVisible(true);
        ui->buttonStart->move(10, 180);
        ui->buttonStop->move(120, 180);
        ui->buttonPunc->move(10, 224);
    }
#endif
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

void MainWindow::on_bUDMin_clicked(int button)
{
    int offset;
    if (button)
        offset = -60000;
    else
        offset = 60000;

    currentTime += offset;
    if (currentTime < 0)
        currentTime = 0;

#ifndef Q_OS_WIN
    if (!bindedPlayer.isEmpty() && synchronized)
    {
        player.seek(bindedPlayer, offset);
        currentTime = player.getPosition(bindedPlayer) + playerSyncShift;
    }
#endif

    showTime(currentTime);
}

void MainWindow::on_bUDSec_clicked(int button)
{
    int offset;
    if (button)
        offset = -1000;
    else
        offset = 1000;

    currentTime += offset;
    if (currentTime < 0)
        currentTime = 0;

#ifndef Q_OS_WIN
    if (!bindedPlayer.isEmpty() && synchronized)
    {
        player.seek(bindedPlayer, offset);
        currentTime = player.getPosition(bindedPlayer) + playerSyncShift;
    }
#endif

    showTime(currentTime);
}

void MainWindow::on_bUDMsec_clicked(int button)
{
    int offset;
    if (button)
        offset = -timerInterval;
    else
        offset = timerInterval;

    currentTime += offset;
    if (currentTime < 0)
        currentTime = 0;

#ifndef Q_OS_WIN
    if (!bindedPlayer.isEmpty() && synchronized)
    {
        player.seek(bindedPlayer, offset);
        currentTime = player.getPosition(bindedPlayer) + playerSyncShift;
    }
#endif

    showTime(currentTime);
}

void MainWindow::on_buttonPunc_clicked()
{
    int i = currentList.addItemByTime(currentTime);
    ui->listTimeline->insertItem(i,
                                 TimeLine::timeStampToString(
                                    currentTime, currentFile.getTimeFormat()));
    setFileModified(true);
}

void MainWindow::on_buttonStart_clicked()
{
    switch (timerState) {
    case PUNCTOR_TIMER_STATE_STOP:
    case PUNCTOR_TIMER_STATE_PAUSE:
#ifndef Q_OS_WIN
        if (!bindedPlayer.isEmpty()  && synchronized)
            player.play(bindedPlayer);
        ui->buttonPlayerPlay->setIcon(QIcon(":/icons/media-pause.png"));
#endif
        startTimer(false);
        break;
    case PUNCTOR_TIMER_STATE_COUNTING:
        pauseTimer();
#ifndef Q_OS_WIN
        if (!bindedPlayer.isEmpty() && synchronized)
            player.pause(bindedPlayer);
        ui->buttonPlayerPlay->setIcon(QIcon(":/icons/media-play.png"));
#endif
    default:
        break;
    }
}

void MainWindow::on_buttonStop_clicked()
{
    stopTimer();
#ifndef Q_OS_WIN
    if (!bindedPlayer.isEmpty() && synchronized)
        player.stop(bindedPlayer);
#endif
}

void MainWindow::on_actionAbout_triggered()
{
    if (!help)
        help = new HelpWindow;
    help->show();
}

void MainWindow::on_actionExit_triggered()
{
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
    setFileModified(false);
}

void MainWindow::on_actionSave_File_As_triggered()
{
    if (lastSavingFilter.isEmpty())
        lastSavingFilter = lastOpeningFilter;
    QString path = QFileDialog::getSaveFileName(
                    this,
                    "Save file as",
                    lastSavingPath,
                    QString(PUNCTOR_FILE_SUFFIX_TXT).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_LRC).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_SRT).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_SMI),
                    &lastSavingFilter);
    if (path.isEmpty())
        return;
    lastSavingPath = path;

    FileContainer::FileType oldType = currentFile.getFileType();
    if (currentFile.saveAs(currentList, path) != FileContainer::fileOK)
    {
        QMessageBox::critical(this,"Error while saving file as",
                              QString("Cannot save file as \"%1\""
                              "\nPlease check "
                              "if there is enough space on the disk,\n"
                              "and you have the permission to write file.")
                              .arg(path));
        currentFile.setFileType(oldType);
        return;
    }
    setFileModified(false);
    if (oldType != currentFile.getFileType())
        updateList();
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
                    lastOpeningPath,
                    QString(PUNCTOR_FILE_SUFFIX_ALL).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_TXT).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_LRC).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_SRT).append(";;")
                    .append(PUNCTOR_FILE_SUFFIX_SMI),
                    &lastOpeningFilter);
    if (path.isEmpty())
        return;
    lastOpeningPath = path;

    QString oldTitle = windowTitle();
    FileContainer::FileErrorNumber openError;
    setWindowTitle(QString("Loading file %1 ...").arg(path));
    openError = currentFile.open(path, currentList, false);
    setWindowTitle(oldTitle);
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
                                  "the content of file \n"
                                  "\"%1\"\n"
                                  "The file may be corrupted.").arg(path));
             return;
         }
    }

    lastSelectedIndex = 0;
    setFileModified(false);
    updateList();
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
    setFileModified(false);
    lastSelectedIndex = 0;

    ui->listTimeline->clear();
}

void MainWindow::on_buttonListRemove_clicked()
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        return;
    if (indexList.count() > 1)
    {
        if (QMessageBox::question(this, "Remove entries",
                                  QString().sprintf("Do you really want to "
                                                    "remove %d entries?\n",
                                                    indexList.count()),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) != QMessageBox::Yes)
                    return;
    }

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
    setFileModified(true);
}

void MainWindow::on_buttonListInsert_clicked()
{
    on_actionCreate_tick_behind_triggered();
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
        setFileModified(true);
        updateListItem(i);
    }
}

void MainWindow::on_buttonFollowTimer_clicked()
{
    ui->actionFollow_timer->trigger();
}

void MainWindow::on_listTimeline_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;
    on_buttonListEdit_clicked();
}

void MainWindow::on_listTimeline_customContextMenuRequested(const QPoint &pos)
{
    if (pos.isNull())
        return;

    if (menuTimeline == NULL)
    {
        menuTimeline = new QMenu(ui->listTimeline);

        QAction* action = new QAction("&Cut", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(on_actionCut_triggered()));
        menuTimeline->addAction(action);        
        action = new QAction("&Copy", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(on_actionCopy_triggered()));
        menuTimeline->addAction(action);        
        action = new QAction("&Paste", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(on_actionPaste_triggered()));
        menuTimeline->addAction(action);        

        menuTimeline->addSeparator();

        action = new QAction("Insert &before", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(on_actionInsert_before_triggered()));
        menuTimeline->addAction(action);
        action = new QAction("Insert &after", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(on_actionCreate_tick_behind_triggered()));
        menuTimeline->addAction(action);

        menuTimeline->addSeparator();

        action = new QAction("&Adjust selected", this);
        connect(action, SIGNAL(triggered(bool)),
                this, SLOT(on_actionAdjust_selected_triggered()));
        menuTimeline->addAction(action);
    }
    menuTimeline->exec(QCursor::pos());
}

#ifndef Q_OS_WIN
void MainWindow::on_menuBind_a_player_aboutToShow()
{
    int i;
    QList<QAction*> actionList = ui->menuBind_a_player->actions();

    for (i=0; i<actionList.count(); i++)
        ui->menuBind_a_player->removeAction(actionList[i]);

    int count = player.getPlayerList(playerNames, playerIDs);
    if (count < 1)
    {
        ui->menuBind_a_player->addAction("(No player detected)");
        return;
    }

    QAction* action;
    for (i=0; i<playerNames.count(); i++)
    {
        action = new QAction(playerNames[i], ui->menuBind_a_player);
        action->setCheckable(true);
        if (bindedPlayer == playerIDs[i])
            action->setChecked(true);
        ui->menuBind_a_player->addAction(action);
    }
}

void MainWindow::on_menuBind_a_player_triggered(QAction* action)
{
    for (int i=0; i<playerNames.count(); i++)
    {
        if (playerNames[i] == action->text())
        {
            if (action->isChecked())
                bindedPlayer = playerIDs[i];
            else
                bindedPlayer.clear();
            break;
        }
    }
}
#endif

bool MainWindow::on_actionCopy_triggered()
{
    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
    {
        QMessageBox::information(this, "No time tick selected",
                                 "Please select one time tick from the list.");
        return false;
    }
    if (indexList.count() > 1)
    {
        QMessageBox::warning(this, "More than one time tick selected",
                             "Please select only one time tick.");
        return false;
    }
    lastSelectedIndex = indexList.first();
    isCutting = false;
    return true;
}

bool MainWindow::on_actionCut_triggered()
{
    if (on_actionCopy_triggered())
    {
        isCutting = true;
        return true;
    }
    else
        return false;
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
    setFileModified(true);

    updateList();
}

void MainWindow::on_actionDuplicate_triggered()
{
    int oldSelectedIndex = lastSelectedIndex;
    if (on_actionCopy_triggered())
        on_actionPaste_triggered();
    lastSelectedIndex = oldSelectedIndex;
}

void MainWindow::on_actionInsert_before_triggered()
{
    TimeTick tick;
    tick.startTime = tick.endTime = currentTime;

    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        indexList.append(0);

    ui->listTimeline->insertItem(indexList.last(),
                                 TimeLine::timeStampToString(currentTime,
                                                 currentFile.getTimeFormat()));
    currentList.addItem(tick, indexList.first());
    setFileModified(true);
}

void MainWindow::on_actionCreate_tick_behind_triggered()
{
    TimeTick tick;
    tick.startTime = tick.endTime = currentTime;

    QList<int> indexList;
    if (!Punctor_getSelectedItemIndex(ui->listTimeline, indexList))
        indexList.append(ui->listTimeline->count());

    ui->listTimeline->insertItem(indexList.last() + 1,
                                 TimeLine::timeStampToString(currentTime,
                                                 currentFile.getTimeFormat()));
    currentList.addItem(tick, indexList.last() + 1);
    setFileModified(true);
}

void MainWindow::on_actionSelect_all_triggered()
{
    ui->listTimeline->selectAll();
}

void MainWindow::on_actionReverse_selection_triggered()
{
    for (int i=0; i<ui->listTimeline->count(); i++)
        ui->listTimeline->item(i)->setSelected(
                    !ui->listTimeline->item(i)->isSelected());
}

void MainWindow::on_actionDelete_triggered()
{
    if (!ui->listTimeline->hasFocus())
        return;
    on_buttonListRemove_clicked();
}
void MainWindow::on_actionFind_Replace_triggered()
{
    if (!search)
        search = new SearchWindow(this);
    search->show();
}

void MainWindow::on_actionSort_by_start_time_triggered()
{
    currentList.sort(PUNCTOR_TIME_FIELD_START);
    setFileModified(true);
    updateList();
}

void MainWindow::on_actionSort_by_end_time_triggered()
{
    currentList.sort(PUNCTOR_TIME_FIELD_END);
    setFileModified(true);
    updateList();

}

void MainWindow::on_actionSort_by_tick_ID_triggered()
{
    currentList.sort(PUNCTOR_TIME_FIELD_ID);
    setFileModified(true);
    updateList();
}

void MainWindow::on_actionAdjust_selected_triggered()
{
    if (!tickShifter)
        tickShifter = new TickShiftWindow(this);
    tickShifter->setPrompt("Shift selected tick with:");
    tickShifter->exec();
    if (tickShifter->accepted)
    {
        shiftSelectedTicks(tickShifter->shiftValue);
        setFileModified(true);
    }
}

void MainWindow::on_actionAdjust_all_ticks_triggered()
{
    if (!tickShifter)
        tickShifter = new TickShiftWindow(this);
    tickShifter->exec();
    if (tickShifter->accepted && ui->listTimeline->count() > 0)
    {
        ui->listTimeline->selectAll();
        shiftSelectedTicks(tickShifter->shiftValue);
        setFileModified(true);
    }
}

void MainWindow::on_actionMove_up_triggered()
{
    on_buttonListMoveUp_clicked();
}

void MainWindow::on_actionMove_down_triggered()
{
    on_buttonListMoveDown_clicked();
}

void MainWindow::onTimelineSearch(QString searched, int &lastIndex)
{
    lastIndex = searchText(searched, lastIndex);

    if (lastIndex == -1)
        QMessageBox::information(this,
                                 "Not found",
                                 "Searching finished. No matched found.");
}

void MainWindow::onTimelineReplace(QString searched,
                                   QString newContent,
                                   int& lastIndex)
{
    onTimelineSearch(searched, lastIndex);
    if (lastIndex < 0) return;

    TimeTick tick = currentList[lastIndex];
    tick.content.replace(searched, newContent);
    currentList.updateItem(tick, lastIndex);
    setFileModified(true);

    updateListItem(lastIndex);
}

void MainWindow::onTimelineReplaceAll(QString searched,
                                      QString newContent,
                                      int& lastIndex)
{
    int i = lastIndex, count = 0;
    while(true)
    {
        i = searchText(searched, i);
        if (i == -1)
            break;
        TimeTick tick = currentList[i];
        tick.content.replace(searched, newContent);
        currentList.updateItem(tick, i);
        count++;
    }
    if (count > 0)
        setFileModified(true);

    updateList();
    lastIndex = i;
    QMessageBox::information(this,
                             "Replace All",
                             QString().sprintf("Finish replacing. "
                                              "%d items replaced.",
                                              count),
                             QMessageBox::Ok);
}

void MainWindow::onTimerTimeout()
{
    currentTime += timerInterval;
    showTime(currentTime);

    if (!followTimer)
        return;

    // Get all ticks that correspond to current time
    QList<int> currentTicks;
    currentList.getIndexesByTime(currentTime,
                                 timerInterval,
                                 currentTicks);
    int count = currentTicks.count();
    if (count > 0)
    {
        ui->listTimeline->clearSelection();
        for (int i=0; i<count; i++)
            ui->listTimeline->item(currentTicks[i])->setSelected(true);

        // Scroll to the last tick in "currentTicks"
        ui->listTimeline->scrollToItem(
                    ui->listTimeline->item(currentTicks[count - 1]));
    }
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

void MainWindow::on_actionFollow_timer_triggered()
{
    followTimer = ui->actionFollow_timer->isChecked();
}

#ifndef Q_OS_WIN
void MainWindow::on_buttonPlayerPlay_clicked()
{
    on_actionPlay_Pause_triggered();
}

void MainWindow::on_buttonPlayerStop_clicked()
{
    on_actionStop_triggered();
}

void MainWindow::on_buttonPlayerPrevious_clicked()
{
    on_actionPrevious_triggered();
}

void MainWindow::on_buttonPlayerNext_clicked()
{
    on_actionNext_triggered();
}

void MainWindow::on_actionSynchronize_with_player_triggered()
{
    synchronized = ui->actionSynchronize_with_player->isChecked();
}

void MainWindow::on_actionShift_synchronization_triggered()
{
    if (!tickShifter)
    {
        tickShifter = new TickShiftWindow;
    }
    tickShifter->setPrompt("Set player sync. shift:");
    tickShifter->exec();
    if (tickShifter->accepted)
        playerSyncShift = tickShifter->shiftValue;
}

void MainWindow::on_actionPlay_Pause_triggered()
{
    if (!playerExists())
        return;
    if (player.getStatus(bindedPlayer) == MediaControl::status_playing)
    {
        player.pause(bindedPlayer);
        if (synchronized)
            pauseTimer();
        ui->buttonPlayerPlay->setIcon(QIcon(":/icons/media-play.png"));
    }
    else
    {
        player.play(bindedPlayer);
        if (synchronized)
            startTimer(false);
        ui->buttonPlayerPlay->setIcon(QIcon(":/icons/media-pause.png"));
    }
}

void MainWindow::on_actionStop_triggered()
{
    if (!playerExists())
        return;
    player.stop(bindedPlayer);

    if (synchronized)
        stopTimer();

    ui->buttonPlayerPlay->setIcon(QIcon(":/icons/media-play.png"));
}

void MainWindow::on_actionPrevious_triggered()
{
    if (!playerExists())
        return;
    player.previous(bindedPlayer);

    if (synchronized)
        startTimer(true);

    ui->buttonPlayerPlay->setIcon(QIcon(":/icons/media-pause.png"));
}

void MainWindow::on_actionNext_triggered()
{
    if (!playerExists())
        return;
    player.next(bindedPlayer);

    if (synchronized)
        startTimer(true);

    ui->buttonPlayerPlay->setIcon(QIcon(":/icons/media-pause.png"));
}
#endif
