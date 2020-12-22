#ifndef DECODETHREAD_H
#define DECODETHREAD_H

#include <QObject>
#include <QThread>
#include <QZXing>
#include <QImage>

class DecodeThread : public QObject
{
    Q_OBJECT
public:
    explicit DecodeThread(QObject *parent = nullptr);

    void preDecode(QImage image);

signals:
   void __decode();
    void decoded(QString code);

private slots:
    void decode();

private:
    QImage mImgae;

    QZXing mZxing;

};

#endif // DECODETHREAD_H
