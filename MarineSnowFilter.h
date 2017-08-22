#ifndef MARINESNOWFILTER_H
#define MARINESNOWFILTER_H

#include "MSFParams.h"

template <typename _InPixType, typename _ComputationalPixelType = long >
class MarineSnowFilterForColor;

template <typename _InPixType, typename _ComputationalPixelType = long >
class MarineSnowFilterForMonochrome;

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
				bool suspected = outliersVideo.get()->GetPixel(col, row, fIdx);

				if (suspected)
				{
					InPixType pixel = inputVideo.get()->GetPixel(col, row, fIdx);
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
	virtual InPixType MaxFromWindow(int c, int r, int fIdx) = 0;

	//calculate median value from window
	virtual InPixType MedFromWindow(int c, int r, int fIdx) = 0;

	//calculate average value from window
	virtual InPixType AverageFromWindow(int c, int r, int fIdx) = 0;

	//function to compare pixxels in connection with choosen parameters
	inline virtual bool Smaller(const InPixType & Lpixel, const InPixType & Rpixel) = 0;

	//here will be function to checking neighbourhood
};

#include "MarineSnowFilterForColor.h"
#include "MarineSnowFilterForMonochrome.h"
#endif
