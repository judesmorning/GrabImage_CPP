#include <string.h>
class CInterpolation  
{
public:
	CInterpolation(double *ctrlPtX,double *ctrlPtY,int nCtrlPtCount);
	virtual ~CInterpolation();

public:
	bool GetInterpolationPts(int outPtCount,double* outPtX,double* outPtY);
	bool GetYByX(const double &dbInX, double &dbOutY);

protected:
	void ReleaseMem();
	void InitParam();
	bool InterPolation();
	
	//sunyb 2015/11/7
	//bool Intersl();
	bool Spline();

protected:
	bool m_bCreate; 
	int N;  
	int M;   
	typedef double* pDouble;

	pDouble X,Y;
	pDouble Z,F; 

	pDouble H,A,B,C,D; 

};

