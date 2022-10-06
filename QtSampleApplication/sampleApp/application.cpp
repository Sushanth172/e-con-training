#include "application.h"
#include "h264decoder.h"
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
//    buffer=(unsigned char*)malloc(FRAME_HEIGHT*FRAME_WIDTH*2);
    m_renderer = NULL;
    H264_Decoder = NULL;
    timer = new QTimer(this);
    renderFlag = false;
}

Application::~Application()
{
    decompressFuture.waitForFinished();
    future.waitForFinished();
    if(decompressedBuffer!=NULL)
    {
        free(decompressedBuffer);
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

//    QString formatType = QString::number(formats);
//    format_Type = QString (QLatin1String (formats));  //Converting char* to QString

int Application::totalFormats(int deviceIndex)
{
    getFormats(&NoOfFormats);
    qDebug()<<"\nTOTAL FORMATS:"<<NoOfFormats;
    return NoOfFormats;
}

//FORMAT TYPE ENUMERATION
void Application::enumFormat()
{
    QString format_Type= QChar(formats);//Converting char to QString
    FormatList.clear();
    FormatList.append("Format Type");

    for(int formatNo=1;formatNo<NoOfFormats;formatNo++)
    {
        getFormatType(formatNo,&formats,&width,&height,&fps);
        FormatList<<format_Type;
    }
    formatTypeModel.setStringList(FormatList);

}

//RESOLUTION ENUMERATION
void Application::enumResolution(int resIndex)
{

    QString Width = QString::number(width);
    QString Height = QString::number(height);
    resolution_List.clear();
    resolution_List.append("Resolutions");
    while(resIndex)
    for(int formatNo=1;formatNo<NoOfFormats;formatNo++)
    {
        getFormatType(formatNo,&formats,&width,&height,&fps);
        resolution_List<<Width/*<<"x"<<Height*/;
    }
    resolutionModel.setStringList(resolution_List);
    fpsModel.setStringList(fps_List);
}

//FRAMES PER SECOND ENUMERATION
void Application::enumFps(int fpsIndex)
{
    QString FPS = QString::number(fps);
    fps_List.clear();
    fps_List.append("Frame Per Second");

    getFormatType(fpsIndex,&formats,&width,&height,&fps);
    fps_List<<FPS;

}

int Application::selectDevice(int index)
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
//        getFormatType(3,&formats,&width,&height,&fps);
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
//            qDebug()<<"\nSHADER FINISHED";
        }
        sleep(2);
//        future=QtConcurrent::run(Application::render,this);
        renderFlag = true;
        return index;
    }
}

void Application::grabframe()
{
    m_renderer->grabframeMutex.lock();

    qDebug()<<"\nGRAB FRAME";

    buffer=grabFrame(&bytesused);
    qDebug()<<"\nBytesUsed:"<<bytesused;


    if(!buffer)
    {
        printf("\nMEMORY NOT ALLOCATED IN BUFFER IN GRAB FRAME\n");
    }

    qDebug()<<"\nBefore Function Call";
//    MJPEG_flag=true;
    if(MJPEG_flag)
    {
        qDebug()<<"\nENTERS MJPEG ";
        decompressFuture = QtConcurrent::run(m_renderer,&Renderer::mjpeg_decompress,buffer, decompressedBuffer, bytesused,2560);
        if(!decompressedBuffer)
        {
            qDebug()<<"\nDECOMPRESSED BUFFER IS NULL";
        }
        qDebug()<<"\nENTERS MJPEG IMAGE BUFFER";
        m_renderer->getImageBuffer(decompressedBuffer);

    }
    else if(H264_flag)
    {
      H264_Decoder->initH264Decoder(FRAME_HEIGHT,FRAME_WIDTH);
      H264_Decoder->decodeH264(decompressedBuffer,buffer,bytesused);
      m_renderer->getImageBuffer(decompressedBuffer);
    }
    else
    {
        qDebug()<<"\nENTERS UYVY IMAGE BUFFER";
        m_renderer->getImageBuffer(buffer);
    }
    qDebug()<<"\nGRAB FRAME FINISHED";
    m_renderer->grabframeMutex.unlock();
}

//void Application::render(Application *app)
//{
//    while(app->renderFlag)
//    {
//        app->grabframe();
//        qDebug()<<"\nGRAB FRAME THREAD";
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
