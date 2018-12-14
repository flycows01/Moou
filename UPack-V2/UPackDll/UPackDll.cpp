// UPackDll.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "UPackDll.h"
#include "lz4.h"
//����DLL��֧��CString������������#include <afxwin.h>�滻#include <Windows.h>

Section m_FileObj;
Section m_DllObj;

typedef int(*FunGetStub)(void);
typedef void(*FunSetStub)(TCHAR* strPath);
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN������stub����    														      //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
void loadStub(StubInfo* Stub)
{
	// 1. ͨ��LoadLibrary����stub.dll
	Stub->DllBase = LoadLibraryEx(L"stub.dll",
		NULL,
		DONT_RESOLVE_DLL_REFERENCES);
	m_DllObj.SetlpBase(Stub->DllBase);
	m_DllObj.ResaveHeader();
	// 2. ��ȡdll�ĵ�������
	Stub->pfnStart = (DWORD)GetProcAddress(Stub->DllBase,"StubEntryPoint");
	Stub->StubConf = (StubConf*)GetProcAddress(Stub->DllBase,
		"g_StubConf");
}

void fixStubRelocation();
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN�������ض�λ����    														      //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
void ModifyReloc()
{
	// 1. �ҵ��ض�λ��(stub.dll).
	DWORD relRva = m_DllObj.GetNtHeader()->OptionalHeader.DataDirectory[5].VirtualAddress;
	IMAGE_BASE_RELOCATION* pRel = (IMAGE_BASE_RELOCATION*)
		(relRva + (DWORD)m_DllObj.GetDosHeader());
	// 2. �����ض�λ��. 
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
//					FUN��TLS������											         			  //
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
//					FUN���ӿ�������������														      //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL UPack(TCHAR* strPath)
{
	//************************ ��ȡ�ӿǳ�������*************************************

	m_FileObj.GetFileData(strPath);
	m_FileObj.ResaveHeader();

	//******************** ���ܱ��ӿǳ���Ĵ����*************************************

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
	
	//***************************** �޸�Ŀ¼����Ϣ*************************************
	PIMAGE_NT_HEADERS pNt0 = m_FileObj.GetNtHeader();
	pNt0->OptionalHeader.DataDirectory[6].VirtualAddress = 0;
	pNt0->OptionalHeader.DataDirectory[6].Size = 0;
	pNt0->OptionalHeader.DataDirectory[10].VirtualAddress = 0;
	pNt0->OptionalHeader.DataDirectory[12].VirtualAddress = 0;
	pNt0->OptionalHeader.DataDirectory[2].VirtualAddress = 0;
	//***************************** ���ÿ�DLL����*************************************
	StubInfo stub = {0};
	loadStub(&stub);           // BUG�����ڿ�������֮ǰѹ����ص����ݱ���
	//***************************** ѹ��ԭ�������*************************************
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
		MessageBox(0, L"ѹ��ʧ�ܣ�", 0, 0);
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
	//************************ ���������δ����*************************************

	DWORD nAddSize = m_DllObj.GetSection(".text")->Misc.VirtualSize;
	m_FileObj.AddSection(nAddSize, ".Ultra");
	
	//�޸��ض�λ��
	fixStubRelocation();

	//************************ �����������δ����*************************************
	PIMAGE_SECTION_HEADER pLastSec = m_FileObj.GetLastSection();
	DWORD RVA = pLastSec->PointerToRawData;
	char* lpBase = (char*)(RVA + (DWORD)m_FileObj.GetlpBase());

	char* RVA2 = (char*)(m_DllObj.GetSection(".text")->VirtualAddress + (DWORD)stub.DllBase);
	int nSize = m_DllObj.GetSection(".text")->Misc.VirtualSize;
	memcpy(lpBase,
		RVA2,
		nSize);

	//************************ �޸�OEP�����*************************************
	PIMAGE_SECTION_HEADER ptextSec = m_DllObj.GetSection(".text");
	PIMAGE_NT_HEADERS &pNt = m_FileObj.GetNtHeader();

	pNt->OptionalHeader.AddressOfEntryPoint
		= stub.pfnStart - (DWORD)stub.DllBase -
		ptextSec->VirtualAddress +            //��ȥ�����dll��.text�ε�ƫ��
		pLastSec->VirtualAddress;           //�������ļ����¼Ӷε���ʼ��ַ  ��Ϊ������ַ������¼Ӷε�ƫ��

	//************************ ���������ַ�����*************************************
	//pNt->OptionalHeader.DllCharacteristics &= (~0x40);

	//����reloc����
	nAddSize = m_DllObj.GetSection(".reloc")->Misc.VirtualSize;
	m_FileObj.AddSection(nAddSize, ".Ultrc");

	ModifyReloc();                        //�޸�DLL�ض�λ��

	//����������������
	pLastSec = m_FileObj.GetSection(".Ultrc");
	RVA = pLastSec->PointerToRawData;
	lpBase = (char*)(RVA + (DWORD)m_FileObj.GetlpBase());

	RVA2 = (char*)(m_DllObj.GetSection(".reloc")->VirtualAddress + (DWORD)stub.DllBase);
	nSize = m_DllObj.GetSection(".reloc")->Misc.VirtualSize;
	memcpy(lpBase,
		RVA2,
		nSize);

	//���ض�λ����Ϣ�޸���Ŀ¼��

	pNt->OptionalHeader.DataDirectory[5].VirtualAddress = pLastSec->VirtualAddress;
	pNt->OptionalHeader.DataDirectory[5].Size = nSize;

	//***************************** ѹ��ԭ�������*************************************
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
	//	MessageBox(0, L"ѹ��ʧ�ܣ�", 0, 0);
	//}

	//��������λ����Ϣ
	WriteSize = m_FileObj.Aligment(WriteSize, pNt->OptionalHeader.FileAlignment);
	char* DstFoa = (char*)(ThrSec.Sec1Fva + (DWORD)m_FileObj.GetlpBase());
	CopySection(LzBuff, DstFoa, WriteSize, pNt, 0);
	DWORD A2B = srcSize - WriteSize;

	m_FileObj.PackPeSection(DstFoa, A2B);      //�޸Ĵӵ��ĸ���ʼ�����ε�PointerToRawData 
	delete []LzBuff;

	//************************ �������ļ������*************************************
	m_FileObj.WriteNewPE(1);
	
	//��������
	FunGetStub pfnGetStub = (FunGetStub)GetProcAddress(stub.DllBase, "GetStub");
	if (pfnGetStub())
	{
		MessageBox(NULL,L"��ĳ����Ѿ�ȫ����װ^-^",L"Ultracis",0);
	}
	return TRUE;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                    //  
//					FUN���Ǵ��������ض�λ����														  //
//																									  //
////////////////////////////////////////////////////////////////////////////////////////////////////////
void fixStubRelocation()
{
	// 1. �ҵ��ض�λ��(stub.dll).
	DWORD relRva = m_DllObj.GetNtHeader()->OptionalHeader.DataDirectory[5].VirtualAddress;
	IMAGE_BASE_RELOCATION* pRel = (IMAGE_BASE_RELOCATION*)
		(relRva + (DWORD)m_DllObj.GetDosHeader());
	// 2. �����ض�λ��. 
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
			// 3. ���ض�λ���޸�:
			DWORD* pFixAddr = (DWORD*)
				(pRel->VirtualAddress
				+ pTypeOfs[i].offset
				+ (DWORD)m_DllObj.GetDosHeader());

			DWORD old;
			VirtualProtect(pFixAddr, 4, PAGE_READWRITE, &old);								// ȥ��dll��ǰ���ػ�ַ
			*pFixAddr -= (DWORD)m_DllObj.GetlpBase();									// ȥ��Ĭ�ϵĶ���rva
			*pFixAddr -= m_DllObj.GetSection(".text")->VirtualAddress;						// ����Ŀ���ļ��ļ��ػ�ַ
			*pFixAddr += (DWORD)m_FileObj.GetNtHeader()->OptionalHeader.ImageBase;          // ���������εĶ���rva
			DWORD LASTRVA = m_FileObj.GetLastSection()->VirtualAddress;
			*pFixAddr += LASTRVA;
			VirtualProtect(pFixAddr, 4, old, &old);
		}

		// �л�����һ��
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

	//�������������ݴӵ��ĸ��ο�ʼ
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
			MessageBox(0,L"��������Ϊ�գ��ð汾����ΪDeBug�汾��",L"��ʾ",0);
			break;
		}
		memcpy(SecBufDst, SecBufSrc, Size);
		dwSize += Size;
		SecBufDst = (char*)(dwSize + (DWORD)dst);
		pSecPack++;
	}
	
}