#ifndef MARINESNOWFILTER_H
#define MARINESNOWFILTER_H

#include <queue>
#include "MSFParams.h"

template < typename _InPixType, typename _ComputationalPixelType = long >
class MarineSnowFilter 
{
public:

	typedef _InPixType												InPixType;

	typedef _ComputationalPixelType									ComputationalPixelType;		// this type is used to build the outliers image

	typedef	TImageFor< InPixType >									InImage;

	typedef	std::auto_ptr<InImage>									InImageAP;

	typedef TVideoFor< InPixType >									InVideo;

	typedef std::auto_ptr<InVideo >									InVideoAP;

	typedef std::auto_ptr<InVideo >									OutVideoAP;


	typedef	TImageFor< ComputationalPixelType >						OutliersImage;	// stores informations about outliers, 0 - not outlier, more than 0 - outlier

	typedef	std::auto_ptr< OutliersImage >							OutliersImageAP;

	typedef	TVideoFor< ComputationalPixelType >						OutliersVideo;	// stores OutliersImages

	typedef	std::auto_ptr< OutliersVideo >							OutliersVideoAP;


	typedef	TImageFor< bool >										BoolImage;

	typedef	std::auto_ptr<BoolImage>								BoolImageAP;
protected:

	int fIdx;

	MSFparams params;

	InVideoAP inputVideo;

	InVideoAP outputVideo;

	OutliersVideoAP outliersVideo;

public:

	//default constructor
	MarineSnowFilter() {}

	virtual ~MarineSnowFilter() {}

	//Main method, overload operator ()
	virtual bool operator()(InVideoAP & inVideo, OutVideoAP & outVideo, OutliersVideoAP & outOutliersVideo, const MSFparams & params) = 0;

protected:
	//initialize filter
	virtual void init(InVideoAP & inVideo, OutVideoAP & outVideo, OutliersVideoAP & outOutliersVideo, const MSFparams & params) = 0;

	//looking for bright areas
	void FindBrights()
	{
		for (int row = 0; row < inputVideo.get()->GetFrameAt(fIdx)->GetRow(); row++)
		{
			for (int col = 0; col < inputVideo.get()->GetFrameAt(fIdx)->GetCol(); col++)
			{
				const bool suspected = outliersVideo.get()->GetPixel(col, row, fIdx);

				if (suspected)
				{
					const InPixType & pixel = inputVideo.get()->GetPixel(col, row, fIdx);
					InPixType backWindowValue, forwardWindowValue;

					if (params.typeForTimeComparison == 0)
					{
						backWindowValue = MaxFromWindow(col, row, fIdx - 1);
						forwardWindowValue = MaxFromWindow(col, row, fIdx + 1);
					}
					else if (params.typeForTimeComparison == 1)
					{
						backWindowValue = MedFromWindow(col, row, fIdx - 1);
						forwardWindowValue = MedFromWindow(col, row, fIdx + 1);
					}
					else if (params.typeForTimeComparison == 2)
					{
						backWindowValue = AverageFromWindow(col, row, fIdx - 1);
						forwardWindowValue = AverageFromWindow(col, row, fIdx + 1);
					}

					if (Smaller(pixel, backWindowValue) || Smaller(pixel, forwardWindowValue)) {
						outliersVideo.get()->SetPixel(col, row, fIdx, 0);
					}
				}
			}
		}
	}

	//calculate max value from window
	virtual InPixType MaxFromWindow(const int & c, const int & r, const int & fIdx) = 0;

	//calculate median value from window
	virtual InPixType MedFromWindow(const int & c, const int & r, const int & fIdx) = 0;

	//calculate average value from window
	virtual InPixType AverageFromWindow(const int & c, const int & r, const int & fIdx) = 0;

	//function to compare pixxels in connection with choosen parameters
	inline virtual bool Smaller(const InPixType & Lpixel, const InPixType & Rpixel) = 0;


