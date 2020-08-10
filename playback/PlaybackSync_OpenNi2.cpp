//#include <XnOpenNI.h>
//#include <XnLog.h>
//#include <XnCppWrapper.h>
//#include <XnPropNames.h>

#include <OpenNI.h>

#include <iostream>
#include <cmath>

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstdio>


#ifndef WIN32
    #include <boost/filesystem.hpp>
#else
    #include <io.h>
    #include <direct.h>
#endif 

using namespace std;

#include <FreeImage.h>

long long kTimeDiff = 33000;  // microseconds (30fps 1 frame = 0.033s = 33ms = 33000us)

void colorToBitmap(openni::VideoFrameRef& colorMD, FIBITMAP* dib)
{
    openni::RGB888Pixel* colorData = (openni::RGB888Pixel*)colorMD.getData();

    int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
    BYTE* bits = FreeImage_GetBits(dib);
    unsigned pitch = FreeImage_GetPitch(dib);

    int height = colorMD.getHeight();
    int width = colorMD.getWidth();

    for (int y = 0; y < height; y++) {
        BYTE* pixel = (BYTE*)bits;
        for (int x = 0; x < width; x++) {
            pixel[FI_RGBA_RED] = colorData[y*width+x].r;
            pixel[FI_RGBA_GREEN] = colorData[y * width + x].g;
            pixel[FI_RGBA_BLUE] = colorData[y * width + x].b;

            pixel += bytespp;
        }
        bits += pitch;

    }
    FreeImage_FlipVertical(dib);
}

void depthToBitmap(openni::VideoFrameRef& depthMD, FIBITMAP* dib)
{
    openni::DepthPixel* depthData = (openni::DepthPixel*)depthMD.getData();

    int height = depthMD.getHeight();
    int width = depthMD.getWidth();

    for (int y = 0; y < height; y++) {
        unsigned short* bits = (unsigned short*)FreeImage_GetScanLine(dib, y);
        for (int x = 0; x < width; x++) {
            bits[x] = depthData[y * width + x];
        }
    }
    FreeImage_FlipVertical(dib);
}

int main(int argc, char* argv[])
{
    if (argc < 3) {
        cout << "Usage: " << argv[0] << "  <file>  <output folder>" << endl;
        return 0;
    }
    string input_file = argv[1];
    string dir = argv[2];

#ifndef _WIN32
    boost::filesystem::create_directory(dir);
    boost::filesystem::create_directory(dir + "/depth");
    boost::filesystem::create_directory(dir + "/image");
#else
    string depth = dir + "/depth";
    string image = dir + "/image";
    /*CreateDirectory(depth, NULL);
    CreateDirectory(image, NULL);*/
    _mkdir(dir.c_str());
    _mkdir(depth.c_str());
    _mkdir(image.c_str());
#endif

    //xn::Context context;
    //XnStatus status = context.Init();
    openni::Status status = openni::OpenNI::initialize();

    openni::Device device;
    status = device.open(input_file.c_str());
    openni::PlaybackControl* player = device.getPlaybackControl();

    player->setRepeatEnabled(false);
    player->setSpeed(0);

    openni::VideoStream depth_stream;
    depth_stream.create(device, openni::SensorType::SENSOR_DEPTH);

    openni::VideoStream color_stream;
    color_stream.create(device, openni::SensorType::SENSOR_COLOR);

    // show total number of frames
    int num_depth_frames = player->getNumberOfFrames(depth_stream);
    int num_color_frames = player->getNumberOfFrames(color_stream);
    std::cout << "Depth frames: " << num_depth_frames << std::endl;
    std::cout << "Color frames: " << num_color_frames << std::endl;

    int colorHeight, colorWidth, depthHeight, depthWidth;
    colorHeight = depthHeight = 480;
    colorWidth = depthWidth = 640;

    FIBITMAP* colorBitmap = FreeImage_Allocate(colorWidth, colorHeight, 24);
    FIBITMAP* depthBitmap = FreeImage_AllocateT(FIT_UINT16, depthWidth, depthHeight, 16);

    // write frames
    int k = 0;
    bool exit = false;
    int discardDepthFrames = 0;
    int discardColorFrames = 0;
    char timeFilename[256];
    sprintf(timeFilename, "%s/timestamp.txt", dir.c_str());
    FILE* file_timestamp = fopen(timeFilename, "w");

    depth_stream.start();
    color_stream.start();

    while (!exit)
    {
        openni::VideoFrameRef depth_meta;
        openni::VideoFrameRef color_meta;

        // get frame data
        if (depth_stream.readFrame(&depth_meta) != openni::Status::STATUS_OK) break;
        if (color_stream.readFrame(&color_meta) != openni::Status::STATUS_OK) break;

        // Check color and depth frame timestamps to ensure they were captured at the same time
        // If not, we attempt to re-synchronize by getting a new frame from the stream that is behind.
        uint64_t color_timestamp = color_meta.getTimestamp();
        uint64_t depth_timestamp = depth_meta.getTimestamp();
        long long diff = std::abs((long long)color_timestamp - (long long)depth_timestamp);

        //std::cout << k << " " << color_timestamp << " " << depth_timestamp << " "
        //    << color_meta.FrameID() << " " << depth_meta.FrameID() << std::endl;

        while (diff > kTimeDiff) {
            if (color_timestamp > depth_timestamp) {
                // Get another depth frame to try and re-sync as color ahead of depth
                discardDepthFrames++;

                if (depth_stream.readFrame(&depth_meta) != openni::Status::STATUS_OK) {
                    exit = true;
                    break;
                }
            }
            else {
                discardColorFrames++;

                if (color_stream.readFrame(&color_meta) != openni::Status::STATUS_OK) {
                    exit = true;
                    break;
                }
            }

            color_timestamp = color_meta.getTimestamp();
            depth_timestamp = depth_meta.getTimestamp();
            diff = std::abs((long long)color_timestamp - (long long)depth_timestamp);

            std::cout << "Sync: " << color_timestamp << " " << depth_timestamp << " "
                << color_meta.getFrameIndex() << " " << depth_meta.getFrameIndex() << std::endl;
        }
        if (exit) break;

        k++;

        // Number frame ID again.
        // Ignore timestamp for now; just use the old timestamp.
        // color_meta.FrameID() = k;
        // depth_meta.FrameID() = k;
        // color_meta.Timestamp() = XnUInt64(double(k) / 30.0 * 1000000);
        // depth_meta.Timestamp() = XnUInt64(double(k) / 30.0 * 1000000);

        colorToBitmap(color_meta, colorBitmap);
        depthToBitmap(depth_meta, depthBitmap);

        char colorFilename[256];
        char depthFilename[256];
        sprintf(colorFilename, "%s/image/image%05d.png", dir.c_str(), k);
        sprintf(depthFilename, "%s/depth/depth%05d.png", dir.c_str(), k);

        FreeImage_Save(FIF_PNG, colorBitmap, colorFilename);
        FreeImage_Save(FIF_PNG, depthBitmap, depthFilename);

        // TODO: remove timestamp from filename, and store into a text file. 
        // Format: frame ID - color timestamp - depth timestamp
        fprintf(file_timestamp, "%d %lld %lld\n", k, color_meta.getTimestamp(), depth_meta.getTimestamp());
    }

    printf("Recorded: %d frames. %d depth, %d color frames discarded.\n", k, discardDepthFrames, discardColorFrames);
    fclose(file_timestamp);

    // TODO: archive everything into a zero-compression zip file


    return 0;
}
