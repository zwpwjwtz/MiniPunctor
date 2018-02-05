#ifndef TickShiftWindow_H
#define TickShiftWindow_H

#include <QDialog>

namespace Ui {
class TickShiftWindow;
}

class TickShiftWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TickShiftWindow(QWidget *parent = 0);
    ~TickShiftWindow();
    void setPrompt(const QString text);

    qint64 shiftValue;
    bool accepted;

private slots:
    void on_buttonOK_clicked();
    void on_buttonCancel_clicked();

private:
    Ui::TickShiftWindow *ui;
};

#endif // TickShiftWindow_H
