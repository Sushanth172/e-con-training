#include "sample.h"
QStringListModel Sample::deviceNameModel;
QStringListModel Sample::formatModel;
QStringListModel Sample::resolutionModel;
QStringListModel Sample::fpsModel;

#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 720

Sample::Sample()
{
    number = 0;
    bytesused=0;
    buffer=(unsigned char*) malloc(FRAME_HEIGHT*FRAME_WIDTH*2);
    m_renderer = NULL;
    timer = new QTimer(this);
    renderFlag = false;
}

Sample::~Sample()
{
    //    qDebug() << Q_FUNC_INFO;
}

void Sample::deviceEnumeration(){
    int iterator;
    deviceNameList.clear();
    deviceNameList.append("Select Device");
    getDeviceCount(&number);
    for(iterator=1;iterator<=number;iterator++)
    {
        getDeviceInfo(iterator,serialNumber,deviceName,productId,vendorId,devicePath);
        deviceNameList << deviceName;
    }
    deviceNameModel.setStringList(deviceNameList);
}
void Sample::selectDevice(int index){

    if(index>0)
    {
        openDevice(index);

        if (!m_renderer)
        {
            m_renderer = new Renderer();
            m_renderer->renderer_width = FRAME_WIDTH;
            m_renderer->renderer_height = FRAME_HEIGHT;
            m_renderer->calculateViewport((window()->height()-100),(window()->width()-300));            //calculating viewport size
            connect( window(), &QQuickWindow::afterRendering, this, &Sample::paint, Qt::DirectConnection);
        }
        if(MJPEG_flag)
        {
            m_renderer->set_shaders_RGB();
        }
        else
        {
            m_renderer->set_shaders_UYVY();
        }

        sleep(2);
        renderFlag = true;
    }
}

void Sample::grabframe()
{
    buffer=grabFrame(&bytesused);
    if(buffer==NULL)
    {
        printf("\nMEMORY NOT ALLOCATED IN BUFFER IN GRAB FRAME\n");
    }
    m_renderer->getImageBuffer(buffer);
}


void Sample::paint()
{
    if(renderFlag)
        grabframe();
    m_renderer->paint();
    window()->resetOpenGLState();
    window()->update();
}
