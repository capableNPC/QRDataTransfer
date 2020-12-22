#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QZXing>
#include <QPainter>

#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QByteArray>

#define FRAME_LENGTH 100

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mTimer = new QTimer();
    connect(mTimer, &QTimer::timeout, this, &MainWindow::updateQR);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::paintEvent(QPaintEvent *evt)
{
    QPainter painter(this);

    painter.drawImage(100, 150, mImg);
}

void MainWindow::updateQR()
{
    if(mCurIndex < mTotalCount)
    {
        int index = ui->comboBox_curIndex->currentIndex() + 1;
        if(index > ui->comboBox_curIndex->model()->rowCount())
        {
            index = 0;
        }
        ui->comboBox_curIndex->setCurrentIndex(index);
    }
    else
    {
        mTimer->stop();
    }

}


void MainWindow::on_pushButton_start_clicked()
{
    mTimer->setInterval(ui->spinBox_interval->value());

    if(ui->pushButton_start->text() == "开始")
    {
        ui->pushButton_start->setText("结束");

        mTimer->start();
    }
    else
    {
        ui->pushButton_start->setText("开始");

        mTimer->stop();
    }
}

void MainWindow::on_pushButton_selectFile_clicked()
{
    QFileInfo tempInfo(ui->lineEdit_file->text());

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    tempInfo.absolutePath(),
                                                    tr(""));

    if(fileName.isEmpty() == false)
    {
        ui->lineEdit_file->setText(fileName);

        QFile file(fileName);
        if(file.open(QFile::ReadOnly))
        {
            QByteArray array = file.readAll();

            array = qCompress(array);//压缩

            mBase64Data = array.toBase64();

            qDebug() << "data length:" << mBase64Data.length();

            mTotalCount = mBase64Data.length() / FRAME_LENGTH;
            if(mBase64Data.length() % FRAME_LENGTH != 0)
            {
                mTotalCount++;
            }

            mTotalCount += 1; //第一张为文件信息

            QString str = QString("共%1张，当前：").arg(mTotalCount);
            ui->label_info->setText(str);

            mCurIndex = 0;

            QFileInfo fileInfo(fileName);
            mFileInfo = QString("0,%1,%2,%3").arg(fileInfo.fileName()).arg(mTotalCount).arg(FRAME_LENGTH);

            qDebug() << mFileInfo;

            ui->comboBox_curIndex->clear();
            QStringList items;
            for(int i = 0; i < mTotalCount; i++)
            {
                items.push_back(QString::number(i));
            }

            ui->comboBox_curIndex->addItems(items);
            ui->comboBox_curIndex->setCurrentIndex(0);

        }
    }

}

void MainWindow::on_comboBox_curIndex_currentIndexChanged(int index)
{
    if(index < 0)
    {
        return;
    }

    mCurIndex = index;

    QString tempData;
    if(index == 0)
    {
        tempData = mFileInfo;
    }
    else
    {
        tempData = QString("%1,").arg(mCurIndex) + mBase64Data.mid((mCurIndex - 1)* FRAME_LENGTH, FRAME_LENGTH);

    }

    QImage img = QZXing::encodeData(tempData,
                                    QZXing::EncoderFormat_QR_CODE,
                                    QSize(256, 256),
                                    QZXing::EncodeErrorCorrectionLevel_L);

    mImg = img;

    update();
}
