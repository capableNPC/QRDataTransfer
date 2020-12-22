#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPaintEvent>
#include <QImage>

#include <QTimer>

#pragma execution_character_set("utf-8")

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *evt);

private slots:
    void updateQR();

    void on_pushButton_start_clicked();

    void on_pushButton_selectFile_clicked();

    void on_comboBox_curIndex_currentIndexChanged(int index);

private:
    Ui::MainWindow *ui;

    QImage mImg;

    QTimer *mTimer;

    QByteArray mBase64Data;

    int mTotalCount;
    int mCurIndex;

    QString mFileInfo;
};
#endif // MAINWINDOW_H
