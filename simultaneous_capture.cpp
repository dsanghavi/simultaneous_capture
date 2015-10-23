#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/bind.hpp>
#include <boost/atomic.hpp>
#include "FlyCapture2.h"
#include <vector>

boost::mutex io_mutex;

using namespace FlyCapture2;

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

void thread_fun(boost::barrier& cur_barier, boost::atomic<int>& current)
{
    ++current;
    cur_barier.wait();
    //boost::lock_guard<boost::mutex> locker( );
    std::cout << current << std::endl;
}

void capture_images(boost::barrier& cur_barier, Camera* c){
    CameraInfo camInfo;
    c->GetCameraInfo( &camInfo );
    PrintCameraInfo(&camInfo);
    /*
    Image rawImage, convertedImage; 
    unsigned int numImages = 50;
    if (argc < 2) {
      printf("No parameter entered for number of images, going with default 50.\n");
    } else {
      // numImages = (int)*argv[1];
      numImages = 1;
    }
    std::vector<Image> vecImages1;
        vecImages1.resize(numImages);
    std::vector<Image> vecImages2;
        vecImages2.resize(numImages);
    for (unsigned int j=0; j < numImages; j++ ) {
        for (unsigned int cam=0; cam < numCameras; cam++) {

        error = pcam[cam]->RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
          PrintError( error );
          continue;
        }

        // We'll also print out a timestamp
            TimeStamp timestamp = rawImage.GetTimeStamp();
                printf(
                  "Cam %d - Frame %d - TimeStamp [%d %d]\n",
                  cam,
                  j,
                  timestamp.cycleSeconds,
                  timestamp.cycleCount
        );

        if(cam==0) {
            vecImages1[j].DeepCopy(&rawImage);
        } else {
            vecImages2[j].DeepCopy(&rawImage);
        }
        }
    }
    //Process and store the images captured
    for (unsigned int j=0; j < numImages; j++) {
        error = vecImages1[j].Convert( PIXEL_FORMAT_RGB, &convertedImage );
                if (error != PGRERROR_OK)
                {
                  PrintError( error );
                  return -1;
                }

                // Create a unique filename
                char filename[512];
                sprintf( filename, "./images/cam--%d-%d.tiff", 0, j);

                // Save the image. If a file format is not passed in, then the file
                // extension is parsed to attempt to determine the file format.
                error = convertedImage.Save( filename );
                if (error != PGRERROR_OK)
                {
                  PrintError( error );
                  return -1;
                }
        //Do the same for the second camera
        error = vecImages2[j].Convert( PIXEL_FORMAT_RGB, &convertedImage );
                if (error != PGRERROR_OK)
                {
                  PrintError( error );
                  return -1;
                }

                // Create a unique filename
                char filename2[512];
                sprintf( filename2, "./images/cam--%d-%d.tiff", 1, j);

                // Save the image. If a file format is not passed in, then the file
                // extension is parsed to attempt to determine the file format.
                error = convertedImage.Save( filename2 );
                if (error != PGRERROR_OK)
                {
                  PrintError( error );
                  return -1;
                }
    }*/
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
    boost::atomic<int> current(0);
    boost::thread thr1(boost::bind(&capture_images, boost::ref(bar), boost::ref(pcam[0])));
    //boost::thread thr2(boost::bind(&capture_images, boost::ref(bar), boost::ref(pcam[1])));
    //boost::thread thr2(boost::bind(&thread_fun, boost::ref(bar), boost::ref(current)));
    thr1.join();
    //thr2.join();
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