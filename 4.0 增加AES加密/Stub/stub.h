#pragma once
#include <windows.h>

typedef struct _StubConf
{
	DWORD srcOep;		//��ڵ�
	DWORD textScnRVA;	//�����RVA
	DWORD textScnSize;	//����εĴ�С
	unsigned char key[16] = {};//������Կ
}StubConf;
