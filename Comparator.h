#ifndef COMPARATOR_H
#define COMPARATOR_H

class Comparator {
public:
	int operator()(const MonochromeImage & userImage, const MonochromeImage & filterImage)
	{
		int kCols = userImage.GetCol();
		int kRows = userImage.GetRow();
		if (filterImage.GetCol() != kCols)  return -1;
		if (filterImage.GetRow() != kRows)  return -1;
		int counter = 0;
		for (int row = 0; row < kRows; row++)
		{
			for (int col = 0; col < kCols; col++)
			{
				unsigned char userPixel = userImage.GetPixel(col, row);
				unsigned char filterPixel = filterImage.GetPixel(col, row);
				if (userPixel == 0 && filterPixel != 0) counter++;
				else if (userPixel != 0 && filterPixel == 0) counter++;
			}
		}
		return counter;
	}
};

#endif
