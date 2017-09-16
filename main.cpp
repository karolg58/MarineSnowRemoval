
#include "ImageCommonDefs.h"
#include "BackgroundSubtraction.h"
#include "HOSVD_Classifier.h"
#include "safe_pointer_vector.h"
#include <Windows.h>
#include <iostream>
#include <fstream>

#include "..\..\MarineSnowRemoval\MarineSnowFilter.h"
#include "..\..\MarineSnowRemoval\MarineSnowFilterForColor.h"
#include "..\..\MarineSnowRemoval\Polygons.h"
#include "..\..\MarineSnowRemoval\Comparator.h"


int main(void) {
	string startPath = "C:\\Users\\Dell\\Desktop\\INZYNIERKA\\DeRecLibProject\\MarineSnowRemoval";
	int startFrame = 1148;//real + 2
	int numOfFrames = 55;//real - 4
	int endFrame = startFrame + numOfFrames;
	MSFparams params;
	std::auto_ptr<TVideoFor<Color_3x8_Pixel>> inputVideo(CreateAndOrphan_ColorVideo_FromFiles("C:\\Users\\Dell\\Desktop\\INZYNIERKA\\antarktyda_jpg\\frame", "jpg", startFrame, endFrame));
	std::auto_ptr<TVideoFor<Color_3x8_Pixel>> outputVideo;
	std::auto_ptr< TVideoFor<long> > outputOutliersVideo;
	int kCols = inputVideo.get()->GetFrameAt(0)->GetCol();
	int kRows = inputVideo.get()->GetFrameAt(0)->GetRow();

	MVAP userVideo(new MonochromeVideo(kCols, kRows, 0));
	for (int i = startFrame + 2; i < endFrame - 2; i++)
	{
		//cout << "frame " << i << endl;
		Polygons polygonsImage(kCols, kRows);
		polygonsImage.loadMetaData(startPath + "\\MetaData\\frame" + to_string(i) + "_MetaData.txt");
		MIAP userImage = polygonsImage.drawAndReturn();
		Save_JPEG_Image(*userImage.get(), startPath + "\\Masks\\mask" + to_string(i) + ".jpg");
		userVideo.get()->AttachOrphanedFrame(new MonochromeImage(*userImage.get()));
	}

	MarineSnowFilterForColor<Color_3x8_Pixel> filter;
	Comparator compare;
	queue<MSFparams> Q;
	long minDiff = MAXLONG;

	for (params.sectorsRGBnumber = 25; params.sectorsRGBnumber <= 25; params.sectorsRGBnumber += 16)
	{
		for (params.RGBdistanceCoeff = 0.6; params.RGBdistanceCoeff <= 1.01; params.RGBdistanceCoeff += 0.2)
		{
			for (params.RGBsectorsPercent = 80; params.RGBsectorsPercent <= 100; params.RGBsectorsPercent += 20)
			{
				//6x
				long long int time = clock();
				for (params.typeForTimeComparison = 0; params.typeForTimeComparison <= 1; params.typeForTimeComparison++)
				{
					for (params.sizeWindowForTimeComparison = 5; params.sizeWindowForTimeComparison <= 9; params.sizeWindowForTimeComparison += 2)
					{
						for (params.windowValueCoeff = 0.9; params.windowValueCoeff <= 1.11; params.windowValueCoeff += 0.1)
						{
							for (params.availableSkippedPixelsForFindingArea = 1; params.availableSkippedPixelsForFindingArea <= 3; params.availableSkippedPixelsForFindingArea += 1)
							{
								for (params.radiusForCheckingNeighbours = 2; params.radiusForCheckingNeighbours <= 6; params.radiusForCheckingNeighbours += 2)
								{
									for (params.minCoeffForCompareNeighboursAreas = 0.3; params.minCoeffForCompareNeighboursAreas <= 0.61; params.minCoeffForCompareNeighboursAreas += 0.15)
									{
										params.maxCoeffForCompareNeighboursAreas = 1 / params.minCoeffForCompareNeighboursAreas;
										filter(inputVideo, outputVideo, outputOutliersVideo, params);
										long res = 0;
										for (int i = 0; i < numOfFrames - 4; i++)
										{
											res += (long)compare(*(userVideo.get()->GetFrameAt(i)), *(outputOutliersVideo.get()->GetFrameAt(i + 2)));
										}
										if (res <= minDiff)
										{
											cout << "res= " << res << endl;
											minDiff = res;
											Q.push(params);
											if (Q.size() > 1000)
											{
												Q.pop();
											}
										}
									}
								}
							}
						}
					}
				}
				cout << "time = " << (clock() - time)/1000 << endl;//6x
			}
		}
	}

	cout << minDiff << endl;
	params = Q.back();
	filter(inputVideo, outputVideo, outputOutliersVideo, params);


	//Save_JPEG_Frames
	for (int i = 0; i < outputVideo.get()->GetNumOfFrames(); i++) 
	{
		Save_JPEG_Image(*outputVideo.get()->GetFrameAt(i), startPath + "\\OutputVideo\\out" + to_string(startFrame + i) + ".jpg");

		//conversion
		for (int row = 0; row < inputVideo.get()->GetFrameAt(i)->GetRow(); row++)
		{
			for (int col = 0; col < inputVideo.get()->GetFrameAt(i)->GetCol(); col++)
			{
				long pixel = outputOutliersVideo.get()->GetPixel(col, row, i);
				if(pixel > 0) outputOutliersVideo.get()->SetPixel(col, row, i, 255);
			}
		}

		Save_JPEG_Image((MonochromeImage)*outputOutliersVideo.get()->GetFrameAt(i), startPath + "\\OutputBoleanVideo\\outBool" + to_string(startFrame + i) + ".jpg");
	}

	cout << "endParams:" << endl << params << endl;
	ofstream myfile(startPath + "\\params_output.txt");
	if (myfile.is_open())
	{
		while (!Q.empty())
		{
			myfile << Q.front() << endl << endl;
			Q.pop();
		}
		myfile.close();
	}
	else cout << "Unable to open file";

	system("pause");
	return 0;
}

