#include "threadpool/ThreadPool.h"
#include "Mandelbrot.h"
#include "DumpPNG.h"

const unsigned int IMAGE_WIDTH = 1920;
const unsigned int IMAGE_HEIGHT = 1080;
const unsigned int CHUNK_COUNT = 40;
const unsigned int CHUNK_LINE_COUNT = IMAGE_HEIGHT / CHUNK_COUNT;

const float RE_START = -2 * 1.28f;
const float RE_DELTA = 3 * 1.28f;
const float IM_START = 1.5f * 0.72f;
const float IM_DELTA = -3 * 0.72f;

int main(const int argc, const char **argv)
{
    unsigned char *outputBuffer = new unsigned char[IMAGE_WIDTH * IMAGE_HEIGHT * 4];
    memset(outputBuffer, 0 , IMAGE_WIDTH * IMAGE_HEIGHT * 4);
    ThreadPool *threadPool = createThreadPool();

    BufferInfo bufferInfo(IMAGE_WIDTH, IMAGE_HEIGHT, outputBuffer);
    MandelbrotParams mp(RE_START, RE_DELTA, IM_START, IM_DELTA);

    unsigned int i = 0;
    for(; i < CHUNK_COUNT; i++)
    {
        threadPool->submit([=]()
        {
            mandelbrotSSE2(bufferInfo, mp, CHUNK_LINE_COUNT * i, CHUNK_LINE_COUNT);
        });
    }
    threadPool->await();

    dumpPNG(IMAGE_WIDTH, IMAGE_HEIGHT, outputBuffer);

	delete [] outputBuffer;
    delete threadPool;
    return 0;
}