#include "application.h"
QStringListModel Application::deviceNameModel;
QStringListModel Application::formatTypeModel;
QStringListModel Application::resolutionModel;
QStringListModel Application::fpsModel;

#define FRAME_WIDTH 1280
#define FRAME_HEIGHT 720

Application::Application()
{
    number = 0;
    bytesused=0;
    decompressedBuffer=(unsigned char*)malloc(FRAME_HEIGHT*FRAME_WIDTH*3);
//        buffer=(unsigned char*)malloc(FRAME_HEIGHT*FRAME_WIDTH*4);
    m_renderer = NULL;
    timer = new QTimer(this);
    renderFlag = false;
}

Application::~Application()
{
    decompressFuture.waitForFinished();
    future.waitForFinished();
    if(buffer!=NULL)
    {
        free(buffer);
    }
    qDebug() << Q_FUNC_INFO;
}

void Application::deviceEnumeration(){
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
//void Application::formatType(){
//    getFormatType(formatsAvailable,&formats,&width,&height,&fps);
//    QString s = QString::number(formatsAvailable);
//    FormatList<<s;
//    formatModel.setStringList(FormatList);
//}

void Application::selectDevice(int index)
{
    if(index>0)
    {
        openDevice(index);

        if (!m_renderer)
        {
            //qDebug()<<"\nRenderer";
            m_renderer = new Renderer();
            m_renderer->renderer_width = FRAME_WIDTH;
            m_renderer->renderer_height = FRAME_HEIGHT;

            //calculating viewport size
            m_renderer->calculateViewport((window()->height()-100),(window()->width()-300));

            //sender,signal,reviever,slot
            connect( window(), &QQuickWindow::afterRendering, this, &Application::paint, Qt::DirectConnection);
            //qDebug()<<"\nCONNECTION FINISHED";

            //from window(i.e UI),
            //once the window is refreshed the afterRendering() is called (i.e once the frame changes)
            //calls the paint located in this function...so (this)
            //Then finally the slot is Paint
        }
        int height, width;
        char pixFmt[MAX_CHAR];
        getCurrentFormat(&height,&width,pixFmt);
        qDebug() << Q_FUNC_INFO << pixFmt << height << width;
        MJPEG_flag=true;

        if(MJPEG_flag)
        {
            qDebug()<<"\nMJPEG SHADER STARTS";
            m_renderer->set_shaders_RGB();
            //qDebug()<<"\nSHADER FINISHED";
        }
        else
        {
            qDebug()<<"\nUYVY SHADER STARTS";
            m_renderer->set_shaders_UYVY();
            qDebug()<<"\nSHADER FINISHED";
        }
        sleep(2);
        //future=QtConcurrent::run(Application::render,this);
        renderFlag = true;
    }
}

void Application::grabframe()
{
//    m_renderer->grabframeMutex.lock();
    qDebug()<<"\nInside grabframe1";
    buffer=grabFrame(&bytesused);
    qDebug()<<"\nBytesUsed:"<<bytesused;

    if(!buffer)
    {
        printf("\nMEMORY NOT ALLOCATED IN BUFFER IN GRAB FRAME\n");
    }

    qDebug()<<"\nBefore Function Call";
    MJPEG_flag=true;
    if(MJPEG_flag)
    {
        decompressFuture = QtConcurrent::run(m_renderer,&Renderer::mjpeg_decompress,buffer, decompressedBuffer, bytesused,2560);
        m_renderer->getImageBuffer(decompressedBuffer);
        if(!decompressedBuffer)
        {
            qDebug()<<"\nDECOMPRESSED BUFFER IS NULL";
        }
    }
    else
    {
        qDebug()<<"\nENTERS UYVY IMAGE BUFFER";
        m_renderer->getImageBuffer(buffer);
    }
//    m_renderer->grabframeMutex.unlock();
}

//void Application::render(Application *app)
//{
//    while(app->renderFlag)
//    {
//        app->grabframe();
//    }
//}

void Application::paint()
{
    grabframe();

//    if(renderFlag)
//    {
//        grabframe();
//    }
    m_renderer->paint(); //Paint() is from mrenderer function...
    window()->resetOpenGLState();
    window()->update();
}
