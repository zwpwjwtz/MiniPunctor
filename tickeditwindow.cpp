#include "tickeditwindow.h"
#include "ui_tickeditwindow.h"
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QLineEdit>
#include <QPlainTextEdit>

#define PUNCTOR_EDITOR_FIELD_NULL 0
#define PUNCTOR_EDITOR_FIELD_STARTTIME 1
#define PUNCTOR_EDITOR_FIELD_ENDTIME 2

#define PUNCTOR_EDITOR_TIMESTAMP_SAMPLE 45296789


TickEditWindow::TickEditWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TickEditWindow)
{
    ui->setupUi(this);
    ui->textID->installEventFilter(this);
    ui->textTimeStart->installEventFilter(this);
    ui->textTimeEnd->installEventFilter(this);
    ui->textContent->installEventFilter(this);
    ui->bUDMin->setText("Min");
    ui->bUDSec->setText("Sec");
    ui->bUDMsec->setText("MSec");
    ui->frameTimeAdj->setVisible(false);

    timerInterval = 100;
    lastModified = PUNCTOR_EDITOR_FIELD_NULL;
    tickModified = false;
}

TickEditWindow::~TickEditWindow()
{
    delete ui;
}

TimeTick TickEditWindow::getTimeTick()
{
    return tick;
}

void TickEditWindow::setTimeTick(TimeTick newTick)
{
    tick = newTick;
}

bool TickEditWindow::isModified()
{
    return tickModified;
}

bool TickEditWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusIn)
    {
        if (watched == ui->textTimeStart || watched == ui->textTimeEnd)
        {
            ui->frameTimeAdj->show();
            if (watched == ui->textTimeStart)
            {
                lastModified = PUNCTOR_EDITOR_FIELD_STARTTIME;
                ui->frameTimeAdj->move(70, ui->frameTimeAdj->pos().y());
            }
            else
            {
                lastModified = PUNCTOR_EDITOR_FIELD_ENDTIME;
                ui->frameTimeAdj->move(180, ui->frameTimeAdj->pos().y());
            }
        }
        else
        {
            lastModified = 0;
            ui->frameTimeAdj->hide();
        }
    }
    return false;
}

void TickEditWindow::hideEvent(QHideEvent* event)
{
    ui->frameTimeAdj->setVisible(false);
    event->accept();
}

void TickEditWindow::showEvent(QShowEvent* event)
{
    reloadTimTick();
    event->accept();
}

void TickEditWindow::on_bUDMin_clicked(int button)
{
    if (button)
        shiftTime(-60000);
    else
        shiftTime(60000);
}

void TickEditWindow::on_bUDSec_clicked(int button)
{
    if (button)
        shiftTime(-1000);
    else
        shiftTime(1000);
}

void TickEditWindow::on_bUDMsec_clicked(int button)
{
    if (button)
        shiftTime(-timerInterval);
    else
        shiftTime(timerInterval);
}

void TickEditWindow::on_textTimeStart_editingFinished()
{
    bool reset;
    if (!verifyTime(ui->textTimeStart->text(), reset))
        ui->textTimeStart->setFocus();
    if (reset)
        ui->textTimeStart->setText(TimeLine::timeStampToString(tick.startTime,
                                                        timeFormat));
}

void TickEditWindow::on_textTimeEnd_editingFinished()
{
    bool reset;
    if (!verifyTime(ui->textTimeEnd->text(), reset))
        ui->textTimeEnd->setFocus();
    if (reset)
        ui->textTimeEnd->setText(TimeLine::timeStampToString(tick.endTime,
                                                      timeFormat));
}

void TickEditWindow::on_buttonBox_accepted()
{
    tick.id = ui->textID->text();
    tick.startTime = TimeLine::stringToTimeStamp(ui->textTimeStart->text(),
                                                 timeFormat);
    tick.endTime = TimeLine::stringToTimeStamp(ui->textTimeEnd->text(),
                                               timeFormat);
    tick.content = ui->textContent->toPlainText();
    tickModified = true;
}

void TickEditWindow::on_buttonBox_clicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ResetRole)
        reloadTimTick();
}

bool TickEditWindow::verifyTime(QString newTime, bool& reset)
{
    reset = false;
    if (TimeLine::stringToTimeStamp(newTime, timeFormat) >= 0)
        return true;

    if (QMessageBox::warning(this, "Time format error",
                             QString("The format of this time is not "
                             "correct.\nIt should be something like ")
                             .append(TimeLine::timeStampToString(
                                 PUNCTOR_EDITOR_TIMESTAMP_SAMPLE, timeFormat))
                             .append("\nWould you like to store it to "
                             "its original value?"),
                             QMessageBox::Yes | QMessageBox::No,
                             QMessageBox::No) != QMessageBox::Yes)
        return false;

    reset = true;
    return true;
}

void TickEditWindow::shiftTime(qint64 delta)
{
    QLineEdit* textEdit;
    if (lastModified == PUNCTOR_EDITOR_FIELD_STARTTIME)
        textEdit = ui->textTimeStart;
    else if (lastModified == PUNCTOR_EDITOR_FIELD_ENDTIME)
        textEdit = ui->textTimeEnd;
    else
        return;

    qint64 timeStamp = TimeLine::stringToTimeStamp(textEdit->text(),
                                                   timeFormat);
    timeStamp += delta;
    if (timeStamp < 0)
        timeStamp = 0;
    textEdit->setText(TimeLine::timeStampToString(timeStamp, timeFormat));
}

void TickEditWindow::reloadTimTick()
{
    ui->textID->setText(tick.id);
    ui->textTimeStart->setText(TimeLine::timeStampToString(tick.startTime,
                                                    timeFormat));
    ui->textTimeEnd->setText(TimeLine::timeStampToString(tick.endTime,
                                                  timeFormat));
    ui->textContent->setPlainText(tick.content);
}
