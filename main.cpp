
#include "ImageCommonDefs.h"
#include "BackgroundSubtraction.h"
#include "HOSVD_Classifier.h"
#include "safe_pointer_vector.h"
#include <Windows.h>

#include "..\..\MarineSnow\MarineSnowFilter.h"


int main(void) {
	int startFrame = 1200;
	int numOfFrames = 3;
	int endFrame = startFrame + numOfFrames;
	MSFparams params;
	TVideoFor<Color_3x8_Pixel> inputVideo = * CreateAndOrphan_ColorVideo_FromFiles("C:\\Users\\Dell\\Desktop\\INZYNIERKA\\antarktyda_jpg\\frame", "jpg", startFrame, endFrame);
	TVideoFor<Color_3x8_Pixel> outputVideo;
	TVideoFor<long> outputOutliersVideo;

	MarineSnowFilterForColor<Color_3x8_Pixel> filter;
	filter(inputVideo, outputVideo, outputOutliersVideo, params);

	//Save_JPEG_Frames
	for (int i = 0; i < outputVideo.GetNumOfFrames(); i++) {
		Save_JPEG_Image(*outputVideo.GetFrameAt(i), "C:\\Users\\Dell\\Desktop\\INZYNIERKA\\DeRecLibProject\\MarineSnow\\OutputVideo\\out" + to_string(startFrame + i) + ".jpg");
		
		//conversion
		for (int row = 0; row < inputVideo.GetFrameAt(i)->GetRow(); row++)
		{
			for (int col = 0; col < inputVideo.GetFrameAt(i)->GetCol(); col++)
			{
				long pixel = outputOutliersVideo.GetPixel(col, row, i);
				if(pixel > 0) outputOutliersVideo.SetPixel(col, row, i, 255);
			}
		}

		Save_JPEG_Image((MonochromeImage)*outputOutliersVideo.GetFrameAt(i), "C:\\Users\\Dell\\Desktop\\INZYNIERKA\\DeRecLibProject\\MarineSnow\\OutputBoleanVideo\\outBool" + to_string(startFrame + i) + ".jpg");
	}
	system("pause");
	return 0;
}

