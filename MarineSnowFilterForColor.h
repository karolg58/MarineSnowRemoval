#ifndef MARINESNOWFILTERFORCOLOR_H
#define MARINESNOWFILTERFORCOLOR_H

#include "MarineSnowFilter.h"
#include "Histogram.h"

template < typename _InPixType, typename _ComputationalPixelType = long  >
class MarineSnowFilterForColor : public MarineSnowFilter<_InPixType, _ComputationalPixelType>
{
protected:

	TVideoFor< ComputationalPixelType >	 meanRGBdistanceVideo;

public:

	//default constructor
	MarineSnowFilterForColor(){}

	virtual ~MarineSnowFilterForColor() 
	{
		meanRGBdistanceVideo.DestroyFrames();
	}

	//Main method, overload operator ()
	virtual bool operator()(const InVideo & inputVideo, InVideo & outputVideo, OutliersVideo & outputOutliersVideo, MSFparams params) 
	{
		this->inputVideo = inputVideo;
		this->outputVideo = outputVideo;
		this->outliersVideo = outputOutliersVideo;
		this->params = params;

		outputOutliersVideo.DestroyFrames();
		outputVideo.DestroyFrames();

		int kCols = inputVideo.GetFrameAt(0)->GetCol();
		int kRows = inputVideo.GetFrameAt(0)->GetRow();

		for (int frameNum = 0; frameNum < inputVideo.GetNumOfFrames(); frameNum++)
		{
			OutliersImage outliersFrame(kCols, kRows, 1);
			outliersVideo.AttachOrphanedFrame(&outliersFrame);

			InImage outputFrame(*inputVideo.GetFrameAt(frameNum));
			outputVideo.AttachOrphanedFrame(&outputFrame);

			MeanRGBdistances(*inputVideo.GetFrameAt(frameNum));
		}

		for (int frameNum = 0; frameNum < inputVideo.GetNumOfFrames(); frameNum++) 
		{			
			this->fIdx = frameNum - 1;

			if (frameNum > 1) 
			{
				//start counting time for processing one frame
				long long int time = clock();

				//for color frames - only pixels which hava RGBdistance low enough could be outliers
				CheckRGBdistance();

				//looking for brights areas at frame
				FindBrights();

				//filtering time
				cout << "time = " << clock() - time << endl;
			}

		}
		return true;
	}

protected:
	//checking if RGBdistance is low enough, result saved into outliersFrames
	void CheckRGBdistance() 
	{
		int idx;
		int numberOfSectors = sqrt(params.sectorsRGBnumber);						//convert to number of sectors per row axis or col axis
		int Rlen = ceil(((double)inputVideo.GetFrameAt(fIdx)->GetRow() / (double)numberOfSectors) / 3.0);			//sector distance between overlapped sectors in pixel from row axis
		int Clen = ceil(((double)inputVideo.GetFrameAt(fIdx)->GetCol() / (double)numberOfSectors) / 3.0);			//sector distance between overlapped sectors in pixel from col axis
		//checking loop
		for (int row = 0; row < inputVideo.GetFrameAt(fIdx)->GetRow(); row++) 
		{
			for (int col = 0; col < inputVideo.GetFrameAt(fIdx)->GetCol(); col++) 
			{
				InPixType pixel = inputVideo.GetPixel(col, row, fIdx);
				long RGBdist = RGBdistance(pixel);

				int counterRGB = 0;

				for (int i = 0; i < 9; i++) 
				{
					counterRGB += CheckRGBdistForSector(col, row, Clen, Rlen, RGBdist, i);
				}

				if (counterRGB < params.RGBsectorsPercent * 9 / 100) 
				{
					outliersVideo.SetPixel(col, row, fIdx, 0);
				}
			}
		}
	}

	//checking RGBdistance in single region
	inline int CheckRGBdistForSector(int col, int row, int Clen, int Rlen, int RGBdist, int number) 
	{
		int r = (row / Rlen) + (number / 3);
		int c = (col / Clen) + (number % 3);
		if (RGBdist < (long)(params.RGBdistanceCoeff * (double)meanRGBdistanceVideo.GetPixel(c, r, fIdx))) return 1;
		return 0;
	}

	//calculate max value from window
	virtual InPixType MaxFromWindow(int c, int r, int fIdx)
	{
		int startRow = max(r - (params.sizeWindowForTimeComparison / 2), 0);
		int stopRow = min(r + 1 + (params.sizeWindowForTimeComparison / 2), inputVideo.GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (params.sizeWindowForTimeComparison / 2), 0);
		int stopCol = min(c + 1 + (params.sizeWindowForTimeComparison / 2), inputVideo.GetFrameAt(fIdx)->GetCol());

		InPixType max;

		max[0] = 0;
		max[1] = 0;
		max[2] = 0;

		for (int row = startRow; row < stopRow; row++) 
		{
			for (int col = startCol; col < stopCol; col++) 
			{
				InPixType pixel = inputVideo.GetPixel(col, row, fIdx);
				for (int channel = 0; channel < 3; channel++) 
				{
					if (pixel[channel] > max[channel]) 
					{
						max[channel] = pixel[channel];
					}
				}
			}
		}
		return max;
	}

