#include "framework.h"
#include "resource.h"
#include <stdio.h>

LPCWSTR szTitle = L"Клієнт 2 (Варіант 6)";
LPCWSTR szWindowClass = L"Client2PipeClass";

static HWND hwndEdit;
char szBuf[512];
char mess[2048] = "";
char* m_mess = mess;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLIENT2));
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
    }
    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = { 0 };
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    // 2. ТУТ ПЕРЕВІРИТИ НАЗВУ МЕНЮ
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_CLIENT2);

    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 400, 300, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd) {
        MessageBoxA(NULL, "Помилка створення вікна! Перевір ID меню.", "Помилка", MB_OK | MB_ICONERROR);
        return FALSE;
    }
    ShowWindow(hWnd, nCmdShow); UpdateWindow(hWnd); return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        hwndEdit = CreateWindowA("EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 10, 10, 360, 200, hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);

        // Формування повідомлення Варіанту 6 (Клієнт 2)
        int maxWindows = GetSystemMetrics(SM_CXMAXIMIZED);
        int frameWidth = GetSystemMetrics(SM_CXFRAME);
        HDC hdc = GetDC(hWnd);
        int dpiY = GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(hWnd, hdc);

        sprintf_s(mess, "Дані Клієнта #2 (Варіант 6):\r\n- К-ть відкритих вікон (заглушка): %d\r\n- Ширина вікна додатку: %d\r\n- DPI вертикально: %d\r\n", maxWindows, frameWidth, dpiY);
        SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
    }
    break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
            // 3. ТУТ ПЕРЕВІРИТИ ID КНОПКИ SEND (або впиши число)
        case ID_FILE_SEND:
        {
            DWORD cbWritten;
            cbWritten = (DWORD)SendMessageA(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
            SendMessageA(hwndEdit, WM_GETTEXT, (WPARAM)cbWritten + 1, (LPARAM)szBuf);

            // Підключення до іменованого каналу[cite: 10]
            HANDLE hPipe = CreateFileA("\\\\.\\pipe\\LOCAL\\Lab07", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
            if (hPipe == INVALID_HANDLE_VALUE) {
                MessageBoxA(hWnd, "Не можливо відкрити канал (Сервер не запущено?)", "Pipe", MB_OK);
                break;
            }

            WriteFile(hPipe, szBuf, strlen(szBuf) + 1, &cbWritten, NULL);
            CloseHandle(hPipe);

            sprintf_s(mess, "%s\r\nДані успішно відправлено через Pipe!\r\n", m_mess);
            SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
            break;
        }
        }
    }
    break;

    case WM_DESTROY: PostQuitMessage(0); break;
    default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}