#pragma once
#include <windows.h>

typedef struct _StubConf
{
	DWORD srcOep;		//��ڵ�
	DWORD textScnRVA;	//�����RVA
	DWORD textScnSize;	//����εĴ�С
	unsigned char key[16] = {};//������Կ
}StubConf;



typedef struct DosStub
{
	DWORD nOldImageBass;//���ӿǳ�������ǰĬ�ϵļ��ػ�ַ
	DWORD nStubTextSectionRva;//���ڿ������text��Rva
	DWORD nStubRelocSectionRva;//�ǵ��ض�λ����text�κϲ����ڱ��ӿǳ����Rva

}DosSub;
