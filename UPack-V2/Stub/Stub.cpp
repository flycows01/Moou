// Stub.cpp : ���� DLL Ӧ�ó���ĵ���������
//

#include "stdafx.h"
#include "Stub.h"
#include "lz4.h"
// ��Stub��Ŀ�е�Stub.cpp�У����һ�´��룬���ƴ���Ŀ�ڱ���ʱ������ѡ����������ɵ�dll����.text��.data��rdata���κϲ���
#pragma comment(linker, "/merge:.data=.text")        // ��.data�ϲ���.text
#pragma comment(linker, "/merge:.rdata=.text")       // ��.rdata�ϲ���.text
#pragma comment(linker, "/section:.text,RWE")        // ��.text�ε���������Ϊ�ɶ�����д����ִ��

//#pragma comment(linker, "/entry:\"StubEntryPoint\"") // ָ��������ں���ΪStubEntryPoint()

//���ǽ����ɵ�Debug�汾��stub.dll���ص�LordPE�У��������α��еĵ�һ������Ϊ.textbss������������Ļ��ͻ�Ӱ�����ǵĲ���
//Ϊ�˴ﵽ��һ������Ϊ.text��������Ҫ����Stub����ΪRelease�汾��(ѡ�����ù�����)
//Ϊ�˺�����Է��㣬����Ҫ��Stub��Ŀ�����Ŀ¼����ΪDebug�������ͻ���Stub.dllΪRelease�汾�����������DebugĿ¼�С�

//************************************API����ָ��*******************************************

