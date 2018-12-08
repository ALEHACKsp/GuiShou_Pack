
#include "stub.h"
// �ϲ�.data��.text��
#pragma comment(linker,"/merge:.data=.text")
// �ϲ�.rdata��.text��
#pragma comment(linker,"/merge:.rdata=.text")
// ��.text�ĳɿɶ���д��ִ��
#pragma comment(linker, "/section:.text,RWE")



//����һ��ȫ�ֱ���
extern "C" __declspec(dllexport)StubConf g_conf = {0};

//���庯��ָ��ͱ���
typedef void* (WINAPI *FnGetProcAddress)(HMODULE, const char*);
FnGetProcAddress MyGetProcAddress;

typedef void* (WINAPI *FnLoadLibraryA)(char*);
FnLoadLibraryA MyLoadLibraryA;

typedef void* (WINAPI *FnVirtualProtect)(LPVOID, SIZE_T, DWORD, PDWORD);
FnVirtualProtect MyVirtualProtect;


//************************************************************
// ��������: Decrypt
// ����˵��: ���ܴ����
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/2
// ��	 ��: void
// �� �� ֵ: void
//************************************************************
void Decrypt()
{
	unsigned char* pText = (unsigned char*)g_conf.textScnRVA + 0x400000;
	//�޸Ĵ���ε�����
	DWORD old = 0;
	MyVirtualProtect(pText,g_conf.textScnSize,PAGE_READWRITE,&old);
	//���ܴ����
	for (DWORD i = 0; i < g_conf.textScnSize; i++)
	{
		pText[i] ^= g_conf.key;
	}
	//�������޸Ļ�ȥ
	MyVirtualProtect(pText,g_conf.textScnSize,old,&old);

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

	MyLoadLibraryA = (FnLoadLibraryA)MyGetProcAddress(hKernel32, "LoadLibrary");
	MyVirtualProtect = (FnVirtualProtect)MyGetProcAddress(hKernel32, "VirtualProtect");

	//���Ե���API

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
	//��ȡ������API��ַ
	GetApis();
	//���ܴ����
	Decrypt();
	//��ת��ԭʼOEP
	__asm
	{
		mov eax, g_conf.srcOep;
		add eax,0x400000
		jmp eax
	}
}
