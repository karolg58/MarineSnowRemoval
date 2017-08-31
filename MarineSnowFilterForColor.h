#ifndef MARINESNOWFILTERFORCOLOR_H
#define MARINESNOWFILTERFORCOLOR_H

#include "MarineSnowFilter.h"
#include "Histogram.h"

template < typename _InPixType, typename _ComputationalPixelType = long  >
class MarineSnowFilterForColor : public MarineSnowFilter<_InPixType, _ComputationalPixelType>
{
protected:

	std::auto_ptr<TVideoFor< ComputationalPixelType > >	 meanRGBdistanceVideo;

public:

	//default constructor
	MarineSnowFilterForColor(){}

	virtual ~MarineSnowFilterForColor() {}

	//Main method, overload operator ()inputVideo.get()->
	virtual bool operator()(InVideoAP & inVideo, OutVideoAP & outVideo, OutliersVideoAP & outOutliersVideo, const MSFparams & params)
	{
		init(inVideo, outVideo, outOutliersVideo, params);

		for (int frameNum = 0; frameNum < inputVideo.get()->GetNumOfFrames(); frameNum++) 
		{			
			this->fIdx = frameNum - 1;

			if (frameNum > 1) 
			{
				//start counting time for processing one frame
				//long long int time = clock();

				MeanRGBdistances(*inputVideo.get()->GetFrameAt(frameNum));

				//for color frames - only pixels which hava RGBdistance low enough could be outliers
				CheckRGBdistance();

				//looking for brights areas at frame
				FindBrights();

				//printArea(1160, 100, 350, 100, fIdx);

				findAreas();

				if (frameNum > 3)
				{
					checkNeighborhoods(fIdx - 1);
				}

				//printArea(1160, 100, 350, 100, fIdx);

				//filtering time
				//cout << "time = " << clock() - time << endl;
			}

		}

		outVideo = this->outputVideo;
		outOutliersVideo = this->outliersVideo;
		inVideo = this->inputVideo;

		return true;
	}

protected:
	//initialize filter
	virtual void init(InVideoAP & inVideo, OutVideoAP & outVideo, OutliersVideoAP & outOutliersVideo, const MSFparams & params)
	{
		const int & kCols = inVideo.get()->GetFrameAt(0)->GetCol();
		const int & kRows = inVideo.get()->GetFrameAt(0)->GetRow();
		const int & numOfFrames = inVideo.get()->GetNumOfFrames();

		this->inputVideo = inVideo;

		OutVideoAP out(new InVideo(*inputVideo.get()));
		this->outputVideo = out;

		OutliersVideoAP outliers(new OutliersVideo(kCols, kRows, numOfFrames, 1));
		this->outliersVideo = outliers;

		this->params = params;

		int numberOfSectors = sqrt(this->params.sectorsRGBnumber);		//convert to number of sectors per row axis or col axis
		int meanRGBsize = 3 * numberOfSectors + 2;						//size of frames accumulate meanRGBdistances for sectors
		std::auto_ptr<TVideoFor< ComputationalPixelType > > mean(new TVideoFor< ComputationalPixelType >(meanRGBsize, meanRGBsize, numOfFrames, 0));
		this->meanRGBdistanceVideo = mean;
	}

	//checking if RGBdistance is low enough, result saved into outliersFrames
	void CheckRGBdistance() 
	{
		int numberOfSectors = sqrt(params.sectorsRGBnumber);	//convert to number of sectors per row axis or col axis
		int Rlen = ceil(((double)inputVideo.get()->GetFrameAt(fIdx)->GetRow() / (double)numberOfSectors) / 3.0);			//sector distance between overlapped sectors in pixel from row axis
		int Clen = ceil(((double)inputVideo.get()->GetFrameAt(fIdx)->GetCol() / (double)numberOfSectors) / 3.0);			//sector distance between overlapped sectors in pixel from col axis
		//checking loop
		for (int row = 0; row < inputVideo.get()->GetFrameAt(fIdx)->GetRow(); row++) 
		{
			for (int col = 0; col < inputVideo.get()->GetFrameAt(fIdx)->GetCol(); col++) 
			{
				const InPixType & pixel = inputVideo.get()->GetPixel(col, row, fIdx);
				const ComputationalPixelType & RGBdist = RGBdistance(pixel);

				int counterRGB = 0;

				for (int i = 0; i < 9; i++) 
				{
					counterRGB += CheckRGBdistForSector(col, row, Clen, Rlen, RGBdist, i);
				}

				if (counterRGB < params.RGBsectorsPercent * 9 / 100) 
				{
					outliersVideo.get()->SetPixel(col, row, fIdx, 0);
				}
			}
		}
	}

