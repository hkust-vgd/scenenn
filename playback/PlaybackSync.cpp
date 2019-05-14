#include <XnOpenNI.h>
#include <XnLog.h>
#include <XnCppWrapper.h>
#include <XnPropNames.h>
#include <iostream>
#include <cmath>

#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cstdio>

#ifndef WIN32
#include <boost/filesystem.hpp>
#endif 

using namespace std;

#include <FreeImage.h>


long long kTimeDiff = 33000;  // microseconds (30fps 1 frame = 0.033s = 33ms = 33000us)

void colorToBuffer(xn::ImageMetaData& colorMD, unsigned char *colorBuffer)
{
    xn::RGB24Map& colorMap = colorMD.WritableRGB24Map();
    for (XnUInt32 y = 0; y < colorMap.YRes(); y++) {
        for (XnUInt32 x = 0; x < colorMap.XRes(); x++) {
            int index = 3 * (y * colorMap.XRes() + x);
            colorBuffer[index++] = colorMap(x, y).nRed;
            colorBuffer[index++] = colorMap(x, y).nGreen;
            colorBuffer[index++] = colorMap(x, y).nBlue;
        }
    }
}

void depthToBuffer(xn::DepthMetaData& depthMD, unsigned short *depthBuffer)
{
    xn::DepthMap& depthMap = depthMD.WritableDepthMap();
    for (XnUInt32 y = 0; y < depthMap.YRes(); y++) {
        for (XnUInt32 x = 0; x < depthMap.XRes(); x++) {
            depthBuffer[y * depthMap.XRes() + x] = depthMap(x, y);
        }
    }
}

void colorToBitmap(xn::ImageMetaData& colorMD, FIBITMAP *dib)
{
    xn::RGB24Map& colorMap = colorMD.WritableRGB24Map();

    int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
    BYTE *bits = FreeImage_GetBits(dib);
    unsigned pitch = FreeImage_GetPitch(dib);

    for (XnUInt32 y = 0; y < colorMap.YRes(); y++) {
        BYTE *pixel = (BYTE *)bits;
        for (XnUInt32 x = 0; x < colorMap.XRes(); x++) {
            pixel[FI_RGBA_RED] = colorMap(x, y).nRed;
            pixel[FI_RGBA_GREEN] = colorMap(x, y).nGreen;
            pixel[FI_RGBA_BLUE] = colorMap(x, y).nBlue;

            pixel += bytespp;
        }
        bits += pitch;

    }
    FreeImage_FlipVertical(dib);
}

void depthToBitmap(xn::DepthMetaData& depthMD, FIBITMAP *dib)
{
    xn::DepthMap& depthMap = depthMD.WritableDepthMap();

    for (XnUInt32 y = 0; y < depthMap.YRes(); y++) {
        unsigned short *bits = (unsigned short *)FreeImage_GetScanLine(dib, y);
        for (XnUInt32 x = 0; x < depthMap.XRes(); x++) {
            bits[x] = depthMap(x, y);
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
  CreateDirectory(depth.c_str(), NULL);
  CreateDirectory(image.c_str(), NULL);
#endif

    xn::Context context;
    XnStatus status = context.Init();

    xn::Player player;
    status = context.OpenFileRecording(input_file.c_str(), player);
    status = player.SetRepeat(false);
    status = player.SetPlaybackSpeed(0);    // fastest possible

    xn::DepthGenerator depth_stream;
    status = context.FindExistingNode(XN_NODE_TYPE_DEPTH, depth_stream);

    xn::ImageGenerator color_stream;
    status = context.FindExistingNode(XN_NODE_TYPE_IMAGE, color_stream);

    // show total number of frames
    XnUInt32 num_depth_frames = 0;
    XnUInt32 num_color_frames = 0;
    status = player.GetNumFrames(depth_stream.GetName(), num_depth_frames);
    status = player.GetNumFrames(color_stream.GetName(), num_color_frames);
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
	FILE *file_timestamp = fopen(timeFilename, "w");
    while (!exit)
    {
        xn::DepthMetaData depth_meta;
        xn::ImageMetaData color_meta;
        
        // get frame data
        if (depth_stream.WaitAndUpdateData() == XN_STATUS_EOF) break;
        depth_stream.GetMetaData(depth_meta);

        if (color_stream.WaitAndUpdateData() == XN_STATUS_EOF) break;
        color_stream.GetMetaData(color_meta);
        
        // Check color and depth frame timestamps to ensure they were captured at the same time
        // If not, we attempt to re-synchronize by getting a new frame from the stream that is behind.
        XnUInt64 color_timestamp = color_meta.Timestamp();
        XnUInt64 depth_timestamp = depth_meta.Timestamp();
        long long diff = std::abs((long long)color_timestamp - (long long)depth_timestamp);
        
        //std::cout << k << " " << color_timestamp << " " << depth_timestamp << " "
        //    << color_meta.FrameID() << " " << depth_meta.FrameID() << std::endl;

        while (diff > kTimeDiff) {
            if (color_timestamp > depth_timestamp) {
                // Get another depth frame to try and re-sync as color ahead of depth
                discardDepthFrames++;
                status = depth_stream.WaitAndUpdateData();
                if (status == XN_STATUS_EOF) {
                    exit = true;
                    break;
                }
                depth_stream.GetMetaData(depth_meta);
            }
            else {
                discardColorFrames++;
                status = color_stream.WaitAndUpdateData();
                if (status == XN_STATUS_EOF) {
                    exit = true;
                    break;
                }
                color_stream.GetMetaData(color_meta);

            }

            color_timestamp = color_meta.Timestamp();
            depth_timestamp = depth_meta.Timestamp();
            diff = std::abs((long long)color_timestamp - (long long)depth_timestamp);

            std::cout << "Sync: " <<  color_timestamp << " " << depth_timestamp << " "
            << color_meta.FrameID() << " " << depth_meta.FrameID() << std::endl;
        }
        if (exit) break;

        k++;


        // Number frame ID again.
        // Ignore timestamp for now; just use the old timestamp.
        //color_meta.FrameID() = k;
        //depth_meta.FrameID() = k;
        //color_meta.Timestamp() = XnUInt64(double(k) / 30.0 * 1000000);
        //depth_meta.Timestamp() = XnUInt64(double(k) / 30.0 * 1000000);

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
			fprintf(file_timestamp, "%d %lld %lld\n", k, color_meta.Timestamp(), depth_meta.Timestamp());
        
    }


    printf("Recorded: %d frames. %d depth, %d color frames discarded.\n", k, discardDepthFrames, discardColorFrames);
	fclose(file_timestamp);

    // TODO: archive everything into a zero-compression zip file


    return 0;
}
