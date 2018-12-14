#pragma once
class Section
{
public:
	
	void ResaveHeader();      //����PE�ļ�ͷ��
	void GetFileData(TCHAR* PeFilePath);      //��ȡ�ļ����ѿռ�
	PIMAGE_SECTION_HEADER GetSection(char* SecName);     //��ȡָ�����ֵ�����ͷ
	PIMAGE_SECTION_HEADER GetLastSection();
	int Aligment(int Size, int aligmen);                //��������С
	PIMAGE_DOS_HEADER GetDosHeader();
	PIMAGE_NT_HEADERS &GetNtHeader();
	BOOL IsTLSFlag();
	void AddSection(DWORD Size, char* SecName);
	void SetlpBase(HMODULE lpBase);
	char* GetlpBase();
	BOOL WriteNewPE(int n);
	void PackPeSection(char* src,DWORD A2B);
private:
	PIMAGE_DOS_HEADER m_pDos;
	PIMAGE_NT_HEADERS m_pNt;
	PIMAGE_DATA_DIRECTORY m_pDir;
	PIMAGE_SECTION_HEADER m_pSec;
	PIMAGE_FILE_HEADER m_File;
	PIMAGE_OPTIONAL_HEADER m_Opt;

	DWORD m_PeFileSize;
	TCHAR* m_NewPath;
	BOOL m_TlsFlag;
	char* m_pFileBuff;
	char* m_pFileData;
	HANDLE m_hFile;
};

