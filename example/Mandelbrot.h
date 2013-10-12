
#ifndef _MANDELBROT_H_
#define _MANDELBROT_H_

struct BufferInfo
{
    int             width;
    int             height;
    unsigned char   *buffer;

    BufferInfo(int width, int height, unsigned char *buffer) :
        width(width),
        height(height),
        buffer(buffer)
    {
    }
};

struct MandelbrotParams
{
    float reStart;
    float reDelta;
    float imStart;
    float imDelta;

    MandelbrotParams(float reStart, float reDelta, float imStart, float imDelta) :
    reStart(reStart),
    reDelta(reDelta),
    imStart(imStart),
    imDelta(imDelta)
    {
    }
};

extern void mandelbrotPure(const BufferInfo &output, const MandelbrotParams &params, int firstLineIndex, int lineCount);
extern void mandelbrotSSE2(const BufferInfo &output, const MandelbrotParams &params, int firstLineIndex, int lineCount);

#endif