	//temporary function
	void PrintArea(int startCol, int numCols, int startRow, int numRows, int fIdx)
	{
		for (int row = max(startRow, 0); row < min(outliersVideo.get()->GetFrameAt(fIdx)->GetRow(), startRow + numRows); row++)
		{
			for (int col = max(startCol, 0); col < min(outliersVideo.get()->GetFrameAt(fIdx)->GetCol(), startCol + numCols); col++)
			{
				int val = outliersVideo.get()->GetPixel(col, row, fIdx);
				if (val < 10) cout << " ";
				cout << val << ",";
			}
			cout << endl;
		}
		cout << endl;
	}

	//counting adjacent bright pixels to find bright areas
	void FindAreas() 
	{
		const int & availableSkipped = params.availableSkippedPixelsForFindingArea;
		const int & minArea = params.minAreaForSuspectOutliers;
		const int & kRows = outliersVideo.get()->GetFrameAt(fIdx)->GetRow();
		const int & kCols = outliersVideo.get()->GetFrameAt(fIdx)->GetCol();
		BoolImageAP visitedFrame(new BoolImage(kCols, kRows, false));
		int meanCol;
		int meanRow;
		for (int row = 0; row < kRows; row++)
		{
			for (int col = 0; col < kCols; col++)
			{
				const bool suspected = outliersVideo.get()->GetPixel(col, row, fIdx);
				const bool & visited = visitedFrame.get()->GetPixel(col, row);

				if (visited == false && suspected == true) 
				{
					ComputationalPixelType area = FindSingleArea(visitedFrame, col, row, availableSkipped, meanCol, meanRow);
					if (outliersVideo.get()->GetPixel(meanCol, meanRow, fIdx) == 1 && visitedFrame.get()->GetPixel(meanCol, meanRow))
					{
						ConfirmArea(meanCol, meanRow, fIdx);
						outliersVideo.get()->SetPixel(meanCol, meanRow, fIdx, area);
					}
					else//shape is not like marine snow
					{
						ClearArea(col, row, fIdx);
					}
				}
			}
		}
	}

	//loking for single bright area which is suspected to be outliers region
	ComputationalPixelType FindSingleArea(BoolImageAP & visitedFrame, const int & col, const int & row, const int & availableSkipped, int & meanCol, int & meanRow)
	{
		std::queue<int> Q;
		Q.push(col);
		Q.push(row);
		Q.push(0);
		ComputationalPixelType sum = 0;
		int minCol = outliersVideo.get()->GetFrameAt(fIdx)->GetCol();
		int minRow = outliersVideo.get()->GetFrameAt(fIdx)->GetRow();
		int maxCol = 0;
		int maxRow = 0;

		while (!Q.empty())
		{
			int c = Q.front(); Q.pop();
			int r = Q.front(); Q.pop();
			int skipped = Q.front(); Q.pop();
			bool visited = visitedFrame.get()->GetPixel(c, r);
			if (visited == false)
			{
				const bool suspected = outliersVideo.get()->GetPixel(c, r, fIdx);
				if (suspected || skipped < availableSkipped)
				{
					visitedFrame.get()->SetPixel(c, r, true);
					sum += 1;
					if (suspected)
					{
						AddAdjacentPixelsToQueue(Q, c, r, 0);
						if (c > maxCol) maxCol = c;
						if (c < minCol) minCol = c;
						if (r > maxRow) maxRow = r;
						if (r < minRow) minRow = r;
					}
					else
					{
						AddAdjacentPixelsToQueue(Q, c, r, skipped + 1);
						outliersVideo.get()->SetPixel(c, r, fIdx, 1);
					}
				}
			}
		}
		meanCol = (minCol + maxCol) / 2;
		meanRow = (minRow + maxRow) / 2;
		return sum;
	}

	// add pixels neighbours if they are at image size
	inline void AddAdjacentPixelsToQueue(std::queue<int> & Q, const int & col, const int & row, const int & skipped)
	{
		for (int i = 0; i < 9; i++)
		{
			int r = row + (i / 3) - 1;
			int c = col + (i % 3) - 1;
			if (c != col || r != row) {
				if (c >= 0 && c < outliersVideo.get()->GetFrameAt(fIdx)->GetCol()
					&& r >= 0 && r < outliersVideo.get()->GetFrameAt(fIdx)->GetRow())
				{
					//cout << row << " | " << col << endl;
					Q.push(c);
					Q.push(r);
					Q.push(skipped);
				}
			}
		}
	}