typedef void*(WINAPI *FnGetProcAddress)(HMODULE, const char*);//getprocaddress
typedef	HMODULE(WINAPI *FnLoadLibraryA)(const char *);//loadlibrary
typedef BOOL(WINAPI *FnVirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD); //virtualprotect
typedef int (WINAPI *LPMESSAGEBOX)(HWND, LPCTSTR, LPCTSTR, UINT); //MessageBoxW
typedef HMODULE(WINAPI *GETModuleHandle)(LPCTSTR);
typedef BOOL(WINAPI* SHOWWINDOW)(HWND, int);
typedef BOOL(WINAPI* GteMessage)(LPMSG, HWND, UINT, UINT);
typedef LRESULT(WINAPI* DISpatchMessage)(const MSG *);
typedef ATOM(WINAPI* REGisterClass)(const WNDCLASS *);
typedef HWND(WINAPI *CREateWindowEx)(DWORD, LPCTSTR, LPCTSTR, DWORD,
	int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef VOID(WINAPI* POSTQuitMessage)(int);
typedef LRESULT(WINAPI* DEFWindowProc)(HWND, UINT, WPARAM, LPARAM);
typedef BOOL(*UPDateWindow)(HWND);
typedef int (WINAPI* GETWindowText)(HWND, LPTSTR, int);
typedef int (WINAPI* GETWindowTextLength)(HWND);
typedef HWND(WINAPI* GETDlgItem)(HWND, int);
typedef BOOL(WINAPI* SETWindowText)(HWND, LPCTSTR);
typedef BOOL(WINAPI* TRanslateMessage)(const MSG *);
typedef LPVOID(WINAPI *MYVIRTUALALLOC)(LPVOID, SIZE_T, DWORD, DWORD);
typedef BOOL(WINAPI *MYVIRTUALFREE)(LPVOID, SIZE_T, DWORD);
typedef BOOL(WINAPI *Fnmemcpy)(void*, void const*, size_t);
typedef BOOL(WINAPI *Fnmalloc)(size_t);

wchar_t g_UserInput[20] = { 0 };
wchar_t g_Psw[20] = L"Ultracis";
FnVirtualProtect    MyVirtualProtect;
FnLoadLibraryA      MyLoadLibraryA;
FnGetProcAddress    MyGetProcAddress;
HMODULE             hUser32 = nullptr;
GETModuleHandle     MyGetModuleHandle = nullptr;
LPMESSAGEBOX        MyMessageBox = nullptr;
CREateWindowEx      MyCreateWindowEx = nullptr;
POSTQuitMessage     MyPostQuitMessage = nullptr;
DEFWindowProc       MyDefWindowProc = nullptr;
GteMessage          MyGetMessage = nullptr;
REGisterClass       MyRegisterClass = nullptr;
SHOWWINDOW          MyShowWindow = nullptr;
UPDateWindow        MyUpdateWindow = nullptr;
DISpatchMessage     MyDispatchMessage = nullptr;
GETWindowText       MyGetWindowText = nullptr;
GETWindowTextLength MyGetWindowTextLength = nullptr;
GETDlgItem          MyGetDlgItem = nullptr;
SETWindowText       MySetWindowText = nullptr;
TRanslateMessage    MyTranslateMessage = nullptr;
MYVIRTUALALLOC      MyVirtualAlloc = nullptr;
MYVIRTUALFREE       MyVirtualFree = nullptr;
Fnmemcpy            MyMemcpy = nullptr;
Fnmalloc            MyMalloc = nullptr;
//************************************ȫ�ֱ���*******************************************
HMODULE g_hKernel32;
HMODULE g_hUser32;
HMODULE g_hInstance;
DWORD g_PEBase;
StubConf g_StubConf = { 0 };
TCHAR* szStub = nullptr;
int nStub = 1;

STUP_API int GetStub()
{
	return nStub;
}

STUP_API void SetStub(TCHAR* strPath)
{
	szStub = strPath;
}

//��ʼͨ��֮ǰ���������Ǿ��Ѿ����ú���Ŀ�ˣ�Ϊ�������ǵ���Ŀ���ø���Щ�������������ɵ��ļ�
//ֻ��MFC�������ɵ�PackBase.exe�ͼӿǹ������ɵ�Pack_Dll.dll�����ļ�����Stub�������ɵ�Stub.dll����Դ����ʽ����Pack_Dll��Ŀ�С�
//Ϊ�ˣ��������Ƚ�Stub���̱���һ�£����ͻ���Debug������һ����ΪStub.dll���Ŀơ�
//Ȼ������ĿPack_dll��Ŀ�ϣ��Ҽ��������ӡ��еġ���Դ�����������Դ�Ի����У���������롱�����뵽����Stub.dll��Ŀ¼�У�
//���ļ����˶Ի���ѡ�������ļ�(*.*)����ѡ��Stub.dll��ȷ����


// ����dll��ʽ�ļ�ͬexe��ʽһ������ִ�������Լ�����ǰ��ִ�кܶ���������룬����Щ�������ǲ��ɿأ�
//ͬ��Ϊ�˳���Ľ�׳�ԣ�������Ҫ����������ȥ������
//Ϊ�ˣ�������Stub.cpp�ļ����Զ���һ����ں������ɣ���������:

//************************************��������*******************************************
void GetApis();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateWindowFun();
//***********************************************************************
//*   FUN���ص��������ú���                                             * 
//*                                                                     *
//***********************************************************************

typedef void (NTAPI *TLS_Fun)(PVOID DllHandle, DWORD Reason, PVOID Reserved);
void TlsCallBack()
{
	DWORD TlsVa = g_PEBase + g_StubConf.tlsRva;
	PIMAGE_TLS_DIRECTORY pTls = (PIMAGE_TLS_DIRECTORY)TlsVa;
	PDWORD pFunAddr = (PDWORD)pTls->AddressOfCallBacks;

	while (*pFunAddr)
	{
		TLS_Fun  pFun = (TLS_Fun)pFunAddr;
		pFun((PVOID)g_PEBase,1,NULL );
		pFunAddr++;
	}

}

//***********************************************************************
//*   FUN���޸��ض�λ����                                             * 
//*                                                                     *
//***********************************************************************

/*extern "C" __declspec(dllexport) */
void ModifyReloc()
{
	//g_PEBase = (DWORD)MyGetModuleHandle(NULL);

	DWORD RelRVA = g_PEBase + g_StubConf.relocScnRva;
	PIMAGE_BASE_RELOCATION pRel = (PIMAGE_BASE_RELOCATION)RelRVA;
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
				+ g_PEBase);

			DWORD old;
			MyVirtualProtect(pFixAddr, 4, PAGE_READWRITE, &old);
			DWORD LASTRVA = g_PEBase - 0x400000;
			*pFixAddr += LASTRVA;
			MyVirtualProtect(pFixAddr, 4, old, &old);
		}
		// �л�����һ��
		pRel = (IMAGE_BASE_RELOCATION*)((DWORD)pRel + pRel->SizeOfBlock);
	}
}

//***********************************************************************
//*   FUN���޸�IAT����                                                * 
//*                                                                     *
//***********************************************************************

