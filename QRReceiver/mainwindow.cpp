#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QHBoxLayout>
#include <QCameraImageCapture>
#include <QDateTime>

#include <QDesktopServices>
#include <QFileDialog>

#include <QMessageBox>

#include <QtAndroid>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    mViewer = new QCameraViewfinder();
//    QHBoxLayout *mLayout = new QHBoxLayout();
//    mLayout->addWidget(mViewer);
//    ui->centralwidget->setLayout(mLayout);
    mState = 0;

    init();
    startCapture();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void NV21_T_RGB(unsigned int width , unsigned int height, unsigned char *yuyv , unsigned char *rgb);
void NV21_T_GRAY(unsigned int width , unsigned int height, unsigned char *pInbuf, unsigned char *pOutbuf);

void MainWindow::init()
{
    mCamera = nullptr;

    mVideoProbe = new QVideoProbe();
    connect(mVideoProbe, &QVideoProbe::videoFrameProbed, this, [=](QVideoFrame frame)
    {
//        qDebug() << "new video frame:" << frame.isValid() << frame.size();

        QDateTime begin = QDateTime::currentDateTime();

        if (frame.isValid())
        {
            if(frame.size() != QSize(640, 480))
            {
                return ;
            }

            QVideoFrame cloneFrame(frame);
            cloneFrame.map(QAbstractVideoBuffer::ReadOnly);

            static uchar *mBuffer = nullptr;
            if(mBuffer == nullptr)
            {
                qDebug() << "create buffer:--------------------------------------";
                mBuffer = new uchar[frame.width() * frame.height() * 3];
            }


//            qDebug() << "interval 0:" << begin.msecsTo(QDateTime::currentDateTime());
//            NV21_T_RGB(frame.width(), frame.height(), cloneFrame.bits(), mBuffer);
            NV21_T_GRAY(frame.width(), frame.height(), cloneFrame.bits(), mBuffer);
//            qDebug() << "interval 1:" << begin.msecsTo(QDateTime::currentDateTime());

//            const QImage image(mBuffer,
//                               cloneFrame.width(),
//                               cloneFrame.height(),
//                               QImage::Format_RGB888);

            const QImage image(mBuffer,
                               cloneFrame.width(),
                               cloneFrame.height(),
                               QImage::Format_Grayscale8);

            static int i = 0;
            if(i > 10)
            {
                i = 0;

                mDecThread->preDecode(image);
            }
            i++;

//            mImage = image.scaled(this->size(), Qt::KeepAspectRatio);
//            mImage = image.copy();
            mImage = image;

//                    delete [] mBuffer;

            cloneFrame.unmap();


//            qDebug() << "interval:" << begin.msecsTo(QDateTime::currentDateTime());

            update();

        }
    }, Qt::QueuedConnection);

    mDecThread = new DecodeThread();
    connect(mDecThread, &DecodeThread::decoded, this, [=](QString info){
        if(info != "")
        {
            qDebug() << info;

            processMessage(info);
        }
    }, Qt::QueuedConnection);

}

void NV21_T_GRAY(unsigned int width, unsigned int height, unsigned char *pInbuf, unsigned char *pOutbuf)
{
    unsigned int len = width * height;

//    quint32 i;

//    quint32 * pIn = (quint32*)pInbuf;

//    quint32 *pOut= (quint32*)pOutbuf;

//    len >>= 2;

//    for(i = 0; i < len; i++)
//    {
//        *pOut= *pIn & (0xff00ff00 | 0x00800080);

//        pIn++;

//        pOut++;
//    }

    //nv21格式下， 前n个字节存储的就是灰度值，android不知为何用不了memset
    for(uint i = 0; i < len; i++)
    {
        pOutbuf[i] = pInbuf[i];
    }
}
void NV21_T_RGB(unsigned int width , unsigned int height , unsigned char *yuyv , unsigned char *rgb)
{

    const int nv_start = width * height ;
    quint32  i, j, index = 0, rgb_index = 0;
    quint8 y, u, v;
    int r, g, b, nv_index = 0;

    for(i = 0; i < height; i++){
        for(j = 0; j < width; j ++){
            //nv_index = (rgb_index / 2 - width / 2 * ((i + 1) / 2)) * 2;
//            nv_index = i / 2  * width + j - j % 2;
            nv_index = (i >> 1)  * width + j - (j & 0x01);


//            y = yuyv[nv_index];
            y = yuyv[rgb_index];
            u = yuyv[nv_start + nv_index ];
            v = yuyv[nv_start + nv_index + 1];

            r = y + (140 * (v-128))/100;  //r
            g = y - (34 * (u-128))/100 - (71 * (v-128))/100; //g
            b = y + (177 * (u-128))/100; //b

            if(r > 255)   r = 255;
            if(g > 255)   g = 255;
            if(b > 255)   b = 255;
            if(r < 0)     r = 0;
            if(g < 0)     g = 0;
            if(b < 0)     b = 0;

            index = rgb_index % width + (height - i - 1) * width;
            //rgb[index * 3+0] = b;
            //rgb[index * 3+1] = g;
            //rgb[index * 3+2] = r;

            //颠倒图像
            //rgb[height * width * 3 - i * width * 3 - 3 * j - 1] = b;
            //rgb[height * width * 3 - i * width * 3 - 3 * j - 2] = g;
            //rgb[height * width * 3 - i * width * 3 - 3 * j - 3] = r;

            //正面图像
//            rgb[i * width * 3 + 3 * j + 0] = b;
//            rgb[i * width * 3 + 3 * j + 1] = g;
//            rgb[i * width * 3 + 3 * j + 2] = r;

//            int tempIndex = (i * width  + j) * 3;
            int tempIndex = rgb_index * 3;

            //正面图像
            rgb[tempIndex + 0] = b;
            rgb[tempIndex + 1] = g;
            rgb[tempIndex + 2] = r;

            rgb_index++;
        }
    }
}

