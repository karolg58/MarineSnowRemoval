#ifndef MARINESNOWFILTERFORMONOCHROME_H
#define MARINESNOWFILTERFORMONOCHROME_H

#include "MarineSnowFilter.h"

template < typename _InPixType, typename _ComputationalPixelType = long >
class MarineSnowFilterForMonochrome : public MarineSnowFilter<_InPixType, _ComputationalPixelType>
{
public:

	//default constructor
	MarineSnowFilterForMonochrome() {}

	virtual ~MarineSnowFilterForMonochrome(){}

	virtual bool operator()(InVideoAP & inVideo, OutVideoAP & outVideo, OutliersVideoAP & outOutliersVideo, const MSFparams & params)
	{ 
		init(inVideo, outVideo, outOutliersVideo, params);

		for (int frameNum = 0; frameNum < inputVideo.GetNumOfFrames(); frameNum++)
		{
			this->fIdx = frameNum - 1;

			if (frameNum > 1)
			{
				//start counting time for processing one frame
				long long int time = clock();

				//looking for brights areas at frame
				FindBrights();

				findAreas();

				if (frameNum > 3)
				{
					checkNeighborhoods(fIdx - 1);
				}

				//filtering time
				cout << "time = " << clock() - time << endl;
			}

			outVideo = this->outputVideo;
			outOutliersVideo = this->outliersVideo;
			inVideo = this->inputVideo;

		}
		return true;
	}

protected:

	//initialize filter
	virtual void init(InVideoAP & inVideo, OutVideoAP & outVideo, OutliersVideoAP & outOutliersVideo, const MSFparams & params)
	{
		int kCols = inVideo.get()->GetFrameAt(0)->GetCol();
		int kRows = inVideo.get()->GetFrameAt(0)->GetRow();
		int numOfFrames = inVideo.get()->GetNumOfFrames();

		this->inputVideo = inVideo;
		OutVideoAP out(new InVideo(*inputVideo.get()));
		this->outputVideo = out;

		OutliersVideoAP outliers(new OutliersVideo(kCols, kRows, numOfFrames, 1));
		this->outliersVideo = outliers;
		this->params = params;
	}

	//calculate max value from window
	virtual InPixType MaxFromWindow(const int & c, const int & r, const int & fIdx)
	{
		int startRow = max(r - (params.sizeWindowForTimeComparison / 2), 0);
		int stopRow = min(r + 1 + (params.sizeWindowForTimeComparison / 2), inputVideo.GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (params.sizeWindowForTimeComparison / 2), 0);
		int stopCol = min(c + 1 + (params.sizeWindowForTimeComparison / 2), inputVideo.GetFrameAt(fIdx)->GetCol());

		InPixType max = 0;

		for (int row = startRow; row < stopRow; row++)
		{
			for (int col = startCol; col < stopCol; col++)
			{
				InPixType & pixel = inputVideo.GetPixel(col, row, fIdx);

				if (pixel > max)
				{
					max = pixel;
				}
			}
		}
		return max;
	}

	//calculate median value from window
	virtual InPixType MedFromWindow(const int & c, const int & r, const int & fIdx)
	{
		int size = params.sizeWindowForTimeComparison;
		int startRow = max(r - (size / 2), 0);
		int stopRow = min(r + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (size / 2), 0);
		int stopCol = min(c + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetCol());

		InPixType med;
		MonochromeImage tab(size, size);

		int relRow = 0;
		for (int row = startRow; row < stopRow; row++) 
		{
			int relCol = 0;
			for (int col = startCol; col < stopCol; col++) 
			{
				InPixType & pixel = inputVideo.GetPixel(col, row, fIdx);
				for (int channel = 0; channel < 3; channel++) 
				{
					tab.SetPixel(relCol, relRow, pixel);
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
		int size = params.sizeWindowForTimeComparison;
		int startRow = max(r - (size / 2), 0);
		int stopRow = min(r + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetRow());
		int startCol = max(c - (size / 2), 0);
		int stopCol = min(c + 1 + (size / 2), inputVideo.GetFrameAt(fIdx)->GetCol());

		InPixType sum;
		long numberOfElements = 0;
		sum = 0;

		//sum elements
		for (int row = startRow; row < stopRow; row++)
		{
			for (int col = startCol; col < stopCol; col++)
			{
				numberOfElements++;
				InPixType & pixel = inputVideo.GetPixel(c, r, fIdx);
				sum += pixel;
			}
		}

		//divide by number of elements
		if (numberOfElements == 0) numberOfElements = 1;
		sum /= numberOfElements;

		//return average
		return sum;
	}

	//function to compare pixxels in connection with choosen parameters
	inline virtual bool Smaller(const InPixType & Lpixel, const InPixType & Rpixel)
	{
		if ((long)Lpixel < (long)(params.windowValueCoeff * (double)Rpixel)) return true;
		return false;
	}
};

#endif
