#include <QPaintEvent>
#include <QPainter>
#include <QImage>

#include <stdlib.h>

#include "renderwindow.h"
#include "decoderthread.h"

RenderWindow::RenderWindow(const char* fileName, QWidget *parent) :
    QWidget(parent),
    mRawImage(NULL),
    mImage(NULL),
    mDecoderThread(NULL),
    decodingResult(-1)
{
    if (fileName != NULL)
        setFileName(fileName);
}

void RenderWindow::setFileName(const char* fileName)
{
    mFileName = fileName;
    if (mDecoderThread) {
        if (mDecoderThread->isRunning())
            mDecoderThread->wait();
        delete mDecoderThread;
        mDecoderThread = NULL;
    }

    mDecoderThread = new DecoderThread(mFileName,&mRawImage,decodingResult);
    QObject::connect(mDecoderThread, SIGNAL(finished()),
                     this, SLOT(decodingFinished()));
    mDecoderThread->start();
}

void RenderWindow::decodingFinished()
{
    if (decodingResult == 0) {
        if (mRawImage) {
            this->resize(mRawImage->width,mRawImage->height);
            mImage = new QImage(mRawImage->data,mRawImage->width,mRawImage->height,QImage::Format_RGB888);
            this->show();
        }
    }
}

RenderWindow::~RenderWindow()
{
    if (mImage) {
        delete mImage;
        mImage = NULL;
    }
    if (mRawImage) {
        free(mRawImage);
        mRawImage = NULL;
    }
    if (mDecoderThread) {
        delete mDecoderThread;
        mDecoderThread = NULL;
    }

}

void RenderWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (decodingResult == -1)
        return;

    QPainter painter(this);
    painter.drawImage(QPoint(0,0),*mImage);

}