	//here will be function to checking neighbourhoods
	void CheckNeighborhoods(const int & fIdx) 
	{
		const int & minArea = params.minAreaForSuspectOutliers;
		for (int row = 0; row < outliersVideo.get()->GetFrameAt(fIdx)->GetRow(); row++)
		{
			for (int col = 0; col < outliersVideo.get()->GetFrameAt(fIdx)->GetCol(); col++)
			{
				const ComputationalPixelType & centerPixelVal = outliersVideo.get()->GetPixel(col, row, fIdx);
				if (centerPixelVal >= minArea)
				{
					if (NeighbourExists(col, row, fIdx - 1, centerPixelVal) == true 
						|| NeighbourExists(col, row, fIdx + 1, centerPixelVal) == true)
					{
						ClearArea(col, row, fIdx);
					}
				}
			}
		}
	}

	//here will be function to checking neighbourhood
	bool NeighbourExists(const int & col, const int & row, const int & fIdx, const ComputationalPixelType & expectedArea)
	{
		const int & kRows = outliersVideo.get()->GetFrameAt(fIdx)->GetRow();
		const int & kCols = outliersVideo.get()->GetFrameAt(fIdx)->GetCol();
		const int & minArea = params.minAreaForSuspectOutliers;
		const double & radiusCoeff = params.radiusForCheckingNeighbours;
		const double & minAreaCoeff = params.minCoeffForCompareNeighboursAreas;
		const double & maxAreaCoeff = params.maxCoeffForCompareNeighboursAreas;
		const double radius = sqrt((double)expectedArea)/ (2 * M_PI);
		const int startCol = col - radius * radiusCoeff;
		const int stopCol = col + radius * radiusCoeff;
		const int startRow = row - radius * radiusCoeff;
		const int stopRow = row + radius * radiusCoeff;

		if (stopCol >= kCols || startCol < 0 || stopRow >= kRows || startRow < 0)
		{
			return false;
		}

		for (int r = startRow; r <= stopRow; r++)
		{
			for (int c = startCol; c <= stopCol; c++)
			{
				const ComputationalPixelType & area = outliersVideo.get()->GetPixel(c, r, fIdx);
				if (area > minArea)
				{
					if (area > (ComputationalPixelType)(minAreaCoeff * (double)expectedArea) 
						&& area < (ComputationalPixelType)(maxAreaCoeff * (double)expectedArea))
					{
						const int & dist = Distance(col, row, c, r);
						if (dist <= (int)(radiusCoeff * radius))
						{
							return true;
						}
					}	
				}
			}
		}
		return false;
	}

	//euclidean distance
	inline double Distance(const int & col, const int & row, const int & c, const int & r)
	{
		return sqrt(pow((col - c), 2) + pow((row - r), 2));
	}

	//area is not an outliers
	//tu uwaga na rogale
	void ClearArea(const int & col, const int & row, const int & fIdx)
	{
		std::queue<int> Q;
		Q.push(col);
		Q.push(row);
		Q.push(0);

		while (!Q.empty())
		{
			int c = Q.front(); Q.pop();
			int r = Q.front(); Q.pop();
			int skipped = Q.front(); Q.pop();
			if (outliersVideo.get()->GetPixel(c, r, fIdx) > 0)
			{
				outliersVideo.get()->SetPixel(c, r, fIdx, 0);
				AddAdjacentPixelsToQueue(Q, c, r, skipped + 1);
			}
		}
	}

	void ConfirmArea(const int & col, const int & row, const int & fIdx)
	{
		std::queue<int> Q;
		Q.push(col);
		Q.push(row);
		Q.push(0);

		while (!Q.empty())
		{
			int c = Q.front(); Q.pop();
			int r = Q.front(); Q.pop();
			int skipped = Q.front(); Q.pop();
			if (outliersVideo.get()->GetPixel(c, r, fIdx) == 1)
			{
				outliersVideo.get()->SetPixel(c, r, fIdx, 2);
				AddAdjacentPixelsToQueue(Q, c, r, skipped + 1);
			}
		}
	}

};

#endif