	//calculate median value from window
	virtual InPixType MedFromWindow(int c, int r, int fIdx)
	{
		int size = params.sizeWindowForTimeComparison;
		int startRow = max(r - (size / 2), 0);
		int stopRow = min(r + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (size / 2), 0);
		int stopCol = min(c + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetCol());

		InPixType med;
		MonochromeImage tab[3];
		for (int i = 0; i < 3; i++)
		{
			tab[i] = MonochromeImage(size, size);
		}

		int relRow = 0;
		for (int row = startRow; row < stopRow; row++) 
		{
			int relCol = 0;
			for (int col = startCol; col < stopCol; col++) 
			{
				InPixType pixel = inputVideo.GetPixel(col, row, fIdx);
				for (int channel = 0; channel < 3; channel++) 
				{
					tab[channel].SetPixel(relCol, relRow, pixel[channel]);
				}
				relCol++;
			}
			relRow++;
		}

		THistogram hist;
		for (int i = 0; i < 3; i++)
		{
			med[i] = hist.GetMedian(tab[i]);
		}

		return med;
	}

	//calculate average value from window
	virtual InPixType AverageFromWindow(int c, int r, int fIdx)
	{
		int size = params.sizeWindowForTimeComparison;
		int startRow = max(r - (size / 2), 0);
		int stopRow = min(r + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (size / 2), 0);
		int stopCol = min(c + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetCol());
		InPixType sum;
		long numberOfElements = 0;
		sum[0] = 0;
		sum[1] = 0;
		sum[2] = 0;

		//sum elements
		for (int row = startRow; row < stopRow; row++) 
		{
			for (int col = startCol; col < stopCol; col++) 
			{
				numberOfElements++;
				InPixType pixel = inputVideo.GetPixel(c, r, fIdx);
				for (int channel = 0; channel < 3; channel++) 
				{
					sum[channel] += pixel[channel];
				}
			}
		}
		//divide by number of elements
		if (numberOfElements == 0) numberOfElements = 1;
		for (int ch = 0; ch < 3; ch++) 
		{
			sum[ch] /= numberOfElements;
		}
		//return average
		return sum;
	}

	//Calculate RGB distance
	inline ComputationalPixelType RGBdistance(InPixType pixel) {
		return abs((ComputationalPixelType)R(pixel)- (ComputationalPixelType)G(pixel)) 
			+ abs((ComputationalPixelType)G(pixel) - (ComputationalPixelType)B(pixel)) 
			+ abs((ComputationalPixelType)B(pixel) - (ComputationalPixelType)R(pixel));
	}

	//Calculate mean RGBdistance for all sectors into frame
	void MeanRGBdistances(const InImage & frame) 
	{
		int numberOfSectors = sqrt(params.sectorsRGBnumber);		//convert to number of sectors per row axis or col axis
		int Rlen = ceil(((double)frame.GetRow() / (double)numberOfSectors) / 3.0);			//sector distance between overlapped sectors in pixel from row axis
		int Clen = ceil(((double)frame.GetCol() / (double)numberOfSectors) / 3.0);			//sector distance between overlapped sectors in pixel from col axis
		int meanRGBsize = 3 * numberOfSectors + 2;					//size of frames accumulate meanRGBdistances for sectors

		//initialize table
		TImageFor< ComputationalPixelType > * meanRGBframe = new TImageFor< ComputationalPixelType >(meanRGBsize, meanRGBsize, 0);
		

		//sum RGBdist value for all overlapped sectors
		for (int row = 0; row < frame.GetRow(); row++) 
		{
			for (int col = 0; col < frame.GetCol(); col++) 
			{
				InPixType pixel = frame.GetPixel(col, row);				//get pixel value
				ComputationalPixelType RGBdist = RGBdistance(pixel);	//calculate RGBdistance

				for (int i = 0; i < 9; i++)
				{
					int r = (row / Rlen) + (i / 3);
					int c = (col / Clen) + (i % 3);
					meanRGBframe->SetPixel(c, r, meanRGBframe->GetPixel(c, r) + RGBdist);
				}
			}
		}
		//change sums to mean values
		for (int i = 0; i < meanRGBsize; i++) 
		{
			for (int j = 0; j < meanRGBsize; j++) 
			{
				int divisor = (3 * Rlen * 3 * Clen); //divisor for center sectors sums (default)

				//change divisor for border setors - it contains sum of smaller number of pixels RGBdist
				if (i == 1 || i == 3 * numberOfSectors) divisor = 2*divisor/3;
				if (j == 1 || j == 3 * numberOfSectors) divisor = 2*divisor/3;
				if (i == 0 || i == 3 * numberOfSectors + 1) divisor /= 3;
				if (j == 0 || j == 3 * numberOfSectors + 1) divisor /= 3;

				//divide sum by number of element have been sumed
				meanRGBframe->SetPixel(i, j, meanRGBframe->GetPixel(i, j) / divisor);
				
			}
		}
		//attach to the video
		meanRGBdistanceVideo.AttachOrphanedFrame(meanRGBframe);
	}

	//function to compare pixxels in connection with choosen parameters
	inline virtual bool Smaller(const InPixType & Lpixel, const InPixType & Rpixel) 
	{
		bool result = false;

		for (int i = 0; i < 3; i++) 
		{
			if (params.channelsToCompare[i]) 
			{
				if ((long)Lpixel.fData[i] < (long)(params.windowValueCoeff * (double)Rpixel.fData[i])) 
				{
					result = true;
					break;
				}
			}
		}

		return result;
	}
};


#endif
