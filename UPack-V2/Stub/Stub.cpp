// Stub.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "Stub.h"
#include "lz4.h"
// 在Stub项目中的Stub.cpp中，添加一下代码，控制此项目在编译时的连接选项。让我们生成的dll程序.text、.data与rdata区段合并。
#pragma comment(linker, "/merge:.data=.text")        // 将.data合并到.text
#pragma comment(linker, "/merge:.rdata=.text")       // 将.rdata合并到.text
#pragma comment(linker, "/section:.text,RWE")        // 将.text段的属性设置为可读、可写、可执行

//#pragma comment(linker, "/entry:\"StubEntryPoint\"") // 指定程序入口函数为StubEntryPoint()

//我们将生成的Debug版本的stub.dll加载到LordPE中，发现区段表中的第一个区段为.textbss，如果是这样的话就会影响我们的操作
//为了达到第一个区段为.text，我们需要将想Stub设置为Release版本。(选项配置管理器)
//为了后面调试方便，我们要将Stub项目的输出目录设置为Debug，这样就会让Stub.dll为Release版本，但是输出到Debug目录中。

//************************************API函数指针*******************************************

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
//************************************全局变量*******************************************
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

//起始通过之前的配置我们就已经配置好项目了，为了让我们的项目配置更好些，想让最终生成的文件
//只有MFC工程生成的PackBase.exe和加壳工程生成的Pack_Dll.dll两个文件，而Stub工程生成的Stub.dll以资源的形式存在Pack_Dll项目中。
//为此，我们首先将Stub工程编译一下，它就会在Debug中生成一个名为Stub.dll的文科。
//然后，在项目Pack_dll项目上，右键点击“添加”中的“资源”，在添加资源对话框中，点击“导入”，进入到生成Stub.dll的目录中，
//在文件过滤对话总选择“所有文件(*.*)”，选中Stub.dll并确定。


// 由于dll格式文件同exe格式一样，在执行我们自己代码前会执行很多的引导代码，而这些代码我们不可控，
//同样为了程序的健壮性，我们需要将引导代码去除掉。
//为此，我们在Stub.cpp文件中自定义一个入口函数即可，代码如下:

//************************************函数申明*******************************************
void GetApis();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CreateWindowFun();
//***********************************************************************
//*   FUN：回调函数调用函数                                             * 
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
//*   FUN：修复重定位表函数                                             * 
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
			// 3. 将重定位项修改:
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
		// 切换到下一个
		pRel = (IMAGE_BASE_RELOCATION*)((DWORD)pRel + pRel->SizeOfBlock);
	}
}

//***********************************************************************
//*   FUN：修复IAT表函数                                                * 
//*                                                                     *
//***********************************************************************

void RecoverIAT()
{
	DWORD ImpVA = g_PEBase + g_StubConf.ImpRva;
	PIMAGE_IMPORT_DESCRIPTOR pImp = (PIMAGE_IMPORT_DESCRIPTOR)ImpVA;

	while (pImp->Name)
	{
		//获取模块名
		DWORD dwDllNameRVA = pImp->Name;
		char * dwDllName = (char*)(g_PEBase + dwDllNameRVA);          //获取模块名称
		HMODULE hMod = MyLoadLibraryA(dwDllName);                    //加载模块
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
				//加密IAT函数
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

		//转换至下一个
		pImp++;
	}
}


void UnPack()
{
	PIMAGE_DOS_HEADER pDosUP = (PIMAGE_DOS_HEADER)g_PEBase;
	PIMAGE_NT_HEADERS pNtUP = (PIMAGE_NT_HEADERS)(pDosUP->e_lfanew + g_PEBase);
	PIMAGE_SECTION_HEADER pSecUP = IMAGE_FIRST_SECTION(pNtUP);
	char* UnPackDst = (char*)(pSecUP->VirtualAddress + g_PEBase);
	DWORD old = 0;                                                                  //BUG 忘记修改内存属性
	MyVirtualProtect(UnPackDst, g_StubConf.ThrSecInf.SecSize, PAGE_READWRITE, &old);
	//解压文件至text区段
	char* TempBuf = (char*)MyMalloc(sizeof(char)*g_StubConf.ThrSecInf.SecSize);
	LZ4_decompress_safe(UnPackDst, TempBuf, g_StubConf.ThrSecInf.ComSize, g_StubConf.ThrSecInf.SecSize);
	//拉伸各个区段
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
		MyMemcpy(SecDst, SecSrc, CopySize);      //恢复第一个区段
		CodeSize = CopySize;
		//pSecUP->SizeOfRawData = CopySize;
		pSecUP++;
	}
	MyVirtualProtect(UnPackDst, g_StubConf.ThrSecInf.SecSize, old, &old);
}