void RecoverIAT()
{
	DWORD ImpVA = g_PEBase + g_StubConf.ImpRva;
	PIMAGE_IMPORT_DESCRIPTOR pImp = (PIMAGE_IMPORT_DESCRIPTOR)ImpVA;

	while (pImp->Name)
	{
		//��ȡģ����
		DWORD dwDllNameRVA = pImp->Name;
		char * dwDllName = (char*)(g_PEBase + dwDllNameRVA);          //��ȡģ������
		HMODULE hMod = MyLoadLibraryA(dwDllName);                    //����ģ��
		PIMAGE_THUNK_DATA pThunkINT = (PIMAGE_THUNK_DATA)(pImp->OriginalFirstThunk + g_PEBase);
		PIMAGE_THUNK_DATA pThunkIAT = (PIMAGE_THUNK_DATA)(pImp->FirstThunk + g_PEBase);
		while (pThunkIAT->u1.AddressOfData)
		{
			DWORD Old = 0;
			MyVirtualProtect(pThunkIAT, 4, PAGE_READWRITE, &Old);
			if (!IMAGE_SNAP_BY_ORDINAL32(pThunkIAT->u1.Ordinal))
			{
				PIMAGE_IMPORT_BY_NAME pINT = (PIMAGE_IMPORT_BY_NAME)(g_PEBase + pThunkINT->u1.AddressOfData);
				DWORD FunAddr = (DWORD)MyGetProcAddress(hMod, pINT->Name);
				//����IAT����
				/*FunAddr = FunAddr << 4;
				FunAddr += 0x10;*/
				//
				pThunkIAT->u1.Function = FunAddr;
			}
			else
			{
				DWORD dwFunOrdinal = (pThunkIAT->u1.Ordinal) & 0x7FFFFFFF;
				DWORD dwFunAddr = (DWORD)MyGetProcAddress(hMod, (char*)dwFunOrdinal);
				pThunkIAT->u1.Function = (DWORD)dwFunAddr;
			}
			MyVirtualProtect(pThunkIAT, 4, Old, &Old);
			pThunkIAT++;
			pThunkINT++;
		}

		//ת������һ��
		pImp++;
	}
}


void UnPack()
{
	PIMAGE_DOS_HEADER pDosUP = (PIMAGE_DOS_HEADER)g_PEBase;
	PIMAGE_NT_HEADERS pNtUP = (PIMAGE_NT_HEADERS)(pDosUP->e_lfanew + g_PEBase);
	PIMAGE_SECTION_HEADER pSecUP = IMAGE_FIRST_SECTION(pNtUP);
	char* UnPackDst = (char*)(pSecUP->VirtualAddress + g_PEBase);
	DWORD old = 0;                                                                  //BUG �����޸��ڴ�����
	MyVirtualProtect(UnPackDst, g_StubConf.ThrSecInf.SecSize, PAGE_READWRITE, &old);
	//��ѹ�ļ���text����
	char* TempBuf = (char*)MyMalloc(sizeof(char)*g_StubConf.ThrSecInf.SecSize);
	LZ4_decompress_safe(UnPackDst, TempBuf, g_StubConf.ThrSecInf.ComSize, g_StubConf.ThrSecInf.SecSize);
	//�����������
	char* SecDst = nullptr;
	char* SecSrc = nullptr;
	DWORD CodeSize = 0;
	DWORD CopySize = 0;
	for (int i = 0; i < 3;++i)
	{
		SecDst = (char*)(pSecUP->VirtualAddress + g_PEBase);
		SecSrc = TempBuf + CodeSize;
		if (i == 0)
			CopySize = g_StubConf.ThrSecInf.Sec1Size;
		else if (i == 1)
			CopySize = g_StubConf.ThrSecInf.Sec2Size;
		else if (i == 2)
			CopySize = g_StubConf.ThrSecInf.Sec3Size;
		MyMemcpy(SecDst, SecSrc, CopySize);      //�ָ���һ������
		CodeSize = CopySize;
		//pSecUP->SizeOfRawData = CopySize;
		pSecUP++;
	}
	MyVirtualProtect(UnPackDst, g_StubConf.ThrSecInf.SecSize, old, &old);
}

//***********************************************************************
//*   FUN���ӿǳ�����ܡ���ѹ������                                     * 
//*                                                                     *
//***********************************************************************
void EnCode()
{
	unsigned char* pText = (unsigned char*)g_StubConf.textScnRva + g_PEBase;
	//����text��
	DWORD old = 0;
	MyVirtualProtect(pText,	g_StubConf.textScnSize,	PAGE_READWRITE,	&old);
	for (DWORD i = 0; i < g_StubConf.textScnSize; ++i)
	{
		pText[i] ^= g_StubConf.key;
	}
	MyVirtualProtect(pText,	g_StubConf.textScnSize,	old,	&old);

	//����rdata��
	pText = (unsigned char*)g_StubConf.rdataScnRva + g_PEBase;
	MyVirtualProtect(pText, g_StubConf.rdataScnSize, PAGE_READWRITE, &old);
	for (DWORD i = 0; i < g_StubConf.rdataScnSize; ++i)
	{
		pText[i] ^= g_StubConf.key;
	}

	MyVirtualProtect(pText, g_StubConf.rdataScnSize, old, &old);

}

