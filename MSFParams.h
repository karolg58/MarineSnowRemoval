#ifndef MSFPARAMS_H
#define MSFPARAMS_H

class MSFparams {
public:
	int sectorsRGBnumber = 9;		//should be n^2, picture is divided into sectorsRGBnumber regions and within is calculated mean value of RGB distance
	double RGBdistanceCoeff = 0.8;	//pixel must has lower RGBdistance than RGBdistanceCoeff * meanRGBdistance from sector
	int RGBsectorsPercent = 100;	//percent of sectors (contains pixel which is checked) with a positive results of RGBdistance condition, must exist to pass checkingRGBdistance
	
	int typeForTimeComparison = 1;	//0 -> max, 1 -> median, 2 -> average
	bool channelsToCompare[3] = { true, true, true };//R, G, B
	
	int sizeWindowForTimeComparison = 5;//n -> window n x n
	double windowValueCoeff = 1.0;	

	int availableSkippedPixelsForFindingArea = 5;

	int minAreaForSuspectOutliers = 9;
	double minRadiusForCheckingNeighbours = 8.0; //relative for blur size
	double maxRadiusForCheckingNeighbours = 20.0; 
	double minCoeffForCompareNeighboursAreas = 0.5;
	double maxCoeffForCompareNeighboursAreas = 1 / minCoeffForCompareNeighboursAreas;

	friend ostream & operator<< (ostream &output, const MSFparams params){
		return output
			<< params.sectorsRGBnumber << endl
			<< params.RGBdistanceCoeff << endl
			<< params.RGBsectorsPercent << endl
			<< params.typeForTimeComparison << endl
			<< params.sizeWindowForTimeComparison << endl
			<< params.windowValueCoeff << endl
			<< params.availableSkippedPixelsForFindingArea << endl
			<< params.minAreaForSuspectOutliers << endl
			<< params.minRadiusForCheckingNeighbours << endl
			<< params.maxRadiusForCheckingNeighbours << endl
			<< params.minCoeffForCompareNeighboursAreas << endl
			<< params.maxCoeffForCompareNeighboursAreas << endl;
	}
};

#endif
