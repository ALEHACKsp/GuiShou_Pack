#include "CPeFileOper.h"



CPeFileOper::CPeFileOper()
{
}


CPeFileOper::~CPeFileOper()
{
}





//************************************************************
// ��������: OpenPeFile
// ����˵��: ��PE�ļ�
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��: _In_ const char* path �ļ�·�� 
// �� �� ֵ: HANDLE �ļ����
HANDLE CPeFileOper::OpenPeFile(_In_ const char* path) {
	return CreateFileA(path,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
}

//************************************************************
// ��������: GetFileData
// ����˵��: ��ȡ�ļ����ݺʹ�С
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��: _In_ const char* pFilePath �ļ�·�� _Out_opt_ int* nFileSize �ļ���С
// �� �� ֵ: char* �ļ����
//************************************************************
char* CPeFileOper::GetFileData(_In_ const char* pFilePath,
	_Out_opt_ int* nFileSize) {
	// ���ļ�
	HANDLE hFile = OpenPeFile(pFilePath);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;
	// ��ȡ�ļ���С
	DWORD dwSize = GetFileSize(hFile, NULL);
	if (nFileSize)
		*nFileSize = dwSize;
	// ����Կռ�
	char* pFileBuff = new char[dwSize]{0};

	// ��ȡ�ļ����ݵ��ѿռ�
	DWORD dwRead = 0;
	ReadFile(hFile, pFileBuff, dwSize, &dwRead, NULL);
	CloseHandle(hFile);
	// ���ѿռ䷵��
	return pFileBuff;
}


//************************************************************
// ��������: GetDosHeader
// ����˵��: ��ȡDosͷ
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��: _In_  char* pFileData �ļ��׵�ַ
// �� �� ֵ: IMAGE_DOS_HEADER* Dosͷ
//************************************************************
IMAGE_DOS_HEADER* CPeFileOper::GetDosHeader(_In_ char* pFileData)
{
	return (IMAGE_DOS_HEADER*)pFileData;
}

//************************************************************
// ��������: GetNtHeader
// ����˵��: ��ȡNtͷ
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��: _In_  char* pFileData �ļ��׵�ַ
// �� �� ֵ: IMAGE_FILE_HEADER* Ntͷ
//************************************************************
IMAGE_NT_HEADERS* CPeFileOper::GetNtHeader(_In_ char* pFileData)
{
	return (IMAGE_NT_HEADERS*)(GetDosHeader(pFileData)->e_lfanew+(SIZE_T)pFileData);
}

//************************************************************
// ��������: GetFileHead
// ����˵��: ��ȡ�ļ�ͷ
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��: _In_  char* pFileData �ļ��׵�ַ
// �� �� ֵ: IMAGE_FILE_HEADER* �ļ�ͷ
//************************************************************
IMAGE_FILE_HEADER* CPeFileOper::GetFileHead(_In_ char* pFileData)
{
	return &GetNtHeader(pFileData)->FileHeader;
}



//************************************************************
// ��������: GetOptionHeader
// ����˵��: ��ȡ��ѡͷ
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��: _In_  char* pFileData �ļ��׵�ַ
// �� �� ֵ: IMAGE_OPTIONAL_HEADER* ��ѡͷ
//************************************************************
IMAGE_OPTIONAL_HEADER* CPeFileOper::GetOptionHeader(_In_ char* pFileData)
{
	return &GetNtHeader(pFileData)->OptionalHeader;
}

//************************************************************
// ��������: GetLastSection
// ����˵��: ��ȡ���һ������
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��: _In_  char* pFileData �ļ��׵�ַ
// �� �� ֵ: IMAGE_SECTION_HEADER* ����ͷ
//************************************************************
IMAGE_SECTION_HEADER* CPeFileOper::GetLastSection(_In_ char* pFileData)
{
	//��ȡ���θ���
	DWORD dwScnCount = GetFileHead(pFileData)->NumberOfSections;
	//��ȡ��һ������
	IMAGE_SECTION_HEADER* pScn = IMAGE_FIRST_SECTION(GetNtHeader(pFileData));
	//�õ����һ����Ч����
	return pScn + (dwScnCount - 1);
}


//************************************************************
// ��������: AlignMent
// ����˵��: ��������Ĵ�С
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��1: _In_ int size ��С
// ��	 ��2: _In_ int alignment ��������
// �� �� ֵ: int �����Ĵ�С
//************************************************************
int CPeFileOper::AlignMent(_In_ int size, _In_ int alignment)
{
	return (size) % (alignment) == 0 ? (size) : ((size) / (alignment)+ 1)*(alignment);
}



//************************************************************
// ��������: GetSection
// ����˵��: ��ȡָ�����ֵ�����ͷ
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��1: _In_ char* pFileData Ŀ���ļ��׵�ַ
// ��	 ��2:  _In_ const char* scnName ������
// �� �� ֵ: IMAGE_SECTION_HEADER* ����ͷ
//************************************************************
IMAGE_SECTION_HEADER* CPeFileOper::GetSection(_In_ char* pFileData, _In_ const char* scnName)
{
	//��ȡ���θ�ʽ
	DWORD dwScnCount = GetFileHead(pFileData)->NumberOfSections;
	//��ȡ��һ������
	IMAGE_SECTION_HEADER* pScn = IMAGE_FIRST_SECTION(GetNtHeader(pFileData));
	char buf[10] = { 0 };
	//��������
	for (DWORD i=0;i< dwScnCount;i++)
	{
		memcpy_s(buf,8,(char*)pScn[i].Name,8);
		//�ж��Ƿ�����ͬ������
		if (strcmp(buf,scnName)==0)
		{
			return pScn + i;
		}
	}
	return nullptr;
}



//************************************************************
// ��������: AddSection
// ����˵��: ���һ���µ�����
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/1
// ��	 ��1: char*& pFileBuff �ļ��������׵�ַ
// ��	 ��2: int& fileSize �ļ���С
// ��	 ��3: const char* scnName Ҫ��ӵ�������
// ��	 ��4: int scnSize		 Ҫ��ӵ����δ�С
// �� �� ֵ: void
//************************************************************
void CPeFileOper::AddSection(char*& pFileBuff, int& fileSize, const char* scnName, int scnSize)
{
	//�����ļ�ͷ�����θ���
	GetFileHead(pFileBuff)->NumberOfSections++;
	//���������ε�����ͷ
	IMAGE_SECTION_HEADER* pNewScn = NULL;
	pNewScn = GetLastSection(pFileBuff);
	//���ε�����
	memcpy(pNewScn->Name,scnName,8);
	//���εĴ�С(ʵ�ʴ�С/�����Ĵ�С)
	pNewScn->Misc.VirtualSize = scnSize;
	pNewScn->SizeOfRawData = AlignMent(scnSize,GetOptionHeader(pFileBuff)->FileAlignment);
	//���ε�λ��(RVA/FOA)
	pNewScn->PointerToRawData = AlignMent(fileSize, GetOptionHeader(pFileBuff)->FileAlignment);
	//�����ε��ڴ�ƫ��=��һ�����ε��ڴ�ƫ��+��һ�����εĴ�С(�ڴ�����Ĵ�С)------------------

	pNewScn->VirtualAddress = (pNewScn - 1)->VirtualAddress + AlignMent((pNewScn-1)->SizeOfRawData,GetOptionHeader(pFileBuff)->SectionAlignment);
	
	//���ε�����
	pNewScn->Characteristics = 0xE00000E0;

	//�޸���չͷ��ӳ���С
	GetOptionHeader(pFileBuff)->SizeOfImage =pNewScn->VirtualAddress + pNewScn->SizeOfRawData;

	//�����ļ����ݵĶѿռ��С
	int newSize = pNewScn->PointerToRawData + pNewScn->SizeOfRawData;
	char* pNewBuff = new char[newSize]{0};
	memcpy(pNewBuff,pFileBuff,fileSize);
	//�ͷžɵĻ�����
	delete pFileBuff;

	//���µĻ������׵�ַ�����ļ��Ĵ�С�����β�(�޸�ʵ��)
	fileSize = newSize;
	pFileBuff = pNewBuff;
}


//************************************************************
// ��������: SavePEFile
// ����˵��: ���ļ����浽ָ��·��
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/2
// ��	 ��1: char*& pFileBuff �ļ��������׵�ַ
// ��	 ��2: int& size �ļ���С
// ��	 ��3: _In_ const char*path �ļ���
// �� �� ֵ: BOOL �����ļ��Ƿ�ɹ�
//************************************************************
BOOL CPeFileOper::SavePEFile(_In_ const char* pFileData, _In_ int size, _In_ const char*path)
{
	HANDLE hFile = CreateFileA( 
		path,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile==INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}
	DWORD dwWrite = 0;
	//������д�뵽�ļ�
	WriteFile(hFile,pFileData,size,&dwWrite,NULL);
	//�ر��ļ����
	CloseHandle(hFile);
	return dwWrite == size;

}


//************************************************************
// ��������: LoadStub
// ����˵��: ����stub.dll
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/2
// ��	 ��1: StubInfo* pStub ����stub��Ϣ�Ľṹ��
// �� �� ֵ: void
//************************************************************
void CPeFileOper::LoadStub(StubInfo* pStub)
{
	//ͨ��LoadLibarary����stub.dll
	pStub->dllbase = (char*)LoadLibraryEx(L"stub.dll",NULL,DONT_RESOLVE_DLL_REFERENCES);

	//��ȡDll��������
	pStub->pfnStart = (DWORD)GetProcAddress((HMODULE)pStub->dllbase,"Start");
	//��ȡStubConf�ṹ���ַ
	pStub->pStubConf = (StubConf*)GetProcAddress((HMODULE)pStub->dllbase,"g_conf");
}


//************************************************************
// ��������: Encrypt
// ����˵��: ����Ŀ�����Ĵ����
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/2
// ��	 ��: _In_ const char* pFileData Ŀ���ļ��������׵�ַ
// ��	 ��: _In_  StubInfo* pStub ����stub������Ϣ�Ľṹ��
// �� �� ֵ: void
//************************************************************
void CPeFileOper::Encrypt(_In_ char* pFileData,_In_  StubInfo pStub)
{
	//��������ݻ��������׵�ַ
	 BYTE* pTargetText = GetSection(pFileData, ".text")->PointerToRawData + (BYTE*)pFileData;

	 //����δ�С
	 DWORD dwTargetTextSize = GetSection(pFileData,".text")->Misc.VirtualSize;
	 //���ܴ����
	 for (DWORD i = 0; i < dwTargetTextSize; i++)
	 {
		 pTargetText[i] ^= 0x15;
	 }
	 //�������ο�ʼ��RVA�ʹ�С �Լ�������Կ
	 pStub.pStubConf->textScnRVA = GetSection(pFileData,".text")->VirtualAddress;
	 pStub.pStubConf->textScnSize = dwTargetTextSize;
	 pStub.pStubConf->key = 0x15;
}



//************************************************************
// ��������: FixStubRelocation
// ����˵��: �޸�stub.dll���ض�λ��
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/3
// ��	 ��: DWORD stubDllbase stub.dll�Ļ�ַ
// ��	 ��: DWORD stubTextRva stub.dll�Ĵ����RVA
// ��	 ��: DWORD targetDllbase Ŀ���ļ���Ĭ�ϼ��ػ�ַ
// ��	 ��: DWORD targetNewScnRva Ŀ���ļ������ε�RVA
// �� �� ֵ: void
//************************************************************
 void CPeFileOper::FixStubRelocation(DWORD stubDllbase, DWORD stubTextRva, DWORD targetDllbase, DWORD targetNewScnRva)
 {
	//�ҵ�stub.dll���ض�λ��
	 DWORD dwRelRva = GetOptionHeader((char*)stubDllbase)->DataDirectory[5].VirtualAddress;
	 IMAGE_BASE_RELOCATION* pRel = (IMAGE_BASE_RELOCATION*)(dwRelRva+stubDllbase);
	
	 //�����ض�λ��
	 while (pRel->SizeOfBlock)
	 {
		 struct TypeOffset
		 {
			 WORD offset : 12;
			 WORD type : 4;

		 };
		 TypeOffset* pTypeOffset = (TypeOffset*)(pRel + 1);
		 DWORD dwCount = (pRel->SizeOfBlock-8)/2;	//��Ҫ�ض�λ������
		 for (DWORD i = 0; i < dwCount; i++)
		 {
			 if (pTypeOffset[i].type!=3)
			 {
				 continue;
			 }
			 //��Ҫ�ض�λ�ĵ�ַ
			 DWORD* pFixAddr = (DWORD*)(pRel->VirtualAddress + pTypeOffset[i].offset + stubDllbase);

			 DWORD dwOld;
			 //�޸�����Ϊ��д
			 VirtualProtect(pFixAddr,4,PAGE_READWRITE,&dwOld);
			 //ȥ��dll��ǰ���ػ�ַ
			 *pFixAddr -= stubDllbase;
			 //ȥ��Ĭ�ϵĶ���RVA
			 *pFixAddr -= stubTextRva;
			 //����Ŀ���ļ��ļ��ػ�ַ
			 *pFixAddr += targetDllbase;
			 //���������εĶ���RVA
			 *pFixAddr += targetNewScnRva;
			 //�������޸Ļ�ȥ
			 VirtualProtect(pFixAddr, 4, dwOld, &dwOld);
		 }
		 //�л�����һ���ض�λ��
		 pRel = (IMAGE_BASE_RELOCATION*)((DWORD)pRel + pRel->SizeOfBlock);
	 }

 }




 //************************************************************
// ��������: CompressPE
// ����˵��: ѹ��PE�ļ�
// ��	 ��: GuiShou
// ʱ	 ��: 2018/12/3
// ��	 ��: _In_ char* pFileData Ŀ���ļ��׵�ַ
// ��	 ��: StubInfo &pStubInfo stub���ε���Ϣ
// �� �� ֵ: void
//************************************************************
 void CPeFileOper::CompressPE(_In_ char* pFileData, StubInfo &pStubInfo)
 {
	 PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)pFileData;
	 PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)(pDos->e_lfanew+ pFileData);
	 PIMAGE_SECTION_HEADER pSection = IMAGE_FIRST_SECTION(pNt);

	 //���ڼ�¼ѹ�����εĸ���
	 pStubInfo.PackSectionNumber = 0;
	 //��ȡ�ļ�ͷ�Ĵ�С ����ȡ����Դ�� .tls��֮��ε��ļ����ܴ�С
	 DWORD SecSizeWithoutResAndTls = 0;
	 PIMAGE_SECTION_HEADER pSectionTmp1 = pSection;
	 BOOL isFirstNoEmptySec = TRUE;
	 DWORD dwHeaderSize = 0;

	 //������������
	 for (size_t i = 0; i < pNt->FileHeader.NumberOfSections; i++)
	 {
		 //���ڻ�õ�һ���ǿ����ε��ļ�ƫ�� Ҳ�����ļ�ͷ�Ĵ�С
		 if (isFirstNoEmptySec&&pSectionTmp1->SizeOfRawData!=0)
		 {
			 dwHeaderSize = pSectionTmp1->PointerToRawData;
			 isFirstNoEmptySec = FALSE;
		 }
		 //���ڻ�ȡ��rsrc/tls�ε��ܴ�С
		 
	 }


 }


