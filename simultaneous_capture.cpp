#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/bind.hpp>
//#include <boost/atomic.hpp>
#include "FlyCapture2.h"
#include <vector>
#include <sstream>
#include <boost/lexical_cast.hpp>

boost::mutex io_mutex;

using namespace FlyCapture2;

unsigned long getThreadId(){
    std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
    unsigned long threadNumber = 0;
    sscanf(threadId.c_str(), "%lx", &threadNumber);
    return threadNumber;
}

void PrintCameraInfo( CameraInfo* pCamInfo )
{
    printf(
        "\n*** CAMERA INFORMATION ***\n"
        "Serial number - %u\n"
        "Camera model - %s\n"
        "Camera vendor - %s\n"
        "Sensor - %s\n"
        "Resolution - %s\n"
        "Firmware version - %s\n"
        "Firmware build time - %s\n\n",
        pCamInfo->serialNumber,
        pCamInfo->modelName,
        pCamInfo->vendorName,
        pCamInfo->sensorInfo,
        pCamInfo->sensorResolution,
        pCamInfo->firmwareVersion,
        pCamInfo->firmwareBuildTime );
}

void PrintError( Error error )
{
    error.PrintErrorTrace();
}

/*void thread_fun(boost::barrier& cur_barier, boost::atomic<int>& current)
{
    ++current;
    cur_barier.wait();
    //boost::lock_guard<boost::mutex> locker( );
    std::cout << current << std::endl;
}*/

void capture_images(boost::barrier& cur_barier, Camera* c){
    Error error;
    CameraInfo camInfo;
    c->GetCameraInfo( &camInfo );
    //boost::lock_guard<boost::mutex> locker( );
    //PrintCameraInfo(&camInfo);
    int threadID = (int)getThreadId();
    Image rawImage, convertedImage; 
    unsigned int numImages = 50;
    
    std::vector<Image> vecImages;
    vecImages.resize(numImages);
    for (unsigned int j=0; j < numImages; j++ ) {
        cur_barier.wait();
        error = c->RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
          PrintError( error );
          continue; //For threads, all the other threads have to continue -------------
        }
        // We'll also print out a timestamp
        TimeStamp timestamp = rawImage.GetTimeStamp();
        printf("Cam %d - Frame %d - TimeStamp [%d %d]\n",threadID,j,timestamp.cycleSeconds,timestamp.cycleCount);
        vecImages[j].DeepCopy(&rawImage);
    }
    //Process and store the images captured
    for (unsigned int j=0; j < numImages; j++) {
        error = vecImages[j].Convert( PIXEL_FORMAT_RGB, &convertedImage );
        if (error != PGRERROR_OK){
          PrintError( error );
          return;
        }
        // Create a unique filename
        char filename[512];
        sprintf( filename, "./images/cam--%d-%d.tiff", threadID, j);
        // Save the image. If a file format is not passed in, then the file
        // extension is parsed to attempt to determine the file format.
        error = convertedImage.Save( filename );
        if (error != PGRERROR_OK){
          PrintError( error );
          return;
        }
    }
}

int main()
{
    Error error;
    CameraInfo camInfo;

    BusManager busMgr;
    unsigned int numCameras;

    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    printf("cameras: %u\n", numCameras);

    Camera* pcam[numCameras] ;

    // now we do the formalities needed to establish a connection
    for (unsigned int i=0; i<numCameras; i++) {
      // connect to a camera 
      PGRGuid guid;
      pcam[i] = new Camera();

      error = busMgr.GetCameraFromIndex( i, &guid );
      if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

      error = pcam[i]->Connect(&guid);
      if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

      // Get the camera information
      error = pcam[i]->GetCameraInfo( &camInfo );
      if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }
      // uncomment the following line if you really care about the camera info
      // PrintCameraInfo(&camInfo);

      // Next we turn isochronous images capture ON for both cameras
      error = pcam[i]->StartCapture();
      if (error != PGRERROR_OK)
          {
            PrintError( error );
            return -1;
          }

    }

//call the threads to capture images, convert and save to file------------------------------

    boost::barrier bar(2);
   // boost::atomic<int> current(0);
    boost::thread thr1(boost::bind(&capture_images, boost::ref(bar), boost::ref(pcam[0])));
    boost::thread thr2(boost::bind(&capture_images, boost::ref(bar), boost::ref(pcam[1])));
    //boost::thread thr2(boost::bind(&thread_fun, boost::ref(bar), boost::ref(current)));
    thr1.join();
    thr2.join();
    printf( "Do not press any key till images are captured and written...\n" );
    getchar();
    for ( unsigned int i = 0; i < numCameras; i++ )
    {
        pcam[i]->StopCapture();
        pcam[i]->Disconnect();
        delete pcam[i];
    }
    printf( "Done! Press Enter to exit...\n" );
    getchar();
    return 0;
}