
#include "ImageCommonDefs.h"
#include "BackgroundSubtraction.h"
#include "HOSVD_Classifier.h"
#include "safe_pointer_vector.h"
#include <Windows.h>

#include "..\..\MarineSnowRemoval\MarineSnowFilter.h"


int main(void) {
	int startFrame = 1200;
	int numOfFrames = 3;
	int endFrame = startFrame + numOfFrames;
	MSFparams params;
	std::auto_ptr<TVideoFor<Color_3x8_Pixel>> inputVideo(CreateAndOrphan_ColorVideo_FromFiles("C:\\Users\\Dell\\Desktop\\INZYNIERKA\\antarktyda_jpg\\frame", "jpg", startFrame, endFrame));
	std::auto_ptr<TVideoFor<Color_3x8_Pixel>> outputVideo;
	std::auto_ptr< TVideoFor<long> > outputOutliersVideo;

	MarineSnowFilterForColor<Color_3x8_Pixel> filter;
	filter(inputVideo, outputVideo, outputOutliersVideo, params);

	//Save_JPEG_Frames
	for (int i = 0; i < outputVideo.get()->GetNumOfFrames(); i++) 
	{
		Save_JPEG_Image(*outputVideo.get()->GetFrameAt(i), "C:\\Users\\Dell\\Desktop\\INZYNIERKA\\DeRecLibProject\\MarineSnow\\OutputVideo\\out" + to_string(startFrame + i) + ".jpg");

		//conversion
		for (int row = 0; row < inputVideo.get()->GetFrameAt(i)->GetRow(); row++)
		{
			for (int col = 0; col < inputVideo.get()->GetFrameAt(i)->GetCol(); col++)
			{
				long pixel = outputOutliersVideo.get()->GetPixel(col, row, i);
				if(pixel > 0) outputOutliersVideo.get()->SetPixel(col, row, i, 255);
			}
		}

		Save_JPEG_Image((MonochromeImage)*outputOutliersVideo.get()->GetFrameAt(i), "C:\\Users\\Dell\\Desktop\\INZYNIERKA\\DeRecLibProject\\MarineSnow\\OutputBoleanVideo\\outBool" + to_string(startFrame + i) + ".jpg");
	}
	system("pause");
	return 0;
}

