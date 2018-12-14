#include "stdafx.h"
#include "Section.h"


void Section::GetFileData(TCHAR* PeFilePath)
{
	//1����PE�ļ�   ֻ�� ���� ��ͨ
	m_NewPath = PeFilePath;
	m_hFile = CreateFile(PeFilePath, GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(NULL, L"File Open Failed!", L"Error", 0);
	}
	//����ѿռ䣬memcpy
	m_PeFileSize = GetFileSize(m_hFile, 0);
	m_pFileBuff = new char[m_PeFileSize];
	memset(m_pFileBuff, 0, m_PeFileSize);
	//��ȡ�ļ���Ϣ
	DWORD dwRead = 0;
	ReadFile(m_hFile, m_pFileBuff, m_PeFileSize, &dwRead, 0);
	CloseHandle(m_hFile);

}

void Section::SetlpBase(HMODULE lpBase)
{
	m_pFileBuff = (char*)lpBase;
}

void Section::ResaveHeader()
{
	m_pFileData = m_pFileBuff;
	m_pDos = (PIMAGE_DOS_HEADER)m_pFileData;
	DWORD e_ifaNew = m_pDos->e_lfanew;
	m_pNt = (PIMAGE_NT_HEADERS)((DWORD)m_pFileData + e_ifaNew);
	m_File = &m_pNt->FileHeader;
	m_Opt = &m_pNt->OptionalHeader;
	m_pSec = IMAGE_FIRST_SECTION(m_pNt);
	IsTLSFlag();
}

PIMAGE_DOS_HEADER Section::GetDosHeader()
{
	return m_pDos;
}

PIMAGE_NT_HEADERS &Section::GetNtHeader()
{
	return m_pNt;
}

PIMAGE_SECTION_HEADER Section::GetSection(char* SecName)
{
	BOOL IsFlag = TRUE;
	DWORD dwSec = m_File->NumberOfSections;
	m_pSec = IMAGE_FIRST_SECTION(m_pNt);
	for (DWORD i = 0; i < dwSec; ++i)
	{
		for (int i = 0; i < 6; ++i)
		{
			if (SecName[i] != m_pSec->Name[i])
			{
				IsFlag = FALSE;
				break;
			}
			else
			{
				IsFlag = TRUE;
			}
		}
		if (IsFlag)
		{
			return m_pSec;
		}
		else
			m_pSec++;
	}

	return nullptr;
}

PIMAGE_SECTION_HEADER Section::GetLastSection()
{
	DWORD dwSec = m_File->NumberOfSections;
	m_pSec = IMAGE_FIRST_SECTION(m_pNt);
	return m_pSec + (dwSec - 1);
}

int Section::Aligment(int Size, int aligment)
{
	return Size % (aligment) == 0 ? (Size) : (Size / (aligment)+1)*aligment;
}

void Section::AddSection(DWORD Size, char* SecName)
{
	PIMAGE_SECTION_HEADER pLastSec = GetLastSection();// ��֮ǰ�����һ��
	//���������ε�RVA FOA
	DWORD NewSecRVA = pLastSec->VirtualAddress + Aligment(pLastSec->Misc.VirtualSize, m_Opt-> SectionAlignment);
	DWORD NewSecFoa = pLastSec->PointerToRawData + pLastSec->SizeOfRawData;
	//����������
	WORD &nSec = m_pNt->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pNewSec = GetLastSection();
	pNewSec++;
	nSec++;
	//�������������
	//д����������
	memcpy(pNewSec->Name, SecName, 8);
	pNewSec->Misc.VirtualSize = Size;
	pNewSec->VirtualAddress = NewSecRVA;
	pNewSec->PointerToRawData = NewSecFoa;
	DWORD nASize = Aligment(Size, 0x200);
	pNewSec->SizeOfRawData = nASize;
	pNewSec->Characteristics = 0xE00000E0;
	//�޸�SizeOfImage��С
	DWORD &SizeofImg = m_pNt->OptionalHeader.SizeOfImage;
	SizeofImg = NewSecRVA + pNewSec->SizeOfRawData;
	//������Ķѿռ�Ĵ�С
	DWORD NewFileSize = NewSecFoa + pNewSec->SizeOfRawData;

	char* pNewBuff = new char[NewFileSize];
	memset(pNewBuff, 0, NewFileSize);
	m_PeFileSize = NewSecFoa;
	memcpy(pNewBuff, m_pFileBuff, NewSecFoa);
	// �ͷžɵĻ�����
	delete m_pFileBuff;
	// ���µĻ������׵�ַ���µ��ļ���С��ֵ���β�(�޸�ʵ��)
	//m_PeFileSize = SizeofImg;
	m_pFileBuff = pNewBuff;
	ResaveHeader();
}

BOOL Section::WriteNewPE(int n)
{
	wcscat(m_NewPath,L"_u.exe");
	HANDLE hFile = CreateFile(m_NewPath,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DWORD dwWrite = 0;
	PIMAGE_SECTION_HEADER pLastSec1= GetLastSection();
	DWORD nSize = pLastSec1->PointerToRawData + pLastSec1->SizeOfRawData;
	
	WriteFile(hFile, m_pFileBuff, nSize, &dwWrite, NULL);
	// �ر��ļ����
	CloseHandle(hFile);
	return dwWrite == nSize;
}

char* Section::GetlpBase()
{
	return m_pFileData;
}

BOOL Section::IsTLSFlag()
{
	if (m_pNt->OptionalHeader.DataDirectory[9].VirtualAddress)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

void Section::PackPeSection(char* src,DWORD A2B)
{
	//��������λ����Ϣ

	int nSecNum = m_pNt->FileHeader.NumberOfSections;
	PIMAGE_SECTION_HEADER pSec = IMAGE_FIRST_SECTION(m_pNt);
	pSec++;
	for (int i = 1; i < nSecNum; ++i)
	{
		if (i < 3)
		{
			pSec->PointerToRawData = 0;
			pSec->SizeOfRawData = 0;
			pSec++;
		}
		else
		{
			pSec->PointerToRawData -= A2B;
			pSec++;
		}
	}

}