void start()
{
	//�����ŶԱ��ӿ��ļ��Ĳ���
	GetApis();
}
//***********************************************************************
//*   FUN���ǳ�����ں���                                               * 
//*                                                                     *
//***********************************************************************
void StubEntryPoint()
{
	_asm sub esp, 0x50;        // ̧��ջ������߼�����
	_asm{
		call      label_1;
		inc       ecx;
		jmp       label_2;
		inc       ecx;
	label_1 : 
		pop       eax;
		jmp       label_3;
		inc       ecx;
	label_3 : 
		inc       eax;
		jmp       label_4;
		inc       ecx;
	label_4 :
		jmp       eax;
		inc       ecx;
	label_2 :
	}
	start();                    // ִ�пǵ����岿��
	_asm add esp, 0x50;        // ƽ���ջ

	/*_asm mov eax, g_StubConf.srcOep;
	_asm add eax, 0x400000;
	_asm jmp eax;*/
}
//***********************************************************************
//*   FUN����ȡAPI����                                                  * 
//*                                                                     *
//***********************************************************************
void GetApis()
{
	_asm
	{
		//��ȡkernel32.dll�ļ��ػ�ַ;
		pushad;                                               
		//1. �ҵ�PEB���׵�ַ;
		mov esi, fs:[0x30];                                   //eax = > peb�׵�ַ;
		//2. �õ�PEB.Ldr��ֵ;
		mov esi, [esi + 0ch];                                 //eax = > PEB.Ldr��ֵ;
		mov esi, [esi + 0ch];                                 //eax = > PEB.Ldr��ֵ;
		//3. �õ�_PEB_LDR_DATA.InLoadOrderMoudleList.Flink��ֵ, ʵ�ʵõ��ľ�����ģ��ڵ���׵�ַ;
		mov esi, [esi];                                       //eax = > _PEB_LDR_DATA.InLoadOrderMoudleList.Flink(NTDLL);
		//4. �ٻ�ȡ��һ��;
		mov esi, [esi];                                       //_LDR_DATA_TABLE_ENTRY.InLoadOrderMoudleList.Flink(kernel32), ;
		mov esi, [esi + 018h];                                //_LDR_DATA_TABLE_ENTRY.DllBase;
		mov edx, esi;                                         //dllBase

		//����������;
		//1. dosͷ-- > ntͷ-- > ��չͷ-- > ����Ŀ¼��;
		mov eax, [esi + 03ch];                                //eax = > ƫ�Ƶ�NTͷ;
		lea eax, dword ptr[edx + eax];                        //ebx = > NTͷ���׵�ַ;
		mov ebx, [eax + 78h];                                 //ebx = > mulubiao�׵�ַ;
		lea ebx, dword ptr[edx + ebx];                        //������VA  
		//2. �õ��������RVA;
		mov eax, [ebx + 1ch];               //��ȡFunAddr��ַ��RVA
		lea eax, [edx + eax];                //��õ�ַ���VA
		mov dword ptr[ebp - 4], eax;         //�����ַ��VA
		mov eax, [ebx + 20h];               //��ȡ���Ʊ�RVA
		lea eax, [edx + eax];
		mov dword ptr[ebp - 8h], eax;         //�������Ʊ�VA
		mov eax, [ebx + 24h];
		lea eax, [edx + eax];
		mov dword ptr[ebp - 0ch], eax;       //������ű�VA

		//3. �������Ʊ��ҵ�GetProcAddress;
		xor esi, esi;
		mov esi, [ebx + 18h];                //���Ʊ����
		xor ecx, ecx;
		//3.1 �������Ʊ�;
	_WHILE:
		mov eax, dword ptr[ebp - 8];
		mov esi, [eax + ecx * 4];                  //esi = > ���Ƶ�rva;
		lea esi, [esi + edx];                      //esi = > �����׵�ַ;
		cmp dword ptr[esi], 050746547h;            
		jne _LOOP;
		cmp dword ptr[esi + 4], 041636f72h;
		jne _LOOP;
		cmp dword ptr[esi + 8], 065726464h;
		jne _LOOP;
		cmp word  ptr[esi + 0ch], 07373h;
		jne _LOOP;
		// �ҵ�֮��;
		mov eax, dword ptr[ebp - 0ch];                 //edi = > ���Ƶ���ű��va;
		mov ax, [eax + ecx * 2];                       //��ű���2�ֽڵ�Ԫ��, ����� * 2;
		//eax�������GetProcAddress���ڵ�ַ���е��±�;
		and eax, 0FFFFh;                               //�õ���ַ���׵�ַ;
		mov ecx, eax;
		mov eax, dword ptr[ebp - 4];                         //eax = > ��ַ���va;
		mov edi, [eax + ecx * 4];                            //edi = > GetProcAddress��rva;
		lea edi, [edi + edx];                                // edx = > GetProcAddress��va;
		mov MyGetProcAddress, edi;
		jmp _ENDWHILE;
	_LOOP:;
		inc ecx; // ++index;
		jmp _WHILE;
	_ENDWHILE:
		mov g_hKernel32, edx;
		popad;
	}

	MyLoadLibraryA = (FnLoadLibraryA)MyGetProcAddress(g_hKernel32, "LoadLibraryA");
	MyVirtualProtect = (FnVirtualProtect)MyGetProcAddress(g_hKernel32, "VirtualProtect");

	// ���Ե���API
	g_hUser32 = MyLoadLibraryA("user32.dll");                     //User32ģ�����

	HMODULE msvcrt = MyLoadLibraryA("msvcrt.dll");                //msvcrtģ�����
	MyMemcpy = (Fnmemcpy)
		MyGetProcAddress(msvcrt, "memcpy");

	HMODULE MSVCR120D = MyLoadLibraryA("MSVCR120D.dll");          //MSVCR120Dģ�����
	MyMalloc = (Fnmalloc)
		MyGetProcAddress(MSVCR120D, "malloc");

	// MFC�������ʱ������
	MyGetModuleHandle = (GETModuleHandle)MyGetProcAddress(g_hKernel32, "GetModuleHandleW");
	MyVirtualAlloc = (MYVIRTUALALLOC)MyGetProcAddress(g_hKernel32, "VirtualAlloc");
	MyVirtualFree = (MYVIRTUALFREE)MyGetProcAddress(g_hKernel32, "VirtualFree");
	MyMessageBox = (LPMESSAGEBOX)MyGetProcAddress(g_hUser32, "MessageBoxW");
	MyCreateWindowEx = (CREateWindowEx)MyGetProcAddress(g_hUser32, "CreateWindowExW");
	MyPostQuitMessage = (POSTQuitMessage)MyGetProcAddress(g_hUser32, "PostQuitMessage");
	MyDefWindowProc = (DEFWindowProc)MyGetProcAddress(g_hUser32, "DefWindowProcW");
	MyGetMessage = (GteMessage)MyGetProcAddress(g_hUser32, "GetMessageW");
	MyRegisterClass = (REGisterClass)MyGetProcAddress(g_hUser32, "RegisterClassW");
	MyShowWindow = (SHOWWINDOW)MyGetProcAddress(g_hUser32, "ShowWindow");
	MyUpdateWindow = (UPDateWindow)MyGetProcAddress(g_hUser32, "UpdateWindow");
	MyDispatchMessage = (DISpatchMessage)MyGetProcAddress(g_hUser32, "DispatchMessageW");
	MyGetWindowText = (GETWindowText)MyGetProcAddress(g_hUser32, "GetWindowTextW");
	MyGetWindowTextLength = (GETWindowTextLength)MyGetProcAddress(g_hUser32, "GetWindowTextLengthW");
	MyGetDlgItem = (GETDlgItem)MyGetProcAddress(g_hUser32, "GetDlgItem");
	MySetWindowText = (SETWindowText)MyGetProcAddress(g_hUser32, "SetWindowTextW");
	MyTranslateMessage = (TRanslateMessage)MyGetProcAddress(g_hUser32, "TranslateMessage");
		// ���Ե���API
	MyGetProcAddress(g_hUser32, "MessageBoxA");

	// MFC�������ʱ������
	MyMessageBox(0, L"��Һ�����һ����", L"��ʾ", 0);

	g_PEBase = (DWORD)MyGetModuleHandle(NULL);
	g_hInstance = MyGetModuleHandle(NULL);
	CreateWindowFun();

}
//***********************************************************************
//*   FUN�����ڴ�������                                                 * 
//*                                                                     *
//***********************************************************************

 void CreateWindowFun()
{
	WNDCLASS wcs = {};
	wcs.lpszClassName = L"15PB";
	wcs.lpfnWndProc = WindowProc;
	wcs.hbrBackground = (HBRUSH)(COLOR_CAPTIONTEXT + 2);
	/////////////////////////////////////////////////////////////////////////////////////////
	MyRegisterClass(&wcs);
	HWND hWnd = MyCreateWindowEx(0L, L"15PB", L"Ultracis", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
		500, 200, 300, 200,
		NULL, NULL, NULL, NULL);

	MyShowWindow(hWnd, SW_SHOW);
	MyUpdateWindow(hWnd);

	MSG msg = { 0 };
	while (MyGetMessage(&msg, 0, 0, 0))
	{
		MyTranslateMessage(&msg);
		MyDispatchMessage(&msg);
	}
}
 int MyStrCmp()
 {
	 for (int i = 0; i < 8; i++)
	 {
		 if (g_Psw[i] != g_UserInput[i])
			 return 0;
	 }
	 return 1;
 }
