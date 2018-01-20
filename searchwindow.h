#ifndef SearchWindow_H
#define SearchWindow_H

#include <QDialog>

namespace Ui {
class SearchWindow;
}

class SearchWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SearchWindow(QWidget *parent = 0);
    ~SearchWindow();

signals:
    void searchTimeline(QString searched, int& lastIndex);
    void replaceTimeline(QString searched,
                         QString newContent,
                         int& lastIndex);
    void replaceTimelineAll(QString searched,
                            QString newContent,
                            int& lastIndex);

private slots:
    void on_checkBox_clicked();
    void on_buttonExit_clicked();
    void on_buttonFind_clicked();
    void on_buttonReplace_clicked();    
    void on_buttonReplaceAll_clicked();

private:
    Ui::SearchWindow *ui;
    int lastSearched;

    bool checkInput(bool replace);
};

#endif // SearchWindow_H
