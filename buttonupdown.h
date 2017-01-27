#ifndef BUTTONUPDOWN_H
#define BUTTONUPDOWN_H

#include <QWidget>
#include <QResizeEvent>

namespace Ui {
class buttonUpDown;
}

class buttonUpDown : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ getText WRITE setText USER true)

public:
    explicit buttonUpDown(QWidget *parent = 0);
    ~buttonUpDown();
    QString getText();
    void setText(QString text);

signals:
    void clicked(int button);   //0 = Up; 1 = Down

private slots:
    void on_buttonUp_clicked();
    void on_buttonDown_clicked();

private:
    Ui::buttonUpDown *ui;

protected:
    void resizeEvent(QResizeEvent *size);

};

#endif // BUTTONUPDOWN_H
