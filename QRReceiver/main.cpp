#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
#ifdef Q_OS_ANDROID
    //先进行自适应缩放,这个会将字体进行合理的缩放
//    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    //再进行叠加缩放。因为前面的自适应缩放会将画面放大得比较厉害，用在手机上有点过。
    //所以在这里要再进行一下人为修正。
//    qputenv("QT_SCALE_FACTOR", QByteArray("0.75"));
#endif

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}