void MainWindow::paintEvent(QPaintEvent *evt)
{
    if(mImage.isNull())
    {
        return;
    }

    QPainter painter(this);

    QMatrix matrix;
    matrix.rotate(90.0);
    QImage tempImage = mImage.transformed(matrix, Qt::FastTransformation);


    QSizeF imgSize(tempImage.width(), tempImage.height());
    QSizeF scaledSize = imgSize.scaled(this->size(), Qt::KeepAspectRatio);

    double scaleRatio = scaledSize.width() / imgSize.width();

    double offsetX = this->width() -  scaledSize.width();
    double offsetY = this->height() -  scaledSize.height();


//    painter->rotate(90);
    painter.translate(offsetX / 2.0, offsetY / 2.0);
    painter.scale(scaleRatio, scaleRatio);

    painter.drawImage(0, 0, tempImage);
}

int MainWindow::startCapture()
{
    if(mCamera != nullptr)
    {
        mCamera->start();
        return 0;
    }

    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();

    foreach (const QCameraInfo &cameraInfo, cameras)
    {
//        if (cameraInfo.position() == QCamera::FrontFace)
        if (cameraInfo.position() == QCamera::BackFace)
        {

            mCamera = new QCamera(cameraInfo);
            mVideoProbe->setSource(mCamera);

//            mCamera->setViewfinder(mViewer);


//            auto imageCapture = new QCameraImageCapture(mCamera);
//            connect(imageCapture, &QCameraImageCapture::imageAvailable, [=](int id, const QVideoFrame &frame){
//                qDebug() << id << frame.pixelFormat();
//            });
//            qDebug() << "iamge capture:"
//                     << imageCapture->supportedImageCodecs()
//                     << imageCapture->supportedResolutions()
//                     << imageCapture->supportedBufferFormats();

//            camera->setViewfinder(viewfinder);

//            mCamera->setCaptureMode(QCamera::CaptureViewfinder);
//            mCamera->setCaptureMode(QCamera::CaptureVideo);
            mCamera->setCaptureMode(QCamera::CaptureStillImage);
            mCamera->start();

//            imageCapture->capture();
//            qDebug() << "formats:" << mCamera->supportedViewfinderPixelFormats();

            qDebug() << "resolutions:" << mCamera->supportedViewfinderResolutions();
            QCameraViewfinderSettings settings;
            settings.setResolution(QSize(640, 480));
            settings.setPixelFormat(QVideoFrame::Format_NV21);
            settings.setMaximumFrameRate(30);
            mCamera->setViewfinderSettings(settings);

            qDebug() << ""
                     << mCamera->viewfinderSettings().pixelFormat()
                     << mCamera->viewfinderSettings().maximumFrameRate();

            return 0;
        }
    }

    return -1;
}

