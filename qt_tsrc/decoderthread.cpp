#include "decoderthread.h"

DecoderThread::DecoderThread(const char* fileName, RawImage** image, int& decodingResult, QObject *parent) :
    QThread(parent),
    mFilename(fileName),
    mImage(image),
    mDecodingResult(decodingResult)
{
}

void DecoderThread::run()
{
    mDecodingResult = read_jpeg_file(mFilename, mImage);
}
