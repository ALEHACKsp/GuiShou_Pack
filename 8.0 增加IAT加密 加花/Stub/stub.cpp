
#include "stub.h"
#include "AES.h"
// �ϲ�.data��.text��
#pragma comment(linker,"/merge:.data=.text")
// �ϲ�.rdata��.text��
#pragma comment(linker,"/merge:.rdata=.text")
// ��.text�ĳɿɶ���д��ִ��
#pragma comment(linker, "/section:.text,RWE")




IMAGE_DOS_HEADER* GetDosHeader(char* pFileData)
{
	return (IMAGE_DOS_HEADER *)pFileData;
}
DosStub* GetDosSubHeader(char* pFileData)
{
	return (DosStub*)(pFileData + sizeof(IMAGE_DOS_HEADER));
}
IMAGE_NT_HEADERS* GetNtHeader(char* pFileData)
{
	return (IMAGE_NT_HEADERS*)(GetDosHeader(pFileData)->e_lfanew + (SIZE_T)pFileData);
}
IMAGE_FILE_HEADER* GetFileHeader(char* pFileData)
{
	return &GetNtHeader(pFileData)->FileHeader;
}
IMAGE_OPTIONAL_HEADER* GetOptionHeader(char* pFileData)
{
	return &GetNtHeader(pFileData)->OptionalHeader;
}


//����һ��ȫ�ֱ���
extern "C" __declspec(dllexport)StubConf g_conf = {0};



HINSTANCE g_hInstance;	//���봰��ʵ�����
HWND hEdit;				//�������봰��
BOOL bSuccess;			//������֤	

//���庯��ָ��ͱ���
//Kernel32
typedef void* (WINAPI *FnGetProcAddress)(HMODULE, const char*);
FnGetProcAddress MyGetProcAddress;

typedef void* (WINAPI *FnLoadLibraryA)(char*);
FnLoadLibraryA MyLoadLibraryA;