void MainWindow::processMessage(QString msg)
{
//    mFileInfo = QString("0,%1,%2,%3").arg(fileInfo.fileName()).arg(mTotalCount).arg(FRAME_LENGTH);

    if(mState == 1)
    {
        return;
    }

    QStringList msgList = msg.split(",");
    if(msgList.length() > 1)
    {
        bool ok;
        int index = msgList[0].toInt(&ok);
        if(ok)
        {
            if(index == 0)
            {
                if(mWaitList.isEmpty() == false)
                {
                    return;
                }

                QString info = QString("文件名：%1").arg(msgList[1]);
                ui->label_Info->setText(info);

                mFileName = msgList[1];

                mTotalCount = msgList[2].toInt() -1;
                mFrameLength = msgList[3].toInt();

                mWaitList.clear();
                for(int i = 0; i < mTotalCount; i++)
                {
                    mWaitList.push_back(QString::number(i + 1));
                }

                ui->comboBox->clear();
                ui->comboBox->addItems(mWaitList);

                ui->progressBar->setValue(0);

                QString tempStr = QString("未识别(%1/%2):").arg(mWaitList.length()).arg(mTotalCount);
                ui->label->setText(tempStr);
            }
            else
            {
               if(mWaitList.isEmpty())
               {
                   return;
               }

               mDataMap.insert(index, QByteArray::fromBase64(msgList[1].toUtf8()));

               mWaitList.removeAll(QString::number(index));

               ui->comboBox->clear();
               ui->comboBox->addItems(mWaitList);

               double percentage = (double)(mTotalCount - mWaitList.length()) / (double)mTotalCount;

               QString tempStr = QString("未识别(%1/%2):").arg(mWaitList.length()).arg(mTotalCount);
               ui->label->setText(tempStr);

               ui->progressBar->setValue(percentage * 100);

               if(ui->progressBar->value() == 100)
               {
                   mState = 1;

                   mCamera->stop();

                   QMessageBox msgb;
                   msgb.setWindowTitle("接收完成");
                   msgb.setText(QString("%1 已经接收完毕。").arg(mFileName));
                   msgb.setIcon(QMessageBox::Information);
                   msgb.exec();

//                   QString fileName = QFileDialog::getSaveFileName(this, tr("保存文件"),
//                                                                   mFileName,
//                                                                   tr("所有 (*)"));

                   requestPermission("android.permission.WRITE_EXTERNAL_STORAGE");
                   requestPermission("android.permission.READ_EXTERNAL_STORAGE");

                   QDir *folder = new QDir;
                   QString baseDir = "/storage/emulated/0/QRReceiver";
                   bool exist = folder->exists(baseDir);
                   if(exist)
                   {
                       //QMessageBox::warning(this, tr("createDir"), tr("Dir is already existed!"));
                   }
                   else
                   {
                       //创建文件夹
                       bool ok = folder->mkdir(baseDir);
                       if(ok)
                       {
//                           QMessageBox::warning(this, tr("CreateDir"), tr("Create Dir success!"));
                       }
                       else
                       {
                           QMessageBox::warning(this, tr("CreateDir"), tr("Create Dir fail"));
                       }
                   }

                   QString fileName = baseDir + "/" + mFileName;

                   saveFile(fileName);

               }

            }

        }
    }


}

bool MainWindow::requestPermission(QString reqStr)
{
    QtAndroid::PermissionResult r = QtAndroid::checkPermission(reqStr);
    if(r == QtAndroid::PermissionResult::Denied) {
        QtAndroid::requestPermissionsSync( QStringList() << reqStr);
        r = QtAndroid::checkPermission(reqStr);
        if(r == QtAndroid::PermissionResult::Denied)
        {
            qDebug() << "QtAndroid::PermissionResult::Denied" << reqStr;
            return false;
        }
    }

    return true;
}
void MainWindow::saveFile(QString fileName)
{

    qDebug() << "save file:" << fileName;

    QFile file(fileName);
    if(file.open(QFile::ReadWrite))
    {
        qDebug() << "open file succeed";

        QByteArray tempArray;
        for(int i = 0; i < mTotalCount; i++)
        {
            tempArray.push_back(mDataMap[ i + 1 ]);
        }

        file.write(qUncompress(tempArray));

        file.close();
    }


}

void MainWindow::searchAndLock()
{
    if(mCamera != nullptr)
    {
        mCamera->searchAndLock();
    }
}

void MainWindow::on_pushButton_clicked()
{
//    QDesktopServices::openUrl(QUrl("/", QUrl::TolerantMode));

    QDir dir("./");
    qDebug() << "abs path:" << dir.absolutePath();

//    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
//                               "",
//                               tr("All (*.*)"));

    mWaitList.clear();
    ui->label_Info->setText("");
    ui->comboBox->clear();
    mTotalCount = 0;
    mState = 0;

    ui->label->setText("未识别：");

    mDataMap.clear();

    mCamera->start();
}
