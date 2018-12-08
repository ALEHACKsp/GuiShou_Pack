#pragma once
#include <windows.h>

typedef struct _StubConf
{
	DWORD srcOep;		//��ڵ�
	DWORD textScnRVA;	//�����RVA
	DWORD textScnSize;	//����εĴ�С
	DWORD key;			//������Կ
}StubConf;

//����stub.dll��Ϣ�Ľṹ��
struct StubInfo
{
	char* dllbase;			//stub.dll�ļ��ػ�ַ
	DWORD pfnStart;			//stub.dll(start)���������ĵ�ַ
	StubConf* pStubConf;	//stub.dll(g_conf)����ȫ�ֱ����ĵ�ַ
};