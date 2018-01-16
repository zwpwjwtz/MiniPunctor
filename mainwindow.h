#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "helpwindow.h"
#include "tickeditwindow.h"
#include "filecontainer.h"
#include "searchwindow.h"

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
    void on_timeline_search(QString searched, int& lastIndex);
    void on_timeline_replace(QString searched,
                             QString newContent,
                             int& lastIndex);
    void on_timeline_replace_all(QString searched,
                                 QString newContent,
                                 int& lastIndex,
                                 int& replaceCount);
private slots:
    void on_timer_timeout();

    void on_bUDMin_clicked(int button);
    void on_bUDSec_clicked(int button);
    void on_bUDMsec_clicked(int button);

    void on_buttonPunc_clicked();
    void on_buttonStart_clicked();
    void on_buttonStop_clicked();    

    void on_buttonListMoveUp_clicked();
    void on_buttonListMoveDown_clicked();
    void on_buttonListEdit_clicked();
    void on_buttonListClear_clicked();
    void on_buttonListRemove_clicked();
    void on_buttonListInsert_clicked();

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

private:
    Ui::MainWindow *ui;
    HelpWindow* help;
    TickEditWindow* tickEditor;
    SearchWindow* search;
    QMenu* menuTimeline;
    QTimer timer;
    FileContainer currentFile;
    TimeLine currentList;
    int timerInterval;
    int timerState;
    qint64 currentTime;
    int lastSelectedIndex;
    QString lastOpeningPath;
    QString lastSavingPath;
    QString lastOpeningFilter;
    QString lastSavingFilter;
    bool isPuncturing;
    bool isCutting;
    bool fileModified;

    void showTime(qint64 timeTick);
    QString& tickToItemText(const TimeTick &tick);
    void updateTitle(QString fileName);
    void updateList();
    void updateListItem(int index);
    void moveSelectedArea(int delta);
    bool sureToExit(bool manualClose);

protected:
    void closeEvent(QCloseEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent* event);
};

#endif // MAINWINDOW_H
