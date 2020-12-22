#include "decodethread.h"

DecodeThread::DecodeThread(QObject *parent) : QObject(parent)
{
    QThread *thread = new QThread();
    this->moveToThread(thread);
    thread->start();

    connect(this, &DecodeThread::__decode, this, &DecodeThread::decode, Qt::QueuedConnection);
}

void DecodeThread::preDecode(QImage image)
{
    mImgae = image.copy();
    emit __decode();
}

void DecodeThread::decode()
{
    QString code = mZxing.decodeImage(mImgae);

    emit decoded(code);
}
