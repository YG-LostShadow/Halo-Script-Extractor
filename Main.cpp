#include <windows.h>
#include "Main.h"
#include "MapHandler.h"
#include "resource.h"

const char g_szClassName[] = "myWindowClass";
HWND hWnd = NULL;
HICON hIcon;

void ClearEditBox(HWND hwnd)
{
  SetDlgItemText(hwnd, IDC_MAIN_EDIT, L"");
}

void LoadScriptToEditBox(HWND hwnd, HWND hEdit, char* szFileName)
{
  ClearEditBox(hwnd);

  MapHandler Map;
  Map.MapFileName = szFileName;

  char WindowTitle[256] = "";
  sprintf_s(WindowTitle, 256, "Halo Script Extractor - %s", szFileName);
  SetWindowTextA(hWnd, WindowTitle);

  if(!Map.InitMap())
  {
    Map.ThrowError("Could not initialize map.", false);
    Map.UnloadMapFromMemory();
    Map.FinalString.append(Map.s);
  }
  else
    Map.ExtractScript();

  SetWindowTextA(hEdit, Map.FinalString.c_str());
}

void SaveScriptFromEditBox(HWND hEdit, char* szFileName)
{
	HANDLE hFile;

	hFile = CreateFileA(szFileName, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwTextLength = GetWindowTextLengthA(hEdit);

		if(dwTextLength > 0)
		{
			LPSTR pszText;
			DWORD dwBufferSize = dwTextLength + 1;

			pszText = (LPSTR)GlobalAlloc(GPTR, dwBufferSize);

			if(pszText != NULL)
			{
				if(GetWindowTextA(hEdit, pszText, dwBufferSize))
				{
					DWORD dwWritten;
					WriteFile(hFile, pszText, dwTextLength, &dwWritten, NULL);
				}
				GlobalFree(pszText);
			}
		}

		CloseHandle(hFile);
	}
}

void DoFileOpen(HWND hwnd)
{
	OPENFILENAMEA ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = hwnd;
	ofn.lpstrFilter = "Halo Map Files (*.map)\0*.map\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile   = szFileName;
	ofn.nMaxFile    = MAX_PATH;
	ofn.Flags       = OFN_EXPLORER | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = "map";

	if(GetOpenFileNameA(&ofn))
		LoadScriptToEditBox(hwnd, GetDlgItem(hwnd, IDC_MAIN_EDIT), szFileName);
}

void DoFileSave(HWND hwnd)
{
	OPENFILENAMEA ofn;
	char szFileName[MAX_PATH] = "";

	ZeroMemory(&ofn, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner   = hwnd;
	ofn.lpstrFilter = "Halo Script Files (*.hsc)\0*.hsc\0All Files (*.*)\0*.*\0";
	ofn.lpstrFile   = szFileName;
	ofn.nMaxFile    = MAX_PATH;
	ofn.lpstrDefExt = "hsc";
	ofn.Flags       = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if(GetSaveFileNameA(&ofn))
		SaveScriptFromEditBox(GetDlgItem(hwnd, IDC_MAIN_EDIT), szFileName);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg)
	{
    // doesn't work.
    case WM_DROPFILES:
    {
      HDROP hDrop = (HDROP)wParam;
	    char szFileName[MAX_PATH] = "";
      int nFilesDropped = DragQueryFileA(hDrop, 1, szFileName, MAX_PATH);

      if(nFilesDropped)
        LoadScriptToEditBox(hwnd, GetDlgItem(hwnd, IDC_MAIN_EDIT), szFileName);

      DragFinish (hDrop);
    }
    break;
		case WM_CREATE:
		{
      HWND hEdit;
			HFONT hfDefault;

			if((hEdit = CreateWindowEx(WS_EX_CLIENTEDGE|WS_EX_ACCEPTFILES, L"EDIT", L"", 
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL, 
				0, 0, 100, 100, hwnd, (HMENU)IDC_MAIN_EDIT, GetModuleHandle(NULL), NULL)) == NULL)
				MessageBox(hwnd, L"Could not create edit box.", L"Error", MB_OK | MB_ICONERROR);

			hfDefault = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			SendMessage(hEdit, WM_SETFONT, (WPARAM)hfDefault, MAKELPARAM(FALSE, 0));
		}
		break;
		case WM_SIZE:
		{
			HWND hEdit;
			RECT rcClient;

			GetClientRect(hwnd, &rcClient);

			hEdit = GetDlgItem(hwnd, IDC_MAIN_EDIT);
			SetWindowPos(hEdit, NULL, 0, 0, rcClient.right, rcClient.bottom, SWP_NOZORDER);
		}
		break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
				case ID_FILE_EXIT:
					PostMessage(hwnd, WM_CLOSE, 0, 0);
				break;
				case ID_FILE_OPEN:
					DoFileOpen(hwnd);
				break;
				case ID_FILE_SAVEAS:
					DoFileSave(hwnd);
				break;
        case ID_FILE_ABOUT:
          MessageBox(hwnd, L"By: Skarma\n\nCredits\n\nCrAsHOvErRide\ntheForger\nKornman00\nChoking Victim", L"About HSE", MB_OK);
        break;
			}
		break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	wc.cbSize		     = sizeof(WNDCLASSEX);
	wc.style		     = 0;
	wc.lpfnWndProc	 = WndProc;
	wc.cbClsExtra	   = 0;
	wc.cbWndExtra	   = 0;
	wc.hInstance	   = hInstance;
	wc.hIcon		     = hIcon = LoadIcon(hInstance, L"MAIN_ICON");
	wc.hCursor		   = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName  = MAKEINTRESOURCE(IDR_MAINMENU);
	wc.lpszClassName = (LPCWSTR)g_szClassName;
	wc.hIconSm		   = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	if((hwnd = CreateWindowEx(0, (LPCWSTR)g_szClassName, L"Halo Script Extractor", 
    WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 480, 320, NULL, NULL, hInstance, NULL)) == NULL)
	{
		MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	hWnd = hwnd;

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while(GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}