	//checking RGBdistance in single region
	inline int CheckRGBdistForSector(const int & col, const int & row, const int & Clen, const int & Rlen, const int & RGBdist, const int & number)
	{
		int r = (row / Rlen) + (number / 3);
		int c = (col / Clen) + (number % 3);
		if (RGBdist < (long)(params.RGBdistanceCoeff * (double)meanRGBdistanceVideo.get()->GetPixel(c, r, fIdx))) return 1;
		return 0;
	}

	//calculate max value from window
	virtual InPixType MaxFromWindow(const int & c, const int & r, const int & fIdx)
	{
		int startRow = max(r - (params.sizeWindowForTimeComparison / 2), 0);
		int stopRow = min(r + 1 + (params.sizeWindowForTimeComparison / 2), inputVideo.get()->GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (params.sizeWindowForTimeComparison / 2), 0);
		int stopCol = min(c + 1 + (params.sizeWindowForTimeComparison / 2), inputVideo.get()->GetFrameAt(fIdx)->GetCol());

		InPixType max;

		max[0] = 0;
		max[1] = 0;
		max[2] = 0;

		for (int row = startRow; row < stopRow; row++) 
		{
			for (int col = startCol; col < stopCol; col++) 
			{
				InPixType & pixel = inputVideo.get()->GetPixel(col, row, fIdx);
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
	virtual InPixType MedFromWindow(const int & c, const int & r, const int & fIdx)
	{
		const int & size = params.sizeWindowForTimeComparison;
		int startRow = max(r - (size / 2), 0);
		int stopRow = min(r + 1 + (size / 2), inputVideo.get()->GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (size / 2), 0);
		int stopCol = min(c + 1 + (size / 2), inputVideo.get()->GetFrameAt(fIdx)->GetCol());

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
				InPixType & pixel = inputVideo.get()->GetPixel(col, row, fIdx);
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
	virtual InPixType AverageFromWindow(const int & c, const int & r, const int & fIdx)
	{
		const int & size = params.sizeWindowForTimeComparison;
		int startRow = max(r - (size / 2), 0);
		int stopRow = min(r + 1 + (size / 2), inputVideo.get()->GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (size / 2), 0);
		int stopCol = min(c + 1 + (size / 2), inputVideo.get()->GetFrameAt(fIdx)->GetCol());
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
				InPixType & pixel = inputVideo.get()->GetPixel(c, r, fIdx);
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
	inline ComputationalPixelType RGBdistance(const InPixType & pixel) {
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

		//sum RGBdist value for all overlapped sectors
		for (int row = 0; row < frame.GetRow(); row++) 
		{
			for (int col = 0; col < frame.GetCol(); col++) 
			{
				const InPixType & pixel = frame.GetPixel(col, row);				//get pixel value
				const ComputationalPixelType & RGBdist = RGBdistance(pixel);	//calculate RGBdistance

				for (int i = 0; i < 9; i++)
				{
					int r = (row / Rlen) + (i / 3);
					int c = (col / Clen) + (i % 3);
					meanRGBdistanceVideo.get()->SetPixel(c, r, fIdx, meanRGBdistanceVideo.get()->GetPixel(c, r, fIdx) + RGBdist);
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
				meanRGBdistanceVideo.get()->SetPixel(i, j, fIdx, meanRGBdistanceVideo.get()->GetPixel(i, j, fIdx) / divisor);
				
			}
		}
		//attach to the video
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
