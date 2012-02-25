#ifndef RENDERWINDOW_H
#define RENDERWINDOW_H

#include <QWidget>

extern "C"
{
#include "jpegdecoder.h"
}

class RenderWindow : public QWidget
{
    Q_OBJECT
public:
    explicit RenderWindow(const char* fileName = 0, QWidget *parent = 0);
    virtual ~RenderWindow();

signals:

public slots:
    void decodingFinished();

public:
    void setFileName(const char* fileName);

protected:
    void paintEvent(QPaintEvent *event);


protected:
    const char* mFileName;
    RawImage* mRawImage;
    QImage* mImage;
    QThread* mDecoderThread;
    int decodingResult;
    clock_t mClock;

};

#endif // RENDERWINDOW_H
