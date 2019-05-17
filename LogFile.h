// LogFile.h: interface for the CLogFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_LOGFILE_H)
#define _LOGFILE_H

#include <afx.h>
#include <string.h>


#define DATETIMESIZE		32		// maxinum size of date/time field
#define DESCRIPTIONSIZE		160		// maxinum size of description fix by zhuxy 100->150 
#define RECORDSIZE			98		// maximum size of each record //78
#define MAXRECORDS			80000	// maximum number of records in the log (including header) //100

struct LogRecord
{
	char szDate[DATETIMESIZE];					// date/time of record entry
	char szDescription[DESCRIPTIONSIZE];		// description of the event being logged
	// leave 2 bytes for \r\n at the end of the record
};


class CLogFile : public CFile  
{
public:
	CLogFile();							// default contructor
	virtual ~CLogFile();

	void AddAppRecord(CString strFileName,LPCTSTR description);    //应用程序记录
	void AddAppRecord(LPCTSTR description);    //应用程序记录
	void AddCameraRecord(int nEvent, LPCTSTR description);    //相机记录
	void AddTrainInfoRecord(int nEvent, LPCTSTR description);  //过车记录
	CString GetLogPath(){ return strLogBaseFloder; }
	void clearRecord(CString strFileName);//add by zhuxy 清空过车记录
protected:
	void AddRecord(CString strFileName, LPCTSTR description);
	void AddRecord2(CString strFileName, LPCTSTR description);
	BOOL OpenLogFile(LPCTSTR filename);
	void CreateFloder();

private:
	CString strLogBaseFloder;	
};


#endif
