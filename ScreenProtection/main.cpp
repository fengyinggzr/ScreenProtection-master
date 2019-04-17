#include <windows.h>
#include <gdiplus.h>
#include <SDKDDKVer.h>
#include "ScreenProtection.h"

#define ID_TIMER    1 
#define SECTION_NAME "config"
#define BACKGROUND_KEY "background"
#define TITLE_KEY "title"
#define DESC_KEY "description"
#define FONT_KEY "font"

// ȥ���������˳�����ʱ�ڿ�
#pragma comment ( linker, "/subsystem:windows /entry:mainCRTStartup" ) 
// ���� GdiPlus
#pragma comment(lib,"gdiplus.lib")


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
TCHAR iniFilePath[MAX_PATH];

PROCESS_INFORMATION pi;

LPTSTR GetArgument(LPTSTR commandStr, int index) 
{
	static TCHAR buffer[MAX_PATH];
	bool quoteFlag = false;
	int commandLineLength = lstrlen(commandStr), current = 0, bufferIndex = 0;
	buffer[0] = '\0';
	for (int i = 0; i < commandLineLength; i++)
	{
		TCHAR ch = commandStr[i];
		if (ch == '\"')
		{
			quoteFlag = !quoteFlag;
		}
		else if (ch == ' ' && !quoteFlag)
		{
			if (index == current)
			{
				break;
			}
			else
			{
				current++;
			}
			// ���������ո�
			while (i<commandLineLength - 1 && commandStr[i+1] == ' ')
			{
				i++;
			}
		}
		else
		{
			if (index == current)
			{
				buffer[bufferIndex++] = commandStr[i];
				buffer[bufferIndex] = '\0';
			}
		}
	}

	return buffer;
}

int main(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("AEPKILL");
	HWND            hwnd;
	MSG            msg;
	WNDCLASS    wndclass;
	LPTSTR commandLine = GetCommandLine();
	LPTSTR arg = GetArgument(commandLine , 1);
	int iniFileLength = 0;
	lstrcat(iniFilePath,GetArgument(commandLine,0));
	iniFileLength = lstrlen(iniFilePath);
	iniFileLength = iniFileLength - 4;
	iniFilePath[iniFileLength] = '\0';
	lstrcat(iniFilePath, TEXT(".ini"));
	if ( arg[0] == '/') 
	{
		if (arg[1] == 'p')
		{
			return 0;
		}
		if (arg[1] == 'c')
		{
			MessageBox(NULL, TEXT("��֧������"), TEXT("Error"), MB_ICONERROR);
		}
	}
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;

	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("�˳������������NT��!"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName, NULL,
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP,
		0, 0,
		GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
		NULL, NULL, hInstance,
		NULL);

	//�����ʾ 
	ShowWindow(hwnd, SW_SHOWMAXIMIZED); 
	UpdateWindow(hwnd);

	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	CreateProcess(TEXT("C:\\ScreenProtection-master\\TestScreen\\TestScreen.exe"),
		NULL,
		NULL,
		NULL,
		FALSE,
		NULL,
		NULL,
		NULL,
		&si,
		&pi);
	//��������� 
	ShowCursor(FALSE); 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	//��ʾ����� 
	ShowCursor(TRUE); 
	return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	using namespace Gdiplus;
	static HDC          hdc;
	//��Ļ�Ŀ�� �߶�. 
	static  int cxScreen, cyScreen; 
	static GdiplusStartupInput gdiplusStartupInput;
	static ULONG_PTR pGdiToken;
	static Graphics* graphics = NULL;
	static Graphics* memGraphics = NULL;
	static Bitmap* bitmap = NULL;
	static Bitmap* background = NULL;
	static ScreenProtection* screenProtection;
	static HWND taskBar = FindWindow(TEXT("Shell_TrayWnd"), NULL);

//	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.wShowWindow = SW_SHOW;
	si.dwFlags = STARTF_USESHOWWINDOW;


	//TCHAR szPath[] = _T("c:\\program files\\internet explorer\\iexplore.exe");
//	LPTSTR szCmdline = _tcsdup(TEXT("C:\\Program Files\\MyApp -L -S"));
	bool ret;
	unsigned long result;
	switch (message)
	{
	case WM_CREATE:
		// ����������
		ShowWindow(taskBar, SW_HIDE);
		// ��ȡ��Ļ���
		cxScreen = GetSystemMetrics(SM_CXSCREEN); 
		cyScreen = GetSystemMetrics(SM_CYSCREEN);
		// ��ȡhdc
		hdc = GetDC(hwnd);
		// ��ʼ�� GdiPlus
		GdiplusStartup(&pGdiToken, &gdiplusStartupInput, NULL);
		// ��ʼ������
		graphics = new Graphics(hdc);
		// ��ʼ���ڴ�λͼ
		bitmap = new Bitmap(cxScreen , cyScreen);
		// ��ʼ���ڴ滭��
		memGraphics = new Graphics(bitmap);
		// ��ʼ��������ͼ��
		screenProtection = new ScreenProtection(cxScreen, cyScreen);
		screenProtection->Init(iniFilePath);
		// ������ʱ��
		SetTimer(hwnd, ID_TIMER, 1000, NULL);
		return 0;

	case WM_TIMER:
		screenProtection->DrawScreenProtection(memGraphics);
		graphics->DrawImage(bitmap,0,0);
		return 0;

	//�����ƺ��� 
	
	case WM_LBUTTONDOWN:
		ret = CreateProcess(TEXT("C:\\ScreenProtection-master\\TestScreen\\TestScreen.exe"),
			NULL,
			NULL,
			NULL,
			TRUE,
			NULL,
			NULL,
			NULL,
			&si,
			&pi);
		//Create("D:\\Relia3Fluoro_v6.47\\Relia3Fluoro.exe", SW_SHOW);

		WaitForSingleObject(pi.hProcess, INFINITE);
		GetExitCodeProcess(pi.hProcess, &result);
		if (result == 1)
		{
			//�°汾�Ѿ����¸�����Ҫ����,�������ֱ���˳�
			return 0;
		}
		else if (result == 2)
		{
			delete graphics;
			delete memGraphics;
			delete bitmap;
			delete screenProtection;

			ShowWindow(taskBar, SW_SHOW);
			GdiplusShutdown(pGdiToken);
			KillTimer(hwnd, ID_TIMER);
			ReleaseDC(hwnd, hdc);
			PostQuitMessage(0);
			return 0;
		}


		return 0;
	case WM_KEYDOWN:
	case WM_DESTROY:
		delete graphics;
		delete memGraphics;
		delete bitmap;
		delete screenProtection;
		
		ShowWindow(taskBar, SW_SHOW);
		GdiplusShutdown(pGdiToken);
		KillTimer(hwnd, ID_TIMER);
		ReleaseDC(hwnd, hdc);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}