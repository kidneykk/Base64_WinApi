#include <Windows.h>
#include <Richedit.h>
#include <ShObjIdl.h>

#include "Resource.h"
#include "EncodingRoutine.h"
#include "base64.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL LoadTextFileToEdit(HWND hEdit, LPCWSTR pszFileName);

HINSTANCE hInst;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR nCmdLine, int nCmdShow)
{
    LPCWSTR szClassName = L"Main class";
	WNDCLASSEX wndClass;

    wndClass.cbSize = sizeof(wndClass);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = WndProc;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;
    wndClass.hInstance = hInstance;
    wndClass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEFAULT_DOOF));
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hbrBackground = (HBRUSH)(WHITE_BRUSH);
    wndClass.lpszMenuName = MAKEINTRESOURCE(IDM_MENU);
    wndClass.lpszClassName = szClassName;
    wndClass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MICRO_DOOF));

    RegisterClassEx(&wndClass);

    hInst = hInstance;

    HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,
                               szClassName,
                               L"Base64",
                               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
                               CW_USEDEFAULT,
                               CW_USEDEFAULT,
                               480,
                               380,
                               NULL,
                               NULL,
                               hInstance,
                               NULL
                              );

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDA_ACCEL));

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return EXIT_SUCCESS;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static HWND hWndEdit_Encode;
    static HWND hWndEdit_Decode;

    std::wstring input_wstr;
    std::string utf8str;
    std::string b64str;
    std::wstring decoded;

    base64 b64;

    switch (msg) {
    case WM_CREATE:
        LoadLibrary(L"Msftedit.dll");   // for RichEdit
        hWndEdit_Encode = CreateWindowEx(0,
                                         MSFTEDIT_CLASS, NULL,
                                         ES_MULTILINE | WS_VSCROLL |
                                         WS_VISIBLE | WS_CHILD | WS_BORDER | WS_TABSTOP,
                                         6, 5, 444, 120,
                                         hWnd,
                                         (HMENU)IDC_EDIT_ENC,
                                         hInst,
                                         NULL
                                        );

        hWndEdit_Decode = CreateWindowEx(0,
                                         MSFTEDIT_CLASS, NULL,
                                         ES_MULTILINE | ES_READONLY |
                                         WS_VISIBLE | WS_VSCROLL | WS_CHILD | WS_BORDER,
                                         6, 130, 444, 120,
                                         hWnd,
                                         (HMENU)IDC_EDIT_DEC,
                                         hInst,
                                         NULL
                                        );

        CreateWindow(L"BUTTON",
                     L"Encode",
                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                     6, 255, 220, 40,
                     hWnd,
                     (HMENU)IDC_ENCODE,
                     hInst,
                     NULL
                    );

        CreateWindow(L"BUTTON",
                     L"Decode",
                     WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                     230, 255, 220, 40,
                     hWnd,
                     (HMENU)IDC_DECODE,
                     hInst,
                     NULL
                    );

        break;  // WM_CREATE
        
    case WM_COMMAND:
        if (HIWORD(wParam) == BN_CLICKED) {
            switch (LOWORD(wParam)) {
            case IDC_ENCODE:
            {
                input_wstr.resize(GetWindowTextLengthW(hWndEdit_Encode) + 1);

                // Get entered text
                const int wstr_size =
                    GetWindowTextW(hWndEdit_Encode, &input_wstr[0], input_wstr.size());
                input_wstr.resize(wstr_size);
                
                // encoding . . .
                utf8str = utf8_encode(input_wstr);  // wstring -> utf-8 string
                b64str = b64.encode(utf8str);       // utf-8 string -> base64

                // Set encoded text
                SetWindowTextA(hWndEdit_Decode, b64str.data());
            }

                break;  // IDC_ENCODE

            case IDC_DECODE:
            {
                b64str.resize(GetWindowTextLengthW(hWndEdit_Encode) + 1);

                // Get entered text
                const int b64str_size =
                    GetWindowTextA(hWndEdit_Encode, &b64str[0], b64str.size());
                b64str.resize(b64str_size);

                // decoding . . .
                utf8str = b64.decode(b64str);    // decoded str (in utf-8)
                decoded = utf8_decode(utf8str);  // utf-8 -> wstring

                // Set decoded text
                SetWindowTextW(hWndEdit_Decode, decoded.data());
            }

                break;  // IDC_DECODE
            }
        }   // BN_CLICKED

        switch (LOWORD(wParam)) {
        case IDM_OPEN_FILE:
        case IDA_OPEN_FILE:
        {
            HRESULT hr = CoInitializeEx(NULL,
                COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
            
            if (SUCCEEDED(hr)) {
                IFileOpenDialog* pFileOpen;

                hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                    IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

                if (SUCCEEDED(hr)) {
                    hr = pFileOpen->Show(NULL);

                    if (SUCCEEDED(hr)) {
                        IShellItem* pItem;
                        hr = pFileOpen->GetResult(&pItem);

                        if (SUCCEEDED(hr)) {
                            PWSTR pszFilePath;
                            hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                            
                            if (SUCCEEDED(hr)) {
                                // try to load text from file to edit. . .
                                if (!LoadTextFileToEdit(hWndEdit_Encode, pszFilePath))
                                    SetWindowTextA(hWndEdit_Encode, "Unable to open file.");

                                CoTaskMemFree(pszFilePath);
                            }
                            pItem->Release();
                        }
                    }
                    pFileOpen->Release();
                }
                CoUninitialize();
            }
        }

            break;

        case IDM_QUIT:
            //SendMessage(hWnd, WM_CLOSE, 0, 0);
            DestroyWindow(hWnd);
            break;

        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlg);
            break;
        }

        break;  // WM_COMMAND

    case WM_DESTROY:
        PostQuitMessage(EXIT_SUCCESS);
        break;
    
    default:
        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
}

BOOL LoadTextFileToEdit(HWND hEdit, LPCWSTR pszFileName)
{
    HANDLE hFile;
    BOOL bSuccess = FALSE;

    hFile = CreateFile(pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
                        OPEN_EXISTING, 0, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD dwFileSize;
        dwFileSize = GetFileSize(hFile, NULL);

        if (dwFileSize != 0xFFFFFFFF) {
            std::string fileText(dwFileSize, '\0');

            if (fileText.size()) {
                DWORD dwRead;

                if (ReadFile(hFile, &fileText[0], fileText.size(), &dwRead, NULL)) {
                    std::wstring output = utf8_decode(fileText);
                    if (SetWindowTextW(hEdit, output.data()))
                        bSuccess = TRUE;
                }
            }
        }
        CloseHandle(hFile);
    }
    return bSuccess;
}

INT_PTR CALLBACK AboutDlg(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}