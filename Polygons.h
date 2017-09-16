#ifndef POLYGONS_H
#define POLYGONS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class Polygons {
public:

	MIAP image;
	vector<vector<int>> colsList;
	vector<vector<int>> rowsList;

	Polygons(int cols, int rows)
	{
		image = MIAP(new MonochromeImage(cols, rows, 0));
	}

	~Polygons() 
	{
		delete image.get();
	}

	void drawLine(int c1, int r1, int c2, int r2)
	{
		int lc = c2 - c1;
		int lr = r2 - r1;
		int length = max(abs(lc), abs(lr));
		double dc = double(lc) / double(length);
		double dr = double(lr) / double(length);
		double c(c1);
		double r(r1);
		for (int i = 0; i <= length; i++)
		{
			image.get()->SetPixel(int(c), int(r), 255);
			c += dc;
			r += dr;
		}
	}

	void loadMetaData(string path)
	{
		ifstream metaData(path);
		int counter = -1;
		if (metaData.is_open())
		{	
			string tmp;
			int counter = -1;
			metaData >> tmp;
			while (!metaData.eof())
			{
				if (tmp[tmp.length() - 1] == ':')
				{
					colsList.push_back(vector<int>());
					rowsList.push_back(vector<int>());
					counter++;
				}
				else
				{
					colsList[counter].push_back(std::stoi(tmp));
					metaData >> tmp;
					rowsList[counter].push_back(std::stoi(tmp));
				}
				metaData >> tmp;
			}
			metaData.close();
		}
	}

	int draw() 
	{
		if (colsList.size() != rowsList.size()) return - 1;
		for (int i = 0; i < colsList.size(); i++)
		{
			//cout << "row " << i << endl;
			if (colsList[i].size() != rowsList[i].size()) return - 2;
			int len = colsList[i].size();
			int minCol = image.get()->GetCol();
			int minRow = image.get()->GetRow();
			int maxCol = 0;
			int maxRow = 0;
			for (int j = 0; j < len; j++)
			{
				if (colsList[i][j] > maxCol) maxCol = colsList[i][j];
				if (rowsList[i][j] > maxRow) maxRow = rowsList[i][j];
				if (colsList[i][j] < minCol) minCol = colsList[i][j];
				if (rowsList[i][j] < minRow) minRow = rowsList[i][j];
				drawLine(colsList[i][j], rowsList[i][j], colsList[i][(j + 1) % len], rowsList[i][(j + 1) % len]);
			}
			if (fill((maxCol + minCol) / 2, (maxRow + minRow) / 2) != 0) return -3;
		}
		return 0;
	}

	int fill(int col, int row)
	{
		std::queue<int> Q;
		Q.push(col);
		Q.push(row);
		image.get()->SetPixel(col, row, 255);

		while (!Q.empty())
		{
			int c = Q.front(); Q.pop();
			int r = Q.front(); Q.pop();
			int mask[] = { 0, 0, 1, -1 };
			for (int i = 0; i < 4; i++)
			{
				int c1 = c + mask[i];
				int r1 = r + mask[(i + 2) % 4];
				if (image.get()->GetPixel(c1, r1) == 0)
				{
					image.get()->SetPixel(c1, r1, 255);
					Q.push(c1);
					Q.push(r1);
				}
			}
			if (Q.size() > 1000000) return -1;
		}
		return 0;
	}

	int drawAndSave(string path)
	{
		if (draw() != 0) return -1;
		Save_JPEG_Image(*image.get(), path);
	}

	MIAP drawAndReturn()
	{
		if (draw() != 0) return MIAP();
		return image;
	}
};



/*void fill()
{
bool insideFlag = false;
int oldVal = 0;
int actualVal;
int counter = 0;
for (int row = 0; row < image.get()->GetRow(); row++)
{
for (int col = 0; col < image.get()->GetCol(); col++)
{
actualVal = image.get()->GetPixel(col, row);
if (oldVal == 0 && actualVal == 255) insideFlag = ~insideFlag;
if (insideFlag == true)
{
image.get()->SetPixel(col, row, 255);
counter++;
}
oldVal = actualVal;
}
if (counter > 5) break;
}
}*/


#endif