typedef void* (WINAPI *FnVirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
FnVirtualProtect MyVirtualProtect;

typedef HMODULE(WINAPI *fnGetMoudleHandleA)(_In_ LPCWSTR lpMoudleName);
fnGetMoudleHandleA pfnGetMoudleHandleA;


//User32
typedef ATOM(WINAPI *fnRegisterClassEx)(_In_ const WNDCLASSEX *lpwcx);
fnRegisterClassEx pfnRegisterClassEx;

typedef HWND(WINAPI *fnCreateWindowEx)(
	_In_ DWORD dwExStyle,
	_In_opt_ LPCTSTR lpClassName,
	_In_opt_ LPCTSTR lpWindowName,
	_In_ DWORD dwStyle,
	_In_ int x,
	_In_ int y,
	_In_ int nWidth,
	_In_ int nHeight,
	_In_opt_ HWND hWndParent,
	_In_opt_ HMENU hMenu,
	_In_opt_ HINSTANCE hInstance,
	_In_opt_ LPVOID lpParam
	);
fnCreateWindowEx pfnCreateWindowEx;

typedef BOOL(*fnUpdateWindow)(HWND hWnd);
fnUpdateWindow pfnUpdateWindow;

typedef BOOL (WINAPI* fnShowWindow)(_In_ HWND hWnd,_In_ int nCmdShow);
fnShowWindow pfnShowWindow;

typedef BOOL (WINAPI* fnGetMessage)(_Out_ LPMSG lpMsg,_In_opt_ HWND hWnd,_In_ UINT wMsgFilterMin,_In_ UINT wMsgFilterMax);
fnGetMessage pfnGetMessage;

typedef BOOL (WINAPI* fnTranslateMessage)(_In_ CONST MSG *lpMsg);
fnTranslateMessage pfnTranslateMessage;

typedef LRESULT (WINAPI* fnDispatchMessageW)(_In_ CONST MSG *lpMsg);
fnDispatchMessageW pfnDispatchMessageW;

typedef int (WINAPI* fnGetWindowTextW)(_In_ HWND hWnd,_Out_writes_(nMaxCount) LPWSTR lpString,_In_ int nMaxCount);
fnGetWindowTextW pfnGetWindowTextW;

typedef void (WINAPI* fnExitProcess)(_In_ UINT uExitCode);
fnExitProcess pfnExitProcess;

typedef LRESULT (WINAPI* fnSendMessageW)(
	_In_ HWND hWnd,
	_In_ UINT Msg,
	_Pre_maybenull_ _Post_valid_ WPARAM wParam,
	_Pre_maybenull_ _Post_valid_ LPARAM lParam);
fnSendMessageW pfnSendMessageW;

typedef LRESULT (WINAPI* fnDefWindowProcW)(_In_ HWND hWnd,_In_ UINT Msg,_In_ WPARAM wParam,_In_ LPARAM lParam);
fnDefWindowProcW pfnDefWindowProcW;

typedef void (WINAPI* fnPostQuitMessage)(_In_ int nExitCode);
fnPostQuitMessage pfnPostQuitMessage;

typedef HWND (WINAPI* fnFindWindowW)(_In_opt_ LPCWSTR lpClassName,_In_opt_ LPCWSTR lpWindowName);
fnFindWindowW pfnFindWindowW;

typedef int (WINAPI* fnMessageBoxW)(_In_opt_ HWND hWnd,_In_opt_ LPCWSTR lpText,_In_opt_ LPCWSTR lpCaption,_In_ UINT uType);
fnMessageBoxW pfnMessageBoxW;

typedef HDC (WINAPI* fnBeginPaint)(_In_ HWND hWnd,_Out_ LPPAINTSTRUCT lpPaint);
fnBeginPaint pfnBeginPaint;

typedef BOOL (WINAPI* fnEndPaint)(_In_ HWND hWnd,_In_ CONST PAINTSTRUCT *lpPaint);
fnEndPaint pfnEndPaint;

typedef LPVOID(WINAPI* FnVirtualAlloc)(LPVOID, SIZE_T, DWORD, DWORD);
FnVirtualAlloc pfnVirtualAlloc;

typedef VOID(WINAPI* FnRtlMoveMemory)(LPVOID, LPVOID, SIZE_T);
FnRtlMoveMemory pfnRtlMoveMemory;

//������Ϣ�ص�����
LRESULT CALLBACK WndPrco(HWND,UINT,WPARAM,LPARAM);

//************************************************************
// ��������: Decrypt
// ����˵��: ���ܴ����
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/2
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
void  Decrypt()
{
	//��ȡ��ǰ����Ļ�ַ
	DWORD dwBase = (DWORD)pfnGetMoudleHandleA(NULL);

	AES aes(g_conf.key);
	//ѭ��������������
	DWORD old = 0;
	for (int i = 0; i < g_conf.index-1; i++)
	{
		//�õ��������ε��׵�ַ�ʹ�С
		unsigned char* pSection = (unsigned char*)g_conf.data[i][0]+ dwBase;
		DWORD dwSectionSize = g_conf.data[i][1];

		//�޸���������
		MyVirtualProtect(pSection, dwSectionSize, PAGE_EXECUTE_READWRITE, &old);

		//���ܴ����
		aes.InvCipher(pSection, dwSectionSize);

		//�������޸Ļ�ȥ
		MyVirtualProtect(pSection, dwSectionSize, old, &old);
	}
}


//************************************************************
// ��������: GetApis
// ����˵��: ��ȡAPI������ַ
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/2
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
void GetApis()
{
	HMODULE hKernel32;

	_asm
	{
		pushad;
		; //��ȡkernel32.dll�ļ��ػ�ַ;
		;// 1. �ҵ�PEB���׵�ַ;
		mov eax, fs:[0x30]; eax = > peb�׵�ַ;
		; 2. �õ�PEB.Ldr��ֵ;
		mov eax, [eax + 0ch]; eax = > PEB.Ldr��ֵ;
		mov eax, [eax + 0ch]; eax = > PEB.Ldr��ֵ;
		; 3. �õ�_PEB_LDR_DATA.InLoadOrderMoudleList.Flink��ֵ, ʵ�ʵõ��ľ�����ģ��ڵ���׵�ַ;
		mov eax, [eax]; eax = > _PEB_LDR_DATA.InLoadOrderMoudleList.Flink(NTDLL);
		; 4. �ٻ�ȡ��һ��;
		mov eax, [eax]; _LDR_DATA_TABLE_ENTRY.InLoadOrderMoudleList.Flink(kernel32), ;
		mov eax, [eax + 018h]; _LDR_DATA_TABLE_ENTRY.DllBase;
		mov hKernel32, eax;;
		; ����������;
		; 1. dosͷ-- > ntͷ-- > ��չͷ-- > ����Ŀ¼��;
		mov ebx, [eax + 03ch]; eax = > ƫ�Ƶ�NTͷ;
		add ebx, eax; ebx = > NTͷ���׵�ַ;
		add ebx, 078h; ebx = >
			; 2. �õ��������RVA;
		mov ebx, [ebx];
		add ebx, eax; ebx == > �������׵�ַ(VA);
		; 3. �������Ʊ��ҵ�GetProcAddress;
		; 3.1 �ҵ����Ʊ���׵�ַ;
		lea ecx, [ebx + 020h];
		mov ecx, [ecx]; // ecx => ���Ʊ���׵�ַ(rva);
		add ecx, eax; // ecx => ���Ʊ���׵�ַ(va);
		xor edx, edx; // ��Ϊindex��ʹ��.
		; 3.2 �������Ʊ�;
	_WHILE:;
		mov esi, [ecx + edx * 4]; esi = > ���Ƶ�rva;
		lea esi, [esi + eax]; esi = > �����׵�ַ;
		cmp dword ptr[esi], 050746547h; 47657450 726F6341 64647265 7373;
		jne _LOOP;
		cmp dword ptr[esi + 4], 041636f72h;
		jne _LOOP;
		cmp dword ptr[esi + 8], 065726464h;
		jne _LOOP;
		cmp word  ptr[esi + 0ch], 07373h;
		jne _LOOP;
		; �ҵ�֮��;
		mov edi, [ebx + 024h]; edi = > ���Ƶ���ű��rva;
		add edi, eax; edi = > ���Ƶ���ű��va;

		mov di, [edi + edx * 2]; ��ű���2�ֽڵ�Ԫ��, ����� * 2;
		; edi�������GetProcAddress����;
		; ��ַ���е��±�;
		and edi, 0FFFFh;
		; �õ���ַ���׵�ַ;
		mov edx, [ebx + 01ch]; edx = > ��ַ���rva;
		add edx, eax; edx = > ��ַ���va;
		mov edi, [edx + edi * 4]; edi = > GetProcAddress��rva;
		add edi, eax; ; edx = > GetProcAddress��va;
		mov MyGetProcAddress, edi;
		jmp _ENDWHILE;
	_LOOP:;
		inc edx; // ++index;
		jmp _WHILE;
	_ENDWHILE:;
		popad;
 	}
	//������ָ�������ֵ
	//Kernel32
	MyLoadLibraryA = (FnLoadLibraryA)MyGetProcAddress(hKernel32, "LoadLibraryA");
	MyVirtualProtect = (FnVirtualProtect)MyGetProcAddress(hKernel32, "VirtualProtect");
	pfnGetMoudleHandleA = (fnGetMoudleHandleA)MyGetProcAddress(hKernel32, "GetModuleHandleA");
	pfnExitProcess = (fnExitProcess)MyGetProcAddress(hKernel32, "ExitProcess");
	pfnVirtualAlloc = (FnVirtualAlloc)MyGetProcAddress(hKernel32, "VirtualAlloc");
	pfnRtlMoveMemory = (FnRtlMoveMemory)MyGetProcAddress(hKernel32, "RtlMoveMemory");
	HMODULE hUser32 = (HMODULE)MyLoadLibraryA((char*)"user32.dll");

	//User32
	pfnRegisterClassEx = (fnRegisterClassEx)MyGetProcAddress(hUser32, "RegisterClassExW");
	pfnCreateWindowEx = (fnCreateWindowEx)MyGetProcAddress(hUser32, "CreateWindowExW");
	pfnUpdateWindow = (fnUpdateWindow)MyGetProcAddress(hUser32, "UpdateWindow");
	pfnShowWindow=(fnShowWindow)MyGetProcAddress(hUser32, "ShowWindow");
	pfnGetMessage=(fnGetMessage)MyGetProcAddress(hUser32, "GetMessageW");
	pfnTranslateMessage=(fnTranslateMessage)MyGetProcAddress(hUser32, "TranslateMessage");
	pfnDispatchMessageW =(fnDispatchMessageW)MyGetProcAddress(hUser32, "DispatchMessageW");
	pfnGetWindowTextW =(fnGetWindowTextW)MyGetProcAddress(hUser32, "GetWindowTextW");
	pfnSendMessageW =(fnSendMessageW)MyGetProcAddress(hUser32, "SendMessageW");
	pfnDefWindowProcW =(fnDefWindowProcW)MyGetProcAddress(hUser32, "DefWindowProcW");
	pfnPostQuitMessage =(fnPostQuitMessage)MyGetProcAddress(hUser32, "PostQuitMessage");
	pfnFindWindowW =(fnFindWindowW)MyGetProcAddress(hUser32, "FindWindowW");
	pfnMessageBoxW =(fnMessageBoxW)MyGetProcAddress(hUser32, "MessageBoxW");
	pfnBeginPaint =(fnBeginPaint)MyGetProcAddress(hUser32, "BeginPaint");
	pfnEndPaint =(fnEndPaint)MyGetProcAddress(hUser32, "EndPaint");

}



//************************************************************
// ��������: MyWcscmp
// ����˵��: �Լ�ʵ�ֵ�һ���ַ����ȽϺ���
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/4
// ��	 ��: const wchar_t * src, const wchar_t * dst
// �� �� ֵ: int ��ȷ���0 ����ȷ���1
//************************************************************
int MyWcscmp(const wchar_t * src, const wchar_t * dst)
{
	int ret = 0;
	while (!(ret = *(wchar_t *)src - *(wchar_t *)dst) && *dst)
		++src, ++dst;
	if (ret < 0)
		ret = -1;
	else if (ret > 0)
		ret = 1;
	return(ret);
}


//************************************************************
// ��������: AlertPasswordBox
// ����˵��: ���뵯��
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/4
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
void AlertPasswordBox()
{
	//ע�ᴰ����
	g_hInstance = (HINSTANCE)pfnGetMoudleHandleA(NULL);
	WNDCLASSEX ws;
	ws.cbSize = sizeof(WNDCLASSEX);
	ws.hInstance = g_hInstance;
	ws.cbWndExtra = ws.cbClsExtra = NULL;
	ws.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	ws.hIcon = NULL;
	ws.hIconSm = NULL;
	ws.hCursor = NULL;
	ws.style = CS_VREDRAW | CS_HREDRAW;
	ws.lpszMenuName = NULL;
	ws.lpfnWndProc = (WNDPROC)WndPrco;		//	�ص�����
	ws.lpszClassName = TEXT("GuiShou");
	pfnRegisterClassEx(&ws);
	//��������
	HWND hWnd = pfnCreateWindowEx(0,TEXT("GuiShou"),TEXT("���뵯��"),
		WS_OVERLAPPED|WS_VISIBLE,
		100,100,400,200,NULL,NULL,g_hInstance,NULL);
	//���´���
	//pfnUpdateWindow(hWnd);
	pfnShowWindow(hWnd, SW_SHOW);
	//��Ϣ����
	MSG msg = { 0 };
	while (pfnGetMessage(&msg,NULL,NULL,NULL))
	{
		pfnTranslateMessage(&msg);
		pfnDispatchMessageW(&msg);
	}
}


//************************************************************
// ��������: AntiDebug
// ����˵��: ������
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/4
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
void AntiDebug()
{

	bool BeingDugged = false;
	__asm
	{
		mov eax, DWORD ptr fs : [0x30];//��ȡpeb
		mov al, byte ptr ds : [eax + 0x02];//��ȡpeb.beingdugged
		mov BeingDugged, al;
	}
	if (BeingDugged)
	{
		pfnMessageBoxW(NULL, L"��һ�� �㱻������", L"ע��", MB_OK);
	}

}


//************************************************************
// ��������: MixFun
// ����˵��: ��������
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/4
// ��	 ��: DWORD funcAddress ������ַ
// �� �� ֵ: void
//************************************************************
void _stdcall FusedFunc(DWORD funcAddress)
{
	_asm
	{
		jmp label1
		label2 :
		_emit 0xeb; //���������call
		_emit 0x04;
		CALL DWORD PTR DS : [EAX + EBX * 2 + 0x123402EB]; //ִ��EB 02  Ҳ����������һ��

														  //	call Init;// ��ȡһЩ���������ĵ�ַ

														  // call��һ��,���ڻ��eip
		_emit 0xE8;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		//-------���������call
		_emit 0xEB;
		_emit 0x0E;

		//-------��
		PUSH 0x0;
		PUSH 0x0;
		MOV EAX, DWORD PTR FS : [0];
		PUSH EAX;
		//-------��


		// fused:
		//����push��һ�����ĵ�ַ
		//pop eax;
		//add eax, 0x1b;
		/*push eax;*/
		CALL DWORD PTR DS : [EAX + EBX * 2 + 0x5019C083];

		push funcAddress; //��������ǲ����������Ҫע�������add eax,??��??
		retn;

		jmp label3

			// ��
			_emit 0xE8;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		_emit 0x00;
		// ��


	label1:
		jmp label2
			label3 :
	}
}




//�������Կ�д
void SetFileHeaderProtect(bool nWrite)
{
	//��ȡ��ǰ����ļ��ػ�ַ
	DWORD ImageBase = (DWORD)pfnGetMoudleHandleA(NULL);
	DWORD nOldProtect = 0;
	if (nWrite)
		MyVirtualProtect((LPVOID)ImageBase, 0x400, PAGE_EXECUTE_READWRITE, &nOldProtect);
	else
		MyVirtualProtect((LPVOID)ImageBase, 0x400, nOldProtect, &nOldProtect);
}



//************************************************************
// ��������: FixImportTable_Normal
// ����˵��: �޸�IAT
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/7
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
void FixImportTable_Normal()
{
	//�����ļ�����Ϊ��д
	SetFileHeaderProtect(true);
	//��ȡ��ǰ����ļ��ػ�ַ
	DWORD ImageBase = (DWORD)pfnGetMoudleHandleA(NULL);

	IMAGE_THUNK_DATA* pInt = NULL;
	IMAGE_THUNK_DATA* pIat = NULL;
	SIZE_T impAddress = 0;
	HMODULE	hImpModule = 0;
	DWORD dwOldProtect = 0;
	IMAGE_IMPORT_BY_NAME* pImpName = 0;

	if (!GetOptionHeader((char*)ImageBase)->DataDirectory[1].VirtualAddress)return;

	//�����=�����ƫ��+���ػ�ַ
	IMAGE_IMPORT_DESCRIPTOR* pImp = (IMAGE_IMPORT_DESCRIPTOR*)(GetOptionHeader((char*)ImageBase)->DataDirectory[1].VirtualAddress + ImageBase);


	while (pImp->Name)
	{
		//IAT=ƫ�ƼӼ��ػ�ַ
		pIat = (IMAGE_THUNK_DATA*)(pImp->FirstThunk + ImageBase);
		if (pImp->OriginalFirstThunk == 0) // ���������INT��ʹ��IAT
		{
			pInt = pIat;
		}
		else
		{
			pInt = (IMAGE_THUNK_DATA*)(pImp->OriginalFirstThunk + ImageBase);
		}

		// ����dll
		hImpModule = (HMODULE)MyLoadLibraryA((char*)(pImp->Name + ImageBase));
		//���뺯����ַ
		while (pInt->u1.Function)
		{
			//�жϵ���ķ�ʽ����Ż�������
			if (!IMAGE_SNAP_BY_ORDINAL(pInt->u1.Ordinal))
			{
				pImpName = (IMAGE_IMPORT_BY_NAME*)(pInt->u1.Function + ImageBase);
				impAddress = (SIZE_T)MyGetProcAddress(hImpModule, (char*)pImpName->Name);
			}
			else
			{
				impAddress = (SIZE_T)MyGetProcAddress(hImpModule, (char*)(pInt->u1.Function & 0xFFFF));
			}

			MyVirtualProtect(&pIat->u1.Function, sizeof(pIat->u1.Function), PAGE_READWRITE, &dwOldProtect);


			pIat->u1.Function = impAddress;
			MyVirtualProtect(&pIat->u1.Function, sizeof(pIat->u1.Function), dwOldProtect, &dwOldProtect);
			++pInt;
			++pIat;
		}
		++pImp;
	}
	SetFileHeaderProtect(false);
}



DWORD EncryptFun(DWORD dwFunAddr)
{
	// 1.�����ڴ�ռ�
	DWORD dwNewMem = (DWORD)pfnVirtualAlloc(NULL, 0x20, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	// 2.���ܺ�����ַ
	//DWORD dwEncryptFunAddr = dwFunAddr ^ 0x15151515;
	DWORD dwEncryptFunAddr = 0;
	_asm
	{
		push eax;
		mov eax, dwFunAddr;
		xor eax, 0x15151515;
		mov dwEncryptFunAddr, eax;
		pop eax;
	}

	// 3.��OpCode[11]���ĵ�ַ���и�д
	BYTE OpCode[] = {
					0xE8, 0x01, 0x00, 0x00,
					0x00, 0xE9, 0x58, 0xEB,
					0x01, 0xE8, 0xB8, 0x85,
					0xEE, 0xCB, 0x60, 0xEB,
					0x01, 0x15, 0x35, 0x15,
					0x15, 0x15, 0x15, 0xEB,
					0x01, 0xFF, 0x50, 0xEB,
					0x02, 0xFF, 0x15, 0xC3
	};
	OpCode[11] = dwEncryptFunAddr;					// 0x85
	OpCode[12] = dwEncryptFunAddr >> 0x08;			// 0xEE
	OpCode[13] = dwEncryptFunAddr >> 0x10;			// 0xCB
	OpCode[14] = dwEncryptFunAddr >> 0x18;			// 0x60

	// 4.�����ݿ�����������ڴ�
	pfnRtlMoveMemory((LPVOID)dwNewMem, OpCode, 0x20);

	// 5.�����µĺ�����ַ
	return dwNewMem;
}



//************************************************************
// ��������: EncodeIAT
// ����˵��: ����IAT
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/7
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
void EncodeIAT()
{
	//�����ļ�����Ϊ��д
	SetFileHeaderProtect(true);
	//��ȡ��ǰ����ļ��ػ�ַ
	DWORD ImageBase = (DWORD)pfnGetMoudleHandleA(NULL);

	IMAGE_THUNK_DATA* pInt = NULL;
	IMAGE_THUNK_DATA* pIat = NULL;
	SIZE_T impAddress = 0;
	HMODULE	hImpModule = 0;
	DWORD dwOldProtect = 0;
	IMAGE_IMPORT_BY_NAME* pImpName = 0;

	if (!GetOptionHeader((char*)ImageBase)->DataDirectory[1].VirtualAddress)return;

	//�����=�����ƫ��+���ػ�ַ
	IMAGE_IMPORT_DESCRIPTOR* pImp = (IMAGE_IMPORT_DESCRIPTOR*)(GetOptionHeader((char*)ImageBase)->DataDirectory[1].VirtualAddress + ImageBase);


	while (pImp->Name)
	{
		//IAT=ƫ�ƼӼ��ػ�ַ
		pIat = (IMAGE_THUNK_DATA*)(pImp->FirstThunk + ImageBase);
		if (pImp->OriginalFirstThunk == 0) // ���������INT��ʹ��IAT
		{
			pInt = pIat;
		}
		else
		{
			pInt = (IMAGE_THUNK_DATA*)(pImp->OriginalFirstThunk + ImageBase);
		}

		// ����dll
		hImpModule = (HMODULE)MyLoadLibraryA((char*)(pImp->Name + ImageBase));
		//���뺯����ַ
		while (pInt->u1.Function)
		{
			//�жϵ���ķ�ʽ����Ż�������
			if (!IMAGE_SNAP_BY_ORDINAL(pInt->u1.Ordinal))
			{
				pImpName = (IMAGE_IMPORT_BY_NAME*)(pInt->u1.Function + ImageBase);
				impAddress = (SIZE_T)MyGetProcAddress(hImpModule, (char*)pImpName->Name);
			}
			else
			{
				impAddress = (SIZE_T)MyGetProcAddress(hImpModule, (char*)(pInt->u1.Function & 0xFFFF));
			}

			MyVirtualProtect(&pIat->u1.Function, sizeof(pIat->u1.Function), PAGE_READWRITE, &dwOldProtect);


			pIat->u1.Function = EncryptFun(impAddress);
			MyVirtualProtect(&pIat->u1.Function, sizeof(pIat->u1.Function), dwOldProtect, &dwOldProtect);
			++pInt;
			++pIat;
		}
		++pImp;
	}
	SetFileHeaderProtect(false);
}




//************************************************************
// ��������: RecoverDataDir
// ����˵��: �ָ�����Ŀ¼��
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/4
// ��	 ��: DWORD funcAddress ������ַ
// �� �� ֵ: void
//************************************************************
void RecoverDataDir()
{	
	//��ȡ��ǰ����ļ��ػ�ַ
	char* dwBase = (char*)pfnGetMoudleHandleA(NULL);
	//��ȡ����Ŀ¼��ĸ���
	DWORD dwNumOfDataDir = g_conf.dwNumOfDataDir;

	DWORD dwOldAttr = 0;
	PIMAGE_DATA_DIRECTORY pDataDirectory = (GetOptionHeader(dwBase)->DataDirectory);
	//��������Ŀ¼��
	for (DWORD i = 0; i < dwNumOfDataDir; i++)
	{
		if (i == 2)
		{
			pDataDirectory++;
			continue;
		}

		//�޸�����Ϊ�ɶ���д
		MyVirtualProtect(pDataDirectory, 0x8, PAGE_EXECUTE_READWRITE, &dwOldAttr);

		//��ԭ����Ŀ¼����
		pDataDirectory->VirtualAddress = g_conf.dwDataDir[i][0];
		pDataDirectory->Size = g_conf.dwDataDir[i][1];

		//�������޸Ļ�ȥ
		MyVirtualProtect(pDataDirectory, 0x8, dwOldAttr, &dwOldAttr);

		pDataDirectory++;
	}
}




void CallTls()
{
	//��ȡ��ǰ����ļ��ػ�ַ
	DWORD dwBase = (DWORD)pfnGetMoudleHandleA(NULL);
	//��ȡTls��
	DWORD dwTlsRva = GetOptionHeader((char*)dwBase)->DataDirectory[9].VirtualAddress;
	if (dwTlsRva != 0)
	{
		PIMAGE_TLS_DIRECTORY pTlsTab = (PIMAGE_TLS_DIRECTORY)(dwTlsRva + dwBase);
		if (pTlsTab->AddressOfCallBacks == 0)
		{
			return;
		}
		DWORD nTlsCallBacks = *(DWORD*)pTlsTab->AddressOfCallBacks;
		__asm
		{
			cmp nTlsCallBacks, 0
			je ENDCALL
			push 0
			push 1
			push dwBase
			call nTlsCallBacks
			ENDCALL :
		}
	}
	
}



// �ǳ���
int g_num11 = 10;
void AllFunc()
{
	// �ݹ�ִ��10�κ�ִ�пǳ���
	if (!g_num11)
	{
		_asm
		{
			nop
			mov   ebp, esp
			push - 1
			push   0
			push   0
			mov   eax, fs:[0]
			push   eax
			mov   fs : [0], esp
			sub   esp, 0x68
			push   ebx
			push   esi
			push   edi
			pop   eax
			pop   eax
			pop   eax
			add   esp, 0x68
			pop   eax
			mov   fs : [0], eax
			pop   eax

			sub g_num11, 1

			pop   eax
			pop   eax
			pop   eax
			mov   ebp, eax

			push AllFunc
			call FusedFunc
		}
	}


	//��ȡ������API��ַ
	FusedFunc((DWORD)GetApis);
	
	//���ܴ����
	FusedFunc((DWORD)Decrypt);
	
	//�ָ�����Ŀ¼��
	FusedFunc((DWORD)RecoverDataDir);
	
	//�޸�IAT
	FusedFunc((DWORD)FixImportTable_Normal);

	//������
	FusedFunc((DWORD)AntiDebug);
	
	//���뵯��
	FusedFunc((DWORD)AlertPasswordBox);
	
	//����Tls�ص�����
	FusedFunc((DWORD)CallTls);

	//����IAT
	FusedFunc((DWORD)EncodeIAT);

}



//************************************************************
// ��������: Start
// ����˵��: dll��OEP
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/2
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
extern "C" __declspec(dllexport) __declspec(naked)
void Start()
{

	// ��ָ��
	_asm
	{
		PUSH - 1
		PUSH 0
		PUSH 0
		MOV EAX, DWORD PTR FS : [0]
		PUSH EAX
		MOV DWORD PTR FS : [0], ESP
		SUB ESP, 0x68
		PUSH EBX
		PUSH ESI
		PUSH EDI
		POP EAX
		POP EAX
		POP EAX
		ADD ESP, 0x68
		POP EAX
		MOV DWORD PTR FS : [0], EAX
		POP EAX
		POP EAX
		POP EAX
		POP EAX
		MOV EBP, EAX
	}
	
	// ִ�п�
	FusedFunc((DWORD)AllFunc);

	////��ȡ������API��ַ
	//GetApis();
	////���ܴ����
	//Decrypt();
	////�ָ�����Ŀ¼��
	//RecoverDataDir();
	////�޸�IAT
	//FixImportTable_Normal();
	////������
	//AntiDebug();
	////���뵯��
	//AlertPasswordBox();
	////����Tls�ص�����
	//CallTls();
	////����IAT
	//EncodeIAT();

	//��ת��ԭʼOEP
	__asm
	{
		mov eax, g_conf.srcOep;
		add eax,0x400000
		jmp eax
	}
}


//************************************************************
// ��������: WndPrco
// ����˵��: ���ڻص�����
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/4
// ��	 ��: HWND, UINT, WPARAM, LPARAM
// �� �� ֵ: LRESULT
//************************************************************
LRESULT CALLBACK WndPrco(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	switch (msg)
	{
	case WM_CREATE:
	{
		pfnCreateWindowEx(0,L"button",L"ȷ��",WS_CHILD|WS_VISIBLE,
			70,115,60,30,hWnd,(HMENU)10001,g_hInstance,NULL);
		pfnCreateWindowEx(0, L"button", L"ȡ��", WS_CHILD | WS_VISIBLE,
			270, 115, 60, 30, hWnd, (HMENU)10002, g_hInstance, NULL);
		hEdit = pfnCreateWindowEx(0, L"edit", L"", WS_CHILD | WS_VISIBLE|WS_BORDER,
			150, 50, 100, 30, hWnd, (HMENU)10003, g_hInstance, NULL);
		HWND hBit = pfnCreateWindowEx(0, L"static", L"����", WS_CHILD | WS_VISIBLE,
			70, 50, 70, 30, hWnd, (HMENU)10004, g_hInstance, NULL);
		bSuccess = FALSE;
		break;
	}
	case  WM_COMMAND:
	{
		WORD wHigh = HIWORD(wParam);
		WORD wLow = LOWORD(wParam);
		switch (wLow)
		{
		case 10001:
		{
			TCHAR GetKey[10] = { 0 };
			pfnGetWindowTextW(hEdit,GetKey,10);
			//����������123
			if (MyWcscmp(GetKey,L"123")==0)
			{
				bSuccess = TRUE;
				//�������ƥ�� ��������
				pfnSendMessageW(hWnd,WM_CLOSE,NULL,NULL);
			}
			else
			{
				//���벻ƥ���˳�����
				pfnExitProcess(1);
			}
			break;
		}
		case 10002:		//ȡ����ť
		{
			pfnExitProcess(1);
			break;
		}
			
		default:
			break;
		}
		break;
	}
	case WM_PAINT:
	{
		hdc = pfnBeginPaint(hWnd, &ps);
		// TODO:  �ڴ���������ͼ����...
		pfnEndPaint(hWnd, &ps);
		break;
	}
	case WM_CLOSE:case WM_QUIT:case WM_DESTROY:
	{
		if (bSuccess)
		{
			pfnPostQuitMessage(0);
		}
		else
		{
			pfnExitProcess(1);
		}
	}

	default:
		return pfnDefWindowProcW(hWnd,msg,wParam,lParam);
	}
	return 0;
}

