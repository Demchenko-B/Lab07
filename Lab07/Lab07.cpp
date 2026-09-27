#include "framework.h"
#include "resource.h" 
#include <stdio.h>

LPCWSTR szTitle = L"Сервер Pipes";
LPCWSTR szWindowClass = L"ServerPipeClass";

static HWND hwndEdit;
static BOOL bServerRunning = FALSE;
static HANDLE hThread = NULL;

char mess[2048] = "Лабораторна робота - Канали обміну даними (Pipes).\r\n";
char* m_mess = mess;

// Фоновий потік для прослуховування каналу (щоб вікно не зависало)
DWORD WINAPI ServerPipeThread(LPVOID lpParam) {
    while (bServerRunning) {
        // Створення іменованого каналу з правильним префіксом[cite: 10]
        HANDLE hPipe = CreateNamedPipeA("\\\\.\\pipe\\LOCAL\\Lab07",
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            512, 512, 5000, NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(1000);
            continue;
        }

        // Чекаємо на підключення клієнта
        if (ConnectNamedPipe(hPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
            char szBuf[512];
            DWORD cbRead;
            if (ReadFile(hPipe, szBuf, 512, &cbRead, NULL)) {
                sprintf_s(mess, "%s\r\n[Отримано через Pipe]:\r\n%s\r\n", m_mess, szBuf);
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
            }
        }
        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }
    return 0;
}

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    MyRegisterClass(hInstance);
    if (!InitInstance(hInstance, nCmdShow)) return FALSE;


    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB07));

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
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB07);

    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, 450, 400, nullptr, nullptr, hInstance, nullptr);
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
        hwndEdit = CreateWindowA("EDIT", mess, WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL, 10, 10, 400, 300, hWnd, NULL, ((LPCREATESTRUCT)lParam)->hInstance, NULL);
        break;

    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
            // 3. ТУТ ПЕРЕВІРИТИ ID КНОПКИ START (або впиши число)
        case ID_MENU_START:
            if (!bServerRunning) {
                bServerRunning = TRUE;
                hThread = CreateThread(NULL, 0, ServerPipeThread, NULL, 0, NULL);
                sprintf_s(mess, "%sСервер Pipes запущено\r\n", m_mess);
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
            }
            break;

            // 4. ТУТ ПЕРЕВІРИТИ ID КНОПКИ STOP (або впиши число)
        case ID_MENU_STOP:
            if (bServerRunning) {
                bServerRunning = FALSE;
                sprintf_s(mess, "%sСервер Pipes зупинено\r\n", m_mess);
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);

                // Технічне підключення, щоб розблокувати потік очікування
                HANDLE hTemp = CreateFileA("\\\\.\\pipe\\LOCAL\\Lab07", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
                if (hTemp != INVALID_HANDLE_VALUE) CloseHandle(hTemp);
            }
            break;

        case IDM_EXIT: DestroyWindow(hWnd); break;
        default: return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_DESTROY:
        bServerRunning = FALSE;
        PostQuitMessage(0);
        break;
    default: return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
} 