/*
  XMPlay Just Log
*/

#include <windows.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <stdio.h>
#include "xmpdsp.h" // requires the XMPlay "DSP/general plugin SDK"

static XMPFUNC_MISC *xmpfmisc;
static XMPFUNC_FILE *xmpffile;

static HINSTANCE ghInstance;
static BOOL isUnicode;

static BOOL CALLBACK DSPDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void WINAPI DSP_About(HWND win);
static void *WINAPI DSP_New(void);
static void WINAPI DSP_Free(void *inst);
static const char *WINAPI DSP_GetDescription(void *inst);
static void WINAPI DSP_Config(void *inst, HWND win);
static DWORD WINAPI DSP_GetConfig(void *inst, void *config);
static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size);
static void WINAPI DSP_NewTrack(void *inst, const char *file);
static void WINAPI DSP_NewTitle(void *inst, const char *title);

// config structure
typedef struct {
    BOOL showCues;
    BOOL newLogSet;
    char filePath[255];
    char fileString[125];
    char actionConfig[25];
    char maxSize[5];
    BOOL excStream;
    BOOL excFile;
    char excType[50];
    char streamString[125];
	BOOL excUntitled;
	BOOL remPath;
} MSNStuff;
static MSNStuff msnConf;
std::string lasttitle;
XMPFILE xmpFile;
DWORD xmpType=-1;
std::string actualPath = "";
char tempPath[255];

static XMPDSP dsp = {
    XMPDSP_FLAG_TITLE,
    "Just Log",
    DSP_About,
    DSP_New,
    DSP_Free,
    DSP_GetDescription,
    DSP_Config,
    DSP_GetConfig,
    DSP_SetConfig,
	DSP_NewTrack,
	NULL,
	NULL,
	NULL,
	DSP_NewTitle,
};

static void revisePath(char *revisePath) {
	std::string calcPath;
	std::string setPath = revisePath;
	if (setPath == "") {
		setPath = msnConf.filePath;
	}
    if (setPath[0] == '.' || (setPath.find("\\/") == -1 && setPath.find("\\") == -1)) {
        TCHAR exePath[FILENAME_MAX];
        GetModuleFileName(nullptr, exePath, FILENAME_MAX);
        std::string::size_type slashPos = std::string(exePath).find_last_of("\\/");
        calcPath = std::string(exePath).substr(0, slashPos);
        calcPath.append("\\");
        calcPath.append(setPath);

		char fullFilename[255];
		GetFullPathName(calcPath.c_str(), 255, fullFilename, nullptr);
		calcPath = fullFilename;
    } else {
        calcPath = setPath;
    }
	actualPath = calcPath;
	actualPath[actualPath.size()] = '\0';
}

static void trimLog(std::string fileName, int maxSize, std::string datePrefix) {
	int fileSize;
	std::string readLine;
	std::string tempName;
	std::string archName;

	do {
		std::ifstream infile(actualPath, std::ifstream::ate | std::ifstream::binary);
		fileSize = infile.tellg();
		infile.close();
		if (fileSize > (maxSize * 1000)) {
			if (!msnConf.newLogSet) {
				// CLIP OLD
				tempName = fileName;
				tempName.replace((tempName.length()-4), 4, ".tmp");
				std::ifstream oldfile (fileName);
				std::ofstream tempfile (tempName);
				int ignoreOne = 0;
				while (getline(oldfile, readLine)) {
					if (ignoreOne != 0)
						tempfile << readLine << std::endl;
					ignoreOne = 1;
				}

				tempfile.close();
				oldfile.close();

				remove(fileName.c_str());
				rename(tempName.c_str(), fileName.c_str());
			} else {
				// RENAME OLD
				archName = actualPath;
				archName.replace((archName.length()-4), 4, " " + datePrefix + ".old");
				rename(actualPath.c_str(),archName.c_str());
			}
		}
	} while (fileSize > (maxSize * 1000));
}

