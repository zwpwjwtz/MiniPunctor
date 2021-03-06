#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QTimer>
#include "helpwindow.h"
#include "tickeditwindow.h"
#include "tickshiftwindow.h"
#include "filecontainer.h"
#include "searchwindow.h"
#ifndef Q_OS_WIN
#include "mediacontrol.h"
#endif

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();    

public slots:
    void onTimelineSearch(QString searched, int& lastIndex);
    void onTimelineReplace(QString searched,
                             QString newContent,
                             int& lastIndex);
    void onTimelineReplaceAll(QString searched,
                                 QString newContent,
                                 int& lastIndex);
private slots:
    void onTimerTimeout();

    void on_bUDMin_clicked(int button);
    void on_bUDSec_clicked(int button);
    void on_bUDMsec_clicked(int button);

    void on_buttonPunc_clicked();
    void on_buttonStart_clicked();
    void on_buttonStop_clicked();    

    void on_buttonListMoveUp_clicked();
    void on_buttonListMoveDown_clicked();
    void on_buttonListEdit_clicked();
    void on_buttonListRemove_clicked();
    void on_buttonListInsert_clicked();
    void on_buttonFollowTimer_clicked();

    void on_listTimeline_doubleClicked(const QModelIndex &index);
    void on_listTimeline_customContextMenuRequested(const QPoint &pos);

    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void on_actionSave_File_triggered();
    void on_actionSave_File_As_triggered();
    void on_actionOpen_File_triggered();
    void on_actionNew_File_triggered();
    bool on_actionCopy_triggered();
    bool on_actionCut_triggered();
    void on_actionPaste_triggered();
    void on_actionDuplicate_triggered();
    void on_actionInsert_before_triggered();
    void on_actionCreate_tick_behind_triggered();    
    void on_actionSelect_all_triggered();
    void on_actionReverse_selection_triggered();
    void on_actionDelete_triggered();
    void on_actionFind_Replace_triggered();
    void on_actionSort_by_start_time_triggered();
    void on_actionSort_by_end_time_triggered();
    void on_actionSort_by_tick_ID_triggered();
    void on_actionAdjust_selected_triggered();
    void on_actionAdjust_all_ticks_triggered();
    void on_actionMove_up_triggered();
    void on_actionMove_down_triggered();
    void on_actionFollow_timer_triggered();
#ifndef Q_OS_WIN
    void on_buttonPlayerPlay_clicked();
    void on_buttonPlayerStop_clicked();
    void on_buttonPlayerPrevious_clicked();
    void on_buttonPlayerNext_clicked();

    void on_actionSynchronize_with_player_triggered();
    void on_actionShift_synchronization_triggered();
    void on_actionPlay_Pause_triggered();
    void on_actionStop_triggered();
    void on_actionPrevious_triggered();
    void on_actionNext_triggered();

    void on_menuBind_a_player_aboutToShow();
    void on_menuBind_a_player_triggered(QAction* action);
#endif

private:
    Ui::MainWindow *ui;
    HelpWindow* help;
    TickEditWindow* tickEditor;
    TickShiftWindow* tickShifter;
    SearchWindow* search;
    QMenu* menuTimeline;
#ifndef Q_OS_WIN
    MediaControl player;
#endif
    QTimer timer;
    FileContainer currentFile;
    TimeLine currentList;
    int timerInterval;
    int timerState;
    qint64 currentTime;
    int lastSelectedIndex;
    int playerSyncShift;
    QString lastOpeningPath;
    QString lastSavingPath;
    QString lastOpeningFilter;
    QString lastSavingFilter;
    QString bindedPlayer;
    QList<QString> playerIDs;
    QList<QString> playerNames;
    bool isCutting;
    bool fileModified;
    bool followTimer;
    bool synchronized;

    void showTime(qint64 timeTick);
    QString& tickToItemText(const TimeTick &tick);
    void updateTitle(QString fileName);
    void updateList();
    void updateListItem(int index);
    void moveSelectedArea(int delta);
    bool sureToExit(bool manualClose);
    int searchText(const QString &str, int lastIndex);
    void shiftSelectedTicks(qint64 value);
    void setFileModified(bool modified);
    void startTimer(bool reset);
    void stopTimer();
    void pauseTimer();
    bool playerExists();

protected:
    void closeEvent(QCloseEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
};

#endif // MAINWINDOW_H