//***********************************************************************
//*   FUN�����ڻص�����                                                 * 
//*                                                                     *
//***********************************************************************
 LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE: 
	{
		/////////////////////////////////////////////////////////////////////////////////////
		//����Ok��ť
		MyCreateWindowEx(0L, L"BUTTON", L"Ok", WS_CHILD | WS_VISIBLE,
			50, 100,50, 20,
			hwnd,(HMENU)0x1000,
			MyGetModuleHandle(NULL), NULL);
		//�����˳���ť
		MyCreateWindowEx(0L, L"BUTTON", L"No", WS_CHILD | WS_VISIBLE,
			200, 100,50, 20,
			hwnd,(HMENU)0x1001,
			MyGetModuleHandle(NULL), NULL);
		//�����༭��
		DWORD dwStyle = ES_LEFT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE;
		DWORD dwExStyle = WS_EX_CLIENTEDGE | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
		HWND hWnd = MyCreateWindowEx(
			dwExStyle, 	L"Edit", NULL, dwStyle, //dwStyle ������ʽ
			50, 50, 200, 20, 
			hwnd, //hWndParent �����ھ��
			(HMENU)0x1002, 
			MyGetModuleHandle(0), NULL);

		HWND hWndEdit = MyGetDlgItem(hwnd, 0x1002);
		wchar_t wStr3[20] = L"";
		MySetWindowText(hWndEdit, wStr3);
		/////////////////////////////////////////////////////////////////////////////////////
		return 0;
	}
	case WM_COMMAND: 
	{
		/////////////////////////////////////////////////////////////////////////////////////
		WORD wId = LOWORD(wParam);
		WORD wCode = HIWORD(wParam);
		HANDLE hChild = (HANDLE)lParam;
		if (wId == 0x1001 && wCode == BN_CLICKED)
		{
			MyPostQuitMessage(0);
		}
		if (wId == 0x1000 && wCode == BN_CLICKED)
		{
			HWND hWndEdit = MyGetDlgItem(hwnd, 0x1002);
			int cTxtLen = MyGetWindowTextLength(hWndEdit);
			MyGetWindowText(hWndEdit, g_UserInput, cTxtLen+1);

			if (MyStrCmp()) 
			{
				MyShowWindow(hwnd, SW_HIDE);
				wchar_t wStr[20] = L"�����ˣ�����";
				wchar_t wStr2[20] = L"Ultracis";
				MyMessageBox(NULL, wStr, wStr2, NULL);
				//��ѹ���ļ�
				//////////////////////////////////////////////////////////////////////////////
				UnPack();
				////////////////////////////////////////////////////////////////////////////
				EnCode();
				ModifyReloc();
				RecoverIAT();
				if (g_StubConf.tlsRva)
				{
					TlsCallBack();
				}
				//��ת��OEP
				_asm mov eax, g_StubConf.srcOep;
				_asm add eax, g_PEBase;
				_asm jmp eax;
			}
			else 
			{
				MyPostQuitMessage(0);
			}
			return 1;
		}
		break;
		/////////////////////////////////////////////////////////////////////////////////////
	}
	case WM_CLOSE:
	{
		MyPostQuitMessage(0);
		break;
	}

	}
	// ����Ĭ�ϵĴ��ڴ������
	return MyDefWindowProc(hwnd, uMsg, wParam, lParam);
}