static void WINAPI SetNowPlaying(BOOL close,std::string SRC)
{
	char *filename=NULL;
	char *title=NULL;
	char *alttitle=NULL;
	char *artist=NULL;
	char *album=NULL;
	char *date=NULL;
	char *track=NULL;
	char *genre=NULL;
	char *comment=NULL;
	char *filetype=NULL;
	int resultIdx;
	std::string resultStr;
	int excludeIdx;
	std::string excludeStr;
	std::string ignoreThis="";
	time_t theTime = time(NULL);
	struct tm *aTime = localtime(&theTime);
	std::stringstream dtYear;
	std::stringstream dtMonth;
	std::stringstream dtDay;
	std::stringstream dtHour;
	std::stringstream dtMinute;
	std::stringstream dtSecond;
	dtYear << std::setfill('0') << std::setw(4) << (aTime->tm_year + 1900);
	dtMonth << std::setfill('0') << std::setw(2) << (aTime->tm_mon + 1);
	dtDay << std::setfill('0') << std::setw(2) << (aTime->tm_mday);
	dtHour << std::setfill('0') << std::setw(2) << (aTime->tm_hour);
	dtMinute << std::setfill('0') << std::setw(2) << (aTime->tm_min);
	dtSecond << std::setfill('0') << std::setw(2) << (aTime->tm_sec);
	int maxfileSize = std::stoi(msnConf.maxSize);
	std::string cropName;
	int notitle=0;
	
	if (!close) {
		if (msnConf.showCues) title=xmpfmisc->GetTag(TAG_TRACK_TITLE); // get cue title
		if (!title) { title=xmpfmisc->GetTag(TAG_TITLE); } else { alttitle=xmpfmisc->GetTag(TAG_TITLE); } // get track title if no cue title
		if (!filename) filename=xmpfmisc->GetTag(TAG_FILENAME); // get track filename

		if (!title && !msnConf.excUntitled) {  // use filename if track title or cue title does not exist
			title=xmpfmisc->GetTag(TAG_FILENAME);
			notitle=1;
		}

		if (!artist) artist=xmpfmisc->GetTag(TAG_ARTIST); // get track artist
		if (!album) album=xmpfmisc->GetTag(TAG_ALBUM); // get track album
		if (!date) date=xmpfmisc->GetTag(TAG_DATE); // get track date
		if (!track) track=xmpfmisc->GetTag(TAG_TRACK); // get track number
		if (!genre) genre=xmpfmisc->GetTag(TAG_GENRE); // get track genre
		if (!comment) comment=xmpfmisc->GetTag(TAG_COMMENT); // get track comment
		if (!filetype) filetype=xmpfmisc->GetTag(TAG_FILETYPE); // get track filetype
	}

	if (msnConf.excFile || msnConf.excStream) {
		if (xmpType == 0) {
			ignoreThis="MEMORY";
		} else if (xmpType == 1 && msnConf.excFile) {
			ignoreThis="FILE";
		} else if (xmpType == 2 && msnConf.excStream) {
			ignoreThis="NETFILE";
		} else if (xmpType == 3 && msnConf.excStream) {
			ignoreThis="NETSTREAM";
		}
	}
	if (strlen(msnConf.excType) > 0 && filetype) {
		excludeStr = "";
		for (excludeIdx = 0; excludeIdx < strlen(msnConf.excType); excludeIdx++) {
			excludeStr += msnConf.excType[excludeIdx];
		}
		if (excludeStr.find(filetype) != -1) {
			ignoreThis="TYPEDENIED";
		}
	}

	if (title) {
		if (ignoreThis == "") {
			if (title != lasttitle) {
				// TRACK LAST 
				lasttitle = title;

				// BUILD STRING FOR LOG FILE
				resultStr = "";
				if (xmpType == 1) {
					for (resultIdx = 0; resultIdx < sizeof(msnConf.fileString); resultIdx++) {
						resultStr += msnConf.fileString[resultIdx];
					}
				} else if (xmpType == 2 || xmpType == 3) {
					for (resultIdx = 0; resultIdx < sizeof(msnConf.streamString); resultIdx++) {
						resultStr += msnConf.streamString[resultIdx];
					}
				}
				while (resultStr.find("%1") != -1) {
					if (!title || notitle==1) {
						resultStr.replace(resultStr.find("%1"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%1"), 2, title);
					}
				}
				while (resultStr.find("%2") != -1) {
					if (!artist) {
						resultStr.replace(resultStr.find("%2"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%2"), 2, artist);
					}
				}
				while (resultStr.find("%3") != -1) {
					if (!album) {
						resultStr.replace(resultStr.find("%3"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%3"), 2, album);
					}
				}
				while (resultStr.find("%4") != -1) {
					if (!date) {
						resultStr.replace(resultStr.find("%4"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%4"), 2, date);
					}
				}
				while (resultStr.find("%5") != -1) {
					if (!track) {
						resultStr.replace(resultStr.find("%5"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%5"), 2, track);
					}
				}
				while (resultStr.find("%6") != -1) {
					if (!genre) {
						resultStr.replace(resultStr.find("%6"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%6"), 2, genre);
					}
				}
				while (resultStr.find("%7") != -1) {
					if (!comment) {
						resultStr.replace(resultStr.find("%7"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%7"), 2, comment);
					}
				}
				while (resultStr.find("%8") != -1) {
					if (!filetype) {
						resultStr.replace(resultStr.find("%8"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%8"), 2, filetype);
					}
				}
				while (resultStr.find("%0") != -1) {
					if (!filetype) {
						resultStr.replace(resultStr.find("%0"), 2, "-");
					} else {
						cropName = filename;
						if (msnConf.remPath) {
							cropName = cropName.substr(cropName.find_last_of("\\/")+1);
						}
						resultStr.replace(resultStr.find("%0"), 2, cropName);
					}
				}
				while (resultStr.find("%z") != -1) {
					if (!alttitle) {
						resultStr.replace(resultStr.find("%z"), 2, "-");
					} else {
						resultStr.replace(resultStr.find("%z"), 2, alttitle);
					}
				}
				while (resultStr.find("%y") != -1) {
					resultStr.replace(resultStr.find("%y"), 2, dtYear.str());
				}
				while (resultStr.find("%m") != -1) {
					resultStr.replace(resultStr.find("%m"), 2, dtMonth.str());
				}
				while (resultStr.find("%d") != -1) {
					resultStr.replace(resultStr.find("%d"), 2, dtDay.str());
				}
				while (resultStr.find("%h") != -1) {
					resultStr.replace(resultStr.find("%h"), 2, dtHour.str());
				}
				while (resultStr.find("%i") != -1) {
					resultStr.replace(resultStr.find("%i"), 2, dtMinute.str());
				}
				while (resultStr.find("%s") != -1) {
					resultStr.replace(resultStr.find("%s"), 2, dtSecond.str());
				}
				while (resultStr.find("%n") != -1) {
					resultStr.replace(resultStr.find("%n"), 2, "\n");
				}
				while (resultStr.find("%t") != -1) {
					resultStr.replace(resultStr.find("%t"), 2, "\t");
				}

				// CHECK LOG SIZE IF NECESSARY
				if (maxfileSize > 0) {
					trimLog(actualPath,maxfileSize,(dtYear.str() + dtMonth.str() + dtDay.str() + " " + dtHour.str() + dtMinute.str() + dtSecond.str()));
				}

				// APPLY STRING TO LOG FILE
				if (strstr(msnConf.actionConfig, "Replace")) {
					std::ofstream outfile (actualPath);
					outfile << resultStr.c_str() << std::endl;
					outfile.close();
				} else if (strstr(msnConf.actionConfig, "Append")) {
					std::ofstream outfile (actualPath, std::ios_base::app);
					outfile << resultStr.c_str() << std::endl;
					outfile.close();
				}
			}
		}
	}
	
	if (title) {
		xmpfmisc->Free(title);
		xmpfmisc->Free(artist);
		xmpfmisc->Free(album);
		xmpfmisc->Free(filetype);
	}
}

static void WINAPI DSP_NewTrack(void *inst, const char *file)
{
	// MAP
	if (actualPath == "") {
		revisePath("");
	}
	// PERFORM
	lasttitle = "";
	if (file != NULL) {
		xmpFile = xmpffile->Open(file);
		xmpType = xmpffile->GetType(xmpFile);
		xmpffile->Close(xmpFile);
		xmpfmisc->Free(xmpFile);

		SetNowPlaying(FALSE,"NEWFILE");
	} else {
		xmpType = -1;
	}
}

static void WINAPI DSP_NewTitle(void *inst, const char *title)
{
	// MAP
	if (actualPath == "") {
		revisePath("");
	}
	// PERFORM
	lasttitle = "";
	if (xmpType != -1) {
		SetNowPlaying(FALSE,"NEWTITLE");
	}
}

static void WINAPI DSP_About(HWND win)
{
	MessageBox(win,
		"Just Log - General Plugin\nCopyright (c) 2021 Nathan Hindley",
		"Just Log - General Plugin [v1.9]",
		MB_ICONINFORMATION);
}

static const char *WINAPI DSP_GetDescription(void *inst)
{
    return dsp.name;
}

static void *WINAPI DSP_New()
{
    msnConf.showCues = TRUE;
    msnConf.newLogSet = TRUE;
    msnConf.excStream = FALSE;
    msnConf.excFile = FALSE;
	std::string dfPath = "log.txt";
	for(int i = 0; i < dfPath.length(); ++i){
		msnConf.filePath[i] = dfPath[i];
	}
	std::string dfFormat = "%y-%m-%d %h:%i:%s %1 %2 %3 %8";
	for(int i = 0; i < dfFormat.length(); ++i){
		msnConf.fileString[i] = dfFormat[i];
		msnConf.streamString[i] = dfFormat[i];
	}
	std::string dfConfig = "Replace";
	for(int i = 0; i < dfConfig.length(); ++i){
		msnConf.actionConfig[i] = dfConfig[i];
	}
	std::string dfSize = "0";
	for(int i = 0; i < dfSize.length(); ++i){
		msnConf.maxSize[i] = dfSize[i];
	}
    msnConf.excUntitled = TRUE;
    msnConf.remPath = FALSE;

    return (void*)1;
}

static void WINAPI DSP_Free(void *inst)
{
	// DO NOTHING
}

static void WINAPI DSP_Config(void *inst, HWND win)
{
    DialogBox(ghInstance, MAKEINTRESOURCE(1000), win, &DSPDialogProc);
}

static DWORD WINAPI DSP_GetConfig(void *inst, void *config)
{
    memcpy(config, &msnConf, sizeof(msnConf));
    return sizeof(msnConf);
}

static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size)
{
    memcpy(&msnConf, config, min(size,sizeof(msnConf)));
    return TRUE;
}

#define MESS(id,m,w,l) SendDlgItemMessage(hWnd,id,m,(WPARAM)(w),(LPARAM)(l))

static BOOL CALLBACK DSPDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	TCHAR buff[1024];
	HWND szPath = GetDlgItem(hWnd, 99001);
	switch (msg) {
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case 80:
					MESS(99001, WM_GETTEXT, 255, tempPath);
					revisePath(tempPath);
					MESS(70, WM_SETTEXT, 0, actualPath.c_str());
					break;
				case IDOK:
					msnConf.showCues = (BST_CHECKED==MESS(10, BM_GETCHECK, 0, 0));
					msnConf.newLogSet = (BST_CHECKED==MESS(20, BM_GETCHECK, 0, 0));
					msnConf.excFile = (BST_CHECKED==MESS(30, BM_GETCHECK, 0, 0));
					msnConf.excStream = (BST_CHECKED==MESS(40, BM_GETCHECK, 0, 0));
					MESS(99001, WM_GETTEXT, 255, msnConf.filePath);
					MESS(99002, WM_GETTEXT, 125, msnConf.fileString);
					MESS(99003, WM_GETTEXT, 25, msnConf.actionConfig);
					MESS(99004, WM_GETTEXT, 5, msnConf.maxSize);
					MESS(99005, WM_GETTEXT, 50, msnConf.excType);
					MESS(99006, WM_GETTEXT, 125, msnConf.streamString);
					msnConf.excUntitled = (BST_CHECKED==MESS(50, BM_GETCHECK, 0, 0));
					msnConf.remPath = (BST_CHECKED==MESS(60, BM_GETCHECK, 0, 0));
					revisePath("");
				case IDCANCEL:
					revisePath("");
					EndDialog(hWnd, 0);
					break;
			}
			break;
        case WM_INITDIALOG:
			HWND szAction = GetDlgItem(hWnd, 99003);
			revisePath("");
			SendMessage(szAction, (UINT) CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT ("Append"));
			SendMessage(szAction, (UINT) CB_ADDSTRING, (WPARAM) 0, (LPARAM) TEXT ("Replace"));
			SendMessage(szAction, CB_SELECTSTRING, (WPARAM) -1, (LPARAM) msnConf.actionConfig);
			MESS(10, BM_SETCHECK, msnConf.showCues?BST_CHECKED:BST_UNCHECKED, 0);
			MESS(20, BM_SETCHECK, msnConf.newLogSet?BST_CHECKED:BST_UNCHECKED, 0);
			MESS(30, BM_SETCHECK, msnConf.excFile?BST_CHECKED:BST_UNCHECKED, 0);
			MESS(40, BM_SETCHECK, msnConf.excStream?BST_CHECKED:BST_UNCHECKED, 0);
			SetDlgItemText(hWnd, 99001, msnConf.filePath);
			SetDlgItemText(hWnd, 99002, msnConf.fileString);
			SetDlgItemText(hWnd, 99004, msnConf.maxSize);
			SetDlgItemText(hWnd, 99005, msnConf.excType);
			SetDlgItemText(hWnd, 99006, msnConf.streamString);
			MESS(50, BM_SETCHECK, msnConf.excUntitled?BST_CHECKED:BST_UNCHECKED, 0);
			MESS(60, BM_SETCHECK, msnConf.remPath?BST_CHECKED:BST_UNCHECKED, 0);
			MESS(70, WM_SETTEXT, 0, actualPath.c_str());
			return TRUE;
    }
	return FALSE;
}

// get the plugin's XMPDSP interface
#ifdef __cplusplus
extern "C"
#endif
XMPDSP *WINAPI XMPDSP_GetInterface2(DWORD face, InterfaceProc faceproc)
{
	if (face!=XMPDSP_FACE) return NULL;
	xmpfmisc=(XMPFUNC_MISC*)faceproc(XMPFUNC_MISC_FACE); // import "misc" functions
	xmpffile=(XMPFUNC_FILE*)faceproc(XMPFUNC_FILE_FACE); // import "file" functions
	return &dsp;
}

BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
    if (reason==DLL_PROCESS_ATTACH) {
        ghInstance=hDLL;
        DisableThreadLibraryCalls(hDLL);
		isUnicode=(int)GetVersion()>=0;
    }
    return 1;
}
