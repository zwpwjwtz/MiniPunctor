#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "helpwindow.h"
#include "filecontainer.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_timer_timeout();

    void on_bUDMin_clicked(int button);
    void on_bUDSec_clicked(int button);
    void on_bUDMsec_clicked(int button);

    void on_buttonPunc_clicked();
    void on_buttonStart_clicked();
    void on_buttonStop_clicked();
    
    void on_buttonListClear_clicked();
    void on_buttonListRemove_clicked();
    void on_buttonListInsert_clicked();

    void on_actionAbout_triggered();
    void on_actionExit_triggered();
    void on_actionSave_File_triggered();
    void on_actionSave_File_As_triggered();
    void on_actionOpen_File_triggered();
    void on_actionNew_File_triggered();

private:
    Ui::MainWindow *ui;
    HelpWindow help;
    QTimer timer;
    int timerInterval;
    int timerState;
    qint64 currentTime;
    bool isPuncturing;
    bool fileModified;
    FileContainer currentFile;
    TimeLine currentList;

    void showTime(qint64 timeTick);
    void updateTitle(QString fileName);
    void updateList();
    bool sureToExit(bool manualClose);

protected:
    void closeEvent(QCloseEvent *);
    void keyReleaseEvent(QKeyEvent *event);
};

#endif // MAINWINDOW_H
