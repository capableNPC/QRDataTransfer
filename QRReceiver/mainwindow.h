#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QCamera>
#include <QCameraInfo>
#include <QCameraViewfinder>
#include <QVideoProbe>

#include <QPaintEvent>

#include "decodethread.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void searchAndLock();

protected:
    void paintEvent(QPaintEvent *evt);

private slots:
    void on_pushButton_clicked();

private:
    void init();
    int startCapture();

    bool requestPermission(QString reqStr);

    void processMessage(QString msg);

    void saveFile(QString fileName);

private:
    Ui::MainWindow *ui;

    QCamera *mCamera;
    QCameraViewfinder *mViewer;
    QVideoProbe *mVideoProbe;

    DecodeThread *mDecThread;

    int mState;

    QImage mImage;

    int mTotalCount;
    int mFrameLength;

    QString mFileName;
    QStringList mWaitList;

    QMap<int, QByteArray> mDataMap;

};
#endif // MAINWINDOW_H
