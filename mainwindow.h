#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QTime>
#include "helpwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showTime(qint64 timeTick);

private slots:
    void on_timer_timeout();

    void on_buttonPunc_clicked();

    void on_buttonStart_clicked();


    void on_buttonStop_clicked();

    void on_actionAbout_triggered();

    void on_actionExit_triggered();

private:
    Ui::MainWindow *ui;
    HelpWindow help;
    QTimer timer;
    int timerInterval;
    int timerState;
    qint64 currentTime;
    bool isPuncturing;

    bool sureToExit(bool manualClose);

protected:
    void closeEvent(QCloseEvent *);
};

#endif // MAINWINDOW_H
