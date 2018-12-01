/* ---------------------------------------------------------------------------
** This software is in the public domain, furnished "as is", without technical
** support, and with no warranty, express or implied, as to its usefulness for
** any purpose.
**
** V4l2DeviceSource.h
** 
** V4L2 live555 source 
**
** -------------------------------------------------------------------------*/


#ifndef DEVICE_SOURCE
#define DEVICE_SOURCE

#include <string>
#include <list>
#include <iostream>
#include <iomanip>

// live555
#include "../../live_server/liveMedia/include/FramedSource.hh"

#include "../../simple_utils.h"

////////////////////////////////////////////////////////

class V4L2DeviceSource : public FramedSource {
public:
    // 从v4l2设备当中捕获到的一帧
    // ---------------------------------
    // Captured frame
    // ---------------------------------
    struct Frame {
        Frame(char *buffer, int size, timeval timestamp) :
                m_buffer(buffer), m_size(size), m_timestamp(timestamp) {};

        Frame(const Frame &);

        Frame &operator=(const Frame &);

        ~Frame() { delete[] m_buffer; };

        char *m_buffer;
        unsigned int m_size;
        timeval m_timestamp;
    };

    // ---------------------------------
    // Compute simple stats
    // ---------------------------------
    class Stats {
    public:
        Stats(const std::string &msg) : m_fps(0), m_fps_sec(0), m_size(0), m_msg(msg) {};

    public:
        int notify(int tv_sec, int framesize);

    protected:
        int m_fps;
        int m_fps_sec;
        int m_size;
        const std::string m_msg;
    };

public:
    // 用于创建V4L2DeviceSource的静态工厂方法
    // 在这里我们使用了DeviceInterface来创建V4L2DeviceSource的实例
    static V4L2DeviceSource *createNew(UsageEnvironment &env,
                                       int outputFd,
                                       unsigned int queueSize,
                                       bool useThread);

    std::string getAuxLine() { return m_auxLine; };

    void setAuxLine(const std::string auxLine) { m_auxLine = auxLine; };

//    int getWidth() { return m_device->getWidth(); };
//
//    int getHeight() { return m_device->getHeight(); };
//
//    // 用于标识v4l2设备出来的视频流的捕获格式
//    int getCaptureFormat() { return m_device->getCaptureFormat(); };

protected:
    V4L2DeviceSource(UsageEnvironment &env, int outputFd,
                     unsigned int queueSize,
                     bool useThread);

    virtual ~V4L2DeviceSource();

protected:
    static void *threadStub(void *clientData) {
        return ((V4L2DeviceSource *) clientData)->thread();
    };

    void *thread();

    static void deliverFrameStub(void *clientData) {
        ((V4L2DeviceSource *) clientData)->deliverFrame();
    };

    void deliverFrame();

    static void incomingPacketHandlerStub(void *clientData,
                                          int mask) {
        ((V4L2DeviceSource *) clientData)->incomingPacketHandler();
    };

    void incomingPacketHandler();

    int getNextFrame();

    void processFrame(char *frame, int frameSize, const timeval &ref);

    void queueFrame(char *frame, int frameSize, const timeval &tv);

    /// 将packet切分成frame
    /// 不同的子类当中提供不同的实现
    /// 其中父类V4L2DeviceSource提供了自己的实现
    /// 在H264_V4l2DeviceSource当中也提供了自己的实现
    /// 其中H264_V4L2DeviceSource当中在splitFrames当中定义了关于sps和pps的解析方式.
    /// 具体采用哪一种具体的splitFrames实现，取决于运行时多态的选择
    /// 这也算是DeviceSource的一种设计,根据不同的视频编码格式来采用不同的解析方式
    /// 例如对于h264编码，我们需要解析出sps和pps
    /// 这样有利于我们扩展更多的视频格式
    // split packet in frames
    virtual std::list<std::pair<unsigned char *, size_t> > splitFrames(unsigned char *frame,
                                                                       unsigned frameSize);

    // override FramedSource
    virtual void doGetNextFrame();

    virtual void doStopGettingFrames();

protected:
    std::list<Frame *> m_captureQueue;
    Stats m_in;
    Stats m_out;
    EventTriggerId m_eventTriggerId;
    int m_outfd;
    unsigned int m_queueSize;
    pthread_t m_thid;
    pthread_mutex_t m_mutex;
    std::string m_auxLine;
};

#endif
