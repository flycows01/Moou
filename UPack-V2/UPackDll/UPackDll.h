#pragma once
#include "Section.h"

#define UPACK_API extern "C" __declspec(dllexport)

UPACK_API BOOL UPack(TCHAR* strPath);
extern Section m_FileObj;
extern Section m_DllObj;

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
typedef struct _StubInfo
{
	HMODULE DllBase;
	DWORD pfnStart; // stub.dll(start)导出函数的地址
	StubConf* StubConf;
}StubInfo, *pStubInfo;














