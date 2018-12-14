// UPackDll.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "UPackDll.h"
#include "lz4.h"
//由于DLL不支持CString，所以在中用#include <afxwin.h>替换#include <Windows.h>

Section m_FileObj;
Section m_DllObj;

typedef int(*FunGetStub)(void);
typedef void(*FunSetStub)(TCHAR* strPath);
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN：加载stub函数    														      //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
void loadStub(StubInfo* Stub)
{
	// 1. 通过LoadLibrary加载stub.dll
	Stub->DllBase = LoadLibraryEx(L"stub.dll",
		NULL,
		DONT_RESOLVE_DLL_REFERENCES);
	m_DllObj.SetlpBase(Stub->DllBase);
	m_DllObj.ResaveHeader();
	// 2. 获取dll的导出函数
	Stub->pfnStart = (DWORD)GetProcAddress(Stub->DllBase,"StubEntryPoint");
	Stub->StubConf = (StubConf*)GetProcAddress(Stub->DllBase,
		"g_StubConf");
}

void fixStubRelocation();
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN：修正重定位函数    														      //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModifyReloc()
{
	// 1. 找到重定位表(stub.dll).
	DWORD relRva = m_DllObj.GetNtHeader()->OptionalHeader.DataDirectory[5].VirtualAddress;
	IMAGE_BASE_RELOCATION* pRel = (IMAGE_BASE_RELOCATION*)
		(relRva + (DWORD)m_DllObj.GetDosHeader());
	// 2. 遍历重定位表. 
	while (pRel->SizeOfBlock)
	{
		DWORD pMdify = pRel->VirtualAddress - m_DllObj.GetSection(".text")->VirtualAddress 
			          + m_FileObj.GetSection(".Ultra")->VirtualAddress;
		DWORD OldProt = 0;
		DWORD OldVA = m_DllObj.GetSection(".reloc")->VirtualAddress + (DWORD)m_DllObj.GetlpBase();
		VirtualProtect((LPVOID)OldVA, 4, PAGE_READWRITE, &OldProt);
		pRel->VirtualAddress = pMdify;
		VirtualProtect((LPVOID)OldVA, 4, PAGE_READWRITE, &OldProt);
		pRel = (IMAGE_BASE_RELOCATION*)((DWORD)pRel + pRel->SizeOfBlock);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN：TLS处理函数											         			  //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////

DWORD HandleTLS()
{
	PIMAGE_NT_HEADERS pNt =  m_FileObj.GetNtHeader();
	DWORD TlsRVA = pNt->OptionalHeader.DataDirectory[9].VirtualAddress;
	return TlsRVA;
}

DWORD PackPe(SecInfo * ThrSec);
void CopySection(char * Src, char* dst, DWORD dwSize, PIMAGE_NT_HEADERS pNt, int n);
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN：加壳器主函数函数														      //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UPack(TCHAR* strPath)
{
	//************************ 读取加壳程序代码段*************************************

	m_FileObj.GetFileData(strPath);
	m_FileObj.ResaveHeader();

	//******************** 加密被加壳程序的代码段*************************************

	char* pTargetBuff = m_FileObj.GetlpBase();	
	unsigned char* pTargetText =
		m_FileObj.GetSection(".text")->PointerToRawData
		+ (unsigned char*)pTargetBuff;
	DWORD dwTargetTextSize = m_FileObj.GetSection(".text")->Misc.VirtualSize;

	for (DWORD i = 0; i < dwTargetTextSize; ++i)
	{
		pTargetText[i] ^= 0x15;
	}
	//**************************************************************
	pTargetBuff = m_FileObj.GetlpBase();
	pTargetText =
		m_FileObj.GetSection(".rdata")->PointerToRawData
		+ (unsigned char*)pTargetBuff;
	dwTargetTextSize = m_FileObj.GetSection(".rdata")->Misc.VirtualSize;

	for (DWORD i = 0; i < dwTargetTextSize; ++i)
	{
		pTargetText[i] ^= 0x15;
	}
	
	//***************************** 修改目录表信息*************************************
	PIMAGE_NT_HEADERS pNt0 = m_FileObj.GetNtHeader();
	pNt0->OptionalHeader.DataDirectory[6].VirtualAddress = 0;
	pNt0->OptionalHeader.DataDirectory[6].Size = 0;
	pNt0->OptionalHeader.DataDirectory[10].VirtualAddress = 0;
	pNt0->OptionalHeader.DataDirectory[12].VirtualAddress = 0;
	pNt0->OptionalHeader.DataDirectory[2].VirtualAddress = 0;
	//***************************** 调用壳DLL程序*************************************
	StubInfo stub = {0};
	loadStub(&stub);           // BUG忘记在拷贝数据之前压缩相关的数据保存
	//***************************** 压缩原程序代码*************************************
	SecInfo &ThrSec = stub.StubConf->ThrSecInf;
	DWORD srcSize = PackPe(&ThrSec);
	int BuffSize = LZ4_compressBound(srcSize);          //srcSize
	char* LzBuff = new char[BuffSize]{};                //dst
	void* src = (void*)
		(ThrSec.Sec1Fva + (DWORD)m_FileObj.GetlpBase());                 //

	DWORD WriteSize = LZ4_compress_default((char*)src, LzBuff, srcSize, BuffSize);
	stub.StubConf->ThrSecInf.ComSize = WriteSize;
	if (!WriteSize)
	{
		MessageBox(0, L"压缩失败！", 0, 0);
	}
	stub.StubConf->key = 0x15;
	stub.StubConf->textScnRva = m_FileObj.GetSection(".text")->VirtualAddress;
	stub.StubConf->textScnSize = m_FileObj.GetSection(".text")->Misc.VirtualSize;
	stub.StubConf->rdataScnRva = m_FileObj.GetSection(".rdata")->VirtualAddress;
	stub.StubConf->rdataScnSize = m_FileObj.GetSection(".rdata")->Misc.VirtualSize;
	stub.StubConf->srcOep = m_FileObj.GetNtHeader()->OptionalHeader.AddressOfEntryPoint;
	stub.StubConf->relocScnRva = m_FileObj.GetNtHeader()->OptionalHeader.DataDirectory[5].VirtualAddress;
	stub.StubConf->relocScnSize = m_FileObj.GetNtHeader()->OptionalHeader.DataDirectory[5].Size;
	stub.StubConf->ImpRva = pNt0->OptionalHeader.DataDirectory[1].VirtualAddress;
	pNt0->OptionalHeader.DataDirectory[1].VirtualAddress = 0;

	if (m_FileObj.IsTLSFlag())
	{
		stub.StubConf->tlsRva = m_FileObj.GetNtHeader()->OptionalHeader.DataDirectory[9].VirtualAddress;
	}
	//************************ 增加新区段代码段*************************************

	DWORD nAddSize = m_DllObj.GetSection(".text")->Misc.VirtualSize;
	m_FileObj.AddSection(nAddSize, ".Ultra");
	
	//修改重定位项
	fixStubRelocation();

	//************************ 拷贝至新区段代码段*************************************
	PIMAGE_SECTION_HEADER pLastSec = m_FileObj.GetLastSection();
	DWORD RVA = pLastSec->PointerToRawData;
	char* lpBase = (char*)(RVA + (DWORD)m_FileObj.GetlpBase());

	char* RVA2 = (char*)(m_DllObj.GetSection(".text")->VirtualAddress + (DWORD)stub.DllBase);
	int nSize = m_DllObj.GetSection(".text")->Misc.VirtualSize;
	memcpy(lpBase,
		RVA2,
		nSize);

	//************************ 修改OEP代码段*************************************
	PIMAGE_SECTION_HEADER ptextSec = m_DllObj.GetSection(".text");
	PIMAGE_NT_HEADERS &pNt = m_FileObj.GetNtHeader();

	pNt->OptionalHeader.AddressOfEntryPoint
		= stub.pfnStart - (DWORD)stub.DllBase -
		ptextSec->VirtualAddress +            //减去相对于dll中.text段的偏移
		pLastSec->VirtualAddress;           //加上在文件中新加段的起始地址  即为函数地址相对于新加段的偏移

	//************************ 修正随机基址代码段*************************************
	//pNt->OptionalHeader.DllCharacteristics &= (~0x40);

	//增加reloc区段
	nAddSize = m_DllObj.GetSection(".reloc")->Misc.VirtualSize;
	m_FileObj.AddSection(nAddSize, ".Ultrc");

	ModifyReloc();                        //修改DLL重定位表

	//拷贝代码至新区段
	pLastSec = m_FileObj.GetSection(".Ultrc");
	RVA = pLastSec->PointerToRawData;
	lpBase = (char*)(RVA + (DWORD)m_FileObj.GetlpBase());

	RVA2 = (char*)(m_DllObj.GetSection(".reloc")->VirtualAddress + (DWORD)stub.DllBase);
	nSize = m_DllObj.GetSection(".reloc")->Misc.VirtualSize;
	memcpy(lpBase,
		RVA2,
		nSize);

	//将重定位表信息修改至目录表

	pNt->OptionalHeader.DataDirectory[5].VirtualAddress = pLastSec->VirtualAddress;
	pNt->OptionalHeader.DataDirectory[5].Size = nSize;

	//***************************** 压缩原程序代码*************************************
	//SecInfo &ThrSec = stub.StubConf->ThrSecInf;
	//DWORD srcSize = PackPe(&ThrSec);
	//int BuffSize = LZ4_compressBound(srcSize);          //srcSize
	//char* LzBuff = new char[BuffSize]{};                //dst
	//unsigned char* src = (unsigned char*)  
	//	(ThrSec.Sec1Fva	+ (DWORD)m_FileObj.GetlpBase());                 //

	//DWORD WriteSize = LZ4_compress_default((const char*)src, LzBuff, srcSize, BuffSize);
	//stub.StubConf->ThrSecInf.ComSize = WriteSize;
	//if (!WriteSize)
	//{
	//	MessageBox(0, L"压缩失败！", 0, 0);
	//}

	//拷贝区段位置信息
	WriteSize = m_FileObj.Aligment(WriteSize, pNt->OptionalHeader.FileAlignment);
	char* DstFoa = (char*)(ThrSec.Sec1Fva + (DWORD)m_FileObj.GetlpBase());
	CopySection(LzBuff, DstFoa, WriteSize, pNt, 0);
	DWORD A2B = srcSize - WriteSize;

	m_FileObj.PackPeSection(DstFoa, A2B);      //修改从第四个开始的区段的PointerToRawData 
	delete []LzBuff;

	//************************ 创建新文件代码段*************************************
	m_FileObj.WriteNewPE(1);
	
	//函数调用
	FunGetStub pfnGetStub = (FunGetStub)GetProcAddress(stub.DllBase, "GetStub");
	if (pfnGetStub())
	{
		MessageBox(NULL,L"你的程序已经全副武装^-^",L"Ultracis",0);
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN：壳代码数据重定位函数														  //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
void fixStubRelocation()
{
	// 1. 找到重定位表(stub.dll).
	DWORD relRva = m_DllObj.GetNtHeader()->OptionalHeader.DataDirectory[5].VirtualAddress;
	IMAGE_BASE_RELOCATION* pRel = (IMAGE_BASE_RELOCATION*)
		(relRva + (DWORD)m_DllObj.GetDosHeader());
	// 2. 遍历重定位表. 
	while (pRel->SizeOfBlock)
	{
		struct TypeOffset
		{
			WORD offset : 12;
			WORD type : 4;
		};
		TypeOffset* pTypeOfs = (TypeOffset*)(pRel + 1);
		DWORD count = (pRel->SizeOfBlock - 8) / 2;
		for (DWORD i = 0; i < count; ++i)
		{
			if (pTypeOfs[i].type != 3)
				continue;
			// 3. 将重定位项修改:
			DWORD* pFixAddr = (DWORD*)
				(pRel->VirtualAddress
				+ pTypeOfs[i].offset
				+ (DWORD)m_DllObj.GetDosHeader());

			DWORD old;
			VirtualProtect(pFixAddr, 4, PAGE_READWRITE, &old);								// 去掉dll当前加载基址
			*pFixAddr -= (DWORD)m_DllObj.GetlpBase();									// 去掉默认的段首rva
			*pFixAddr -= m_DllObj.GetSection(".text")->VirtualAddress;						// 换上目标文件的加载基址
			*pFixAddr += (DWORD)m_FileObj.GetNtHeader()->OptionalHeader.ImageBase;          // 换上新区段的段首rva
			DWORD LASTRVA = m_FileObj.GetLastSection()->VirtualAddress;
			*pFixAddr += LASTRVA;
			VirtualProtect(pFixAddr, 4, old, &old);
		}

		// 切换到下一个
		pRel = (IMAGE_BASE_RELOCATION*)((DWORD)pRel + pRel->SizeOfBlock);
	}

}

DWORD PackPe(SecInfo* ThrSec)
{
	DWORD nSize = 0;
	PIMAGE_NT_HEADERS pNts = m_FileObj.GetNtHeader();
	PIMAGE_SECTION_HEADER pSec = IMAGE_FIRST_SECTION(pNts);
	DWORD dwSecNum = pNts->FileHeader.NumberOfSections;
	
	if (pSec->Name)
	{
		ThrSec->Sec1Fva = pSec->PointerToRawData;
		ThrSec->Sec1Size = pSec->SizeOfRawData;
		nSize += pSec->SizeOfRawData;
	}
	pSec++;
	
	if (pSec->Name)
	{
		ThrSec->Sec2Fva = pSec->PointerToRawData;
		ThrSec->Sec2Size = pSec->SizeOfRawData;
		nSize += pSec->SizeOfRawData;
	}
	pSec++;

	if (pSec->Name)
	{
		ThrSec->Sec3Fva = pSec->PointerToRawData;
		ThrSec->Sec3Size = pSec->SizeOfRawData;
		nSize += pSec->SizeOfRawData;
	}
	ThrSec->SecSize = nSize;
	return nSize;
}

void CopySection(char * Src, char* dst, DWORD dwSize, PIMAGE_NT_HEADERS pNt,int n)
{
	PIMAGE_SECTION_HEADER pSecPack = IMAGE_FIRST_SECTION(pNt);
	memcpy(dst, Src, dwSize);
	pSecPack += n;
	pSecPack->PointerToRawData = (DWORD)dst - (DWORD)m_FileObj.GetlpBase();
	pSecPack->SizeOfRawData = dwSize;

	//拷贝其他段数据从第四个段开始
	pSecPack += 3;
	char* SecBufDst = (char*)(pSecPack->PointerToRawData + (DWORD)m_FileObj.GetlpBase());
	SecBufDst += dwSize;
	DWORD nCount = pNt->FileHeader.NumberOfSections;
	DWORD Size = 0;
	char* SecBufSrc = nullptr;
	for (DWORD i = 0; i < nCount - 3; i++)
	{
		SecBufSrc = (char*)(pSecPack->PointerToRawData + (DWORD)m_FileObj.GetlpBase());
		Size = pSecPack->SizeOfRawData;
		if (pSecPack->SizeOfRawData == 0)
		{
			MessageBox(0,L"区段数据为空！该版本可能为DeBug版本。",L"提示",0);
			break;
		}
		memcpy(SecBufDst, SecBufSrc, Size);
		dwSize += Size;
		SecBufDst = (char*)(dwSize + (DWORD)dst);
		pSecPack++;
	}
	
}