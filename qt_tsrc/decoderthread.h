#ifndef DECODERTHREAD_H
#define DECODERTHREAD_H

#include <QThread>

extern "C"
{
#include "jpegdecoder.h"
}

class DecoderThread : public QThread
{
    Q_OBJECT
public:
    explicit DecoderThread(const char* fileName, RawImage** image, int& decodingResult, QObject *parent = 0);

signals:

public slots:

public:
    void run();

protected:
    const char* mFilename;
    RawImage** mImage;
    int& mDecodingResult;
};

#endif // DECODERTHREAD_H
