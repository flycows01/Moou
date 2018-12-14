
#define STUP_API extern "C" __declspec(dllexport)

STUP_API int GetStub(void);
STUP_API void SetStub(TCHAR* strPath);
STUP_API void StubEntryPoint();

typedef struct _SecInfo
{
	DWORD ComSize;
	DWORD SecSize;
	DWORD Sec1Fva;
	DWORD Sec1Size;
	DWORD Sec2Fva;
	DWORD Sec2Size;
	DWORD Sec3Fva;
	DWORD Sec3Size;
}SecInfo, *pSecInfo;
//定义数据结构体
typedef struct _StubConf
{
	DWORD srcOep;
	DWORD textScnRva;
	DWORD textScnSize;
	DWORD rdataScnRva;
	DWORD rdataScnSize;
	DWORD key;
	DWORD relocScnRva;
	DWORD relocScnSize;
	DWORD tlsRva;
	DWORD ImpRva;
	SecInfo ThrSecInf;
}StubConf;

STUP_API StubConf g_StubConf;