//***********************************************************************
//*   FUN：加壳程序解密、解压缩函数                                     * 
//*                                                                     *
//***********************************************************************
void EnCode()
{
	unsigned char* pText = (unsigned char*)g_StubConf.textScnRva + g_PEBase;
	//解密text段
	DWORD old = 0;
	MyVirtualProtect(pText,	g_StubConf.textScnSize,	PAGE_READWRITE,	&old);
	for (DWORD i = 0; i < g_StubConf.textScnSize; ++i)
	{
		pText[i] ^= g_StubConf.key;
	}
	MyVirtualProtect(pText,	g_StubConf.textScnSize,	old,	&old);

	//解密rdata段
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
	//这里存放对被加壳文件的操作
	GetApis();
}
//***********************************************************************
//*   FUN：壳程序入口函数                                               * 
//*                                                                     *
//***********************************************************************
void StubEntryPoint()
{
	_asm sub esp, 0x50;        // 抬高栈顶，提高兼容性
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
	start();                    // 执行壳的主体部分
	_asm add esp, 0x50;        // 平衡堆栈

	/*_asm mov eax, g_StubConf.srcOep;
	_asm add eax, 0x400000;
	_asm jmp eax;*/
}
//***********************************************************************
//*   FUN：获取API函数                                                  * 
//*                                                                     *
//***********************************************************************
void GetApis()
{
	_asm
	{
		//获取kernel32.dll的加载基址;
		pushad;                                               
		//1. 找到PEB的首地址;
		mov esi, fs:[0x30];                                   //eax = > peb首地址;
		//2. 得到PEB.Ldr的值;
		mov esi, [esi + 0ch];                                 //eax = > PEB.Ldr的值;
		mov esi, [esi + 0ch];                                 //eax = > PEB.Ldr的值;
		//3. 得到_PEB_LDR_DATA.InLoadOrderMoudleList.Flink的值, 实际得到的就是主模块节点的首地址;
		mov esi, [esi];                                       //eax = > _PEB_LDR_DATA.InLoadOrderMoudleList.Flink(NTDLL);
		//4. 再获取下一个;
		mov esi, [esi];                                       //_LDR_DATA_TABLE_ENTRY.InLoadOrderMoudleList.Flink(kernel32), ;
		mov esi, [esi + 018h];                                //_LDR_DATA_TABLE_ENTRY.DllBase;
		mov edx, esi;                                         //dllBase

		//遍历导出表;
		//1. dos头-- > nt头-- > 扩展头-- > 数据目录表;
		mov eax, [esi + 03ch];                                //eax = > 偏移到NT头;
		lea eax, dword ptr[edx + eax];                        //ebx = > NT头的首地址;
		mov ebx, [eax + 78h];                                 //ebx = > mulubiao首地址;
		lea ebx, dword ptr[edx + ebx];                        //导出表VA  
		//2. 得到导出表的RVA;
		mov eax, [ebx + 1ch];               //获取FunAddr地址的RVA
		lea eax, [edx + eax];                //获得地址表的VA
		mov dword ptr[ebp - 4], eax;         //保存地址表VA
		mov eax, [ebx + 20h];               //获取名称表RVA
		lea eax, [edx + eax];
		mov dword ptr[ebp - 8h], eax;         //保存名称表VA
		mov eax, [ebx + 24h];
		lea eax, [edx + eax];
		mov dword ptr[ebp - 0ch], eax;       //保存序号表VA

		//3. 遍历名称表找到GetProcAddress;
		xor esi, esi;
		mov esi, [ebx + 18h];                //名称表个数
		xor ecx, ecx;
		//3.1 遍历名称表;
	_WHILE:
		mov eax, dword ptr[ebp - 8];
		mov esi, [eax + ecx * 4];                  //esi = > 名称的rva;
		lea esi, [esi + edx];                      //esi = > 名称首地址;
		cmp dword ptr[esi], 050746547h;            
		jne _LOOP;
		cmp dword ptr[esi + 4], 041636f72h;
		jne _LOOP;
		cmp dword ptr[esi + 8], 065726464h;
		jne _LOOP;
		cmp word  ptr[esi + 0ch], 07373h;
		jne _LOOP;
		// 找到之后;
		mov eax, dword ptr[ebp - 0ch];                 //edi = > 名称的序号表的va;
		mov ax, [eax + ecx * 2];                       //序号表是2字节的元素, 因此是 * 2;
		//eax保存的是GetProcAddress的在地址表中的下标;
		and eax, 0FFFFh;                               //得到地址表首地址;
		mov ecx, eax;
		mov eax, dword ptr[ebp - 4];                         //eax = > 地址表的va;
		mov edi, [eax + ecx * 4];                            //edi = > GetProcAddress的rva;
		lea edi, [edi + edx];                                // edx = > GetProcAddress的va;
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

	// 测试调用API
	g_hUser32 = MyLoadLibraryA("user32.dll");                     //User32模块加载

	HMODULE msvcrt = MyLoadLibraryA("msvcrt.dll");                //msvcrt模块加载
	MyMemcpy = (Fnmemcpy)
		MyGetProcAddress(msvcrt, "memcpy");

	HMODULE MSVCR120D = MyLoadLibraryA("MSVCR120D.dll");          //MSVCR120D模块加载
	MyMalloc = (Fnmalloc)
		MyGetProcAddress(MSVCR120D, "malloc");

	// MFC程序调用时有问题
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
		// 测试调用API
	MyGetProcAddress(g_hUser32, "MessageBoxA");

	// MFC程序调用时有问题
	MyMessageBox(0, L"大家好我是一个壳", L"提示", 0);

	g_PEBase = (DWORD)MyGetModuleHandle(NULL);
	g_hInstance = MyGetModuleHandle(NULL);
	CreateWindowFun();

}
//***********************************************************************
//*   FUN：窗口创建函数                                                 * 
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
//*   FUN：窗口回调函数                                                 * 
//*                                                                     *
//***********************************************************************
 LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE: 
	{
		/////////////////////////////////////////////////////////////////////////////////////
		//创建Ok按钮
		MyCreateWindowEx(0L, L"BUTTON", L"Ok", WS_CHILD | WS_VISIBLE,
			50, 100,50, 20,
			hwnd,(HMENU)0x1000,
			MyGetModuleHandle(NULL), NULL);
		//创建退出按钮
		MyCreateWindowEx(0L, L"BUTTON", L"No", WS_CHILD | WS_VISIBLE,
			200, 100,50, 20,
			hwnd,(HMENU)0x1001,
			MyGetModuleHandle(NULL), NULL);
		//创建编辑框
		DWORD dwStyle = ES_LEFT | WS_CHILD | WS_OVERLAPPED | WS_VISIBLE;
		DWORD dwExStyle = WS_EX_CLIENTEDGE | WS_EX_LEFT | WS_EX_LTRREADING | WS_EX_RIGHTSCROLLBAR;
		HWND hWnd = MyCreateWindowEx(
			dwExStyle, 	L"Edit", NULL, dwStyle, //dwStyle 窗口样式
			50, 50, 200, 20, 
			hwnd, //hWndParent 父窗口句柄
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
				wchar_t wStr[20] = L"壳跑了！！！";
				wchar_t wStr2[20] = L"Ultracis";
				MyMessageBox(NULL, wStr, wStr2, NULL);
				//解压缩文件
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
				//跳转至OEP
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
	// 返回默认的窗口处理过程
	return MyDefWindowProc(hwnd, uMsg, wParam, lParam);
}