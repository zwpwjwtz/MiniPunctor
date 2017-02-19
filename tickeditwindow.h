#ifndef TICKEDITWINDOW_H
#define TICKEDITWINDOW_H

#include <QDialog>
#include <QAbstractButton>
#include "timeline.h"

namespace Ui {
class TickEditWindow;
}

class TickEditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TickEditWindow(QWidget *parent = 0);
    ~TickEditWindow();
    TimeTick getTimeTick();
    void setTimeTick(TimeTick newTick);
    bool isModified();
    int timerInterval;
    QString timeFormat;

private slots:
    void on_bUDMin_clicked(int button);
    void on_bUDSec_clicked(int button);
    void on_bUDMsec_clicked(int button);

    void on_textTimeStart_editingFinished();
    void on_textTimeEnd_editingFinished();
    void on_buttonBox_accepted();
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::TickEditWindow *ui;
    TimeTick tick;
    int lastModified;
    bool tickModified;
    bool verifyTime(QString newTime, bool& reset);
    void reloadTimTick();
    void shiftTime(qint64 delta);

protected:
    bool eventFilter(QObject* watched, QEvent* event);
    void hideEvent(QHideEvent* event);
    void showEvent(QShowEvent* event);

};

#endif // TICKEDITWINDOW_H
