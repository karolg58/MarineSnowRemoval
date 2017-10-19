#ifndef MSFPARAMS_H
#define MSFPARAMS_H

class MSFparams {
public:
	int sectorsRGBnumber = 9;		//should be n^2, picture is divided into sectorsRGBnumber regions and within is calculated mean value of RGB distance
	double RGBdistanceCoeff = 0.8;	//pixel must has lower RGBdistance than RGBdistanceCoeff * meanRGBdistance from sector
	int RGBsectorsPercent = 100;	//percent of sectors (contains pixel which is checked) with a positive results of RGBdistance condition, must exist to pass checkingRGBdistance
	
	int typeForTimeComparison = 0;	//0 -> max, 1 -> median, 2 -> average
	bool channelsToCompare[3] = { true, true, true };//R, G, B
	
	int sizeWindowForTimeComparison = 1;//n -> window n x n
	double windowValueCoeff = 0.7;	

	int availableSkippedPixelsForFindingArea = 3;

	int minAreaForSuspectOutliers = 15;
	double radiusForCheckingNeighbours = 5.0; //relative for blur size 
	double minCoeffForCompareNeighboursAreas = 0.75;
	double maxCoeffForCompareNeighboursAreas = 1 / minCoeffForCompareNeighboursAreas;

	friend ostream & operator<< (ostream &output, const MSFparams params){
		return output
			<< params.sectorsRGBnumber << ","
			<< params.RGBdistanceCoeff << ","
			<< params.RGBsectorsPercent << ","
			<< params.typeForTimeComparison << ","
			<< params.sizeWindowForTimeComparison << ","
			<< params.windowValueCoeff << ","
			<< params.availableSkippedPixelsForFindingArea << ","
			<< params.minAreaForSuspectOutliers << ","
			<< params.radiusForCheckingNeighbours << ","
			<< params.minCoeffForCompareNeighboursAreas << ","
			<< params.maxCoeffForCompareNeighboursAreas;
	}
};

#endif
