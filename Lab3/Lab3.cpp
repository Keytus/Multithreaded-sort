// Lab3.cpp : Defines the entry point for the application.
//

#include "pch.h"
#include "framework.h"
#include "Lab3.h"
#include <string>
#include <strsafe.h>
#include <chrono>

#define MAX_LOADSTRING 100
#define IDM_STARTBUTTON 10001
#define IDM_REFREASHBUTTON 10002

typedef struct ROWINFO {
    int* row;
    int size;
    bool stateInfo;
    std::chrono::duration<double> time;
} ROWINFO, * PROWINFO;

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING] = L"Lab3";        // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
static HWND hColEdit;//поле ввода числа столбов
static HWND hRowEdit;//поле ввода числа строк
static HWND hMatrixEdit;//поле вывода матрицы
static HWND hThreadInfoEdit;//поле инфы о потоках
int** matrix = NULL;//матрица, строки которой будут обрабатываться
int col=0, row=0;
HANDLE*  hThreadArray;
DWORD*   dwThreadIdArray;
PROWINFO* pDataArray;
int TIMER_ID = 1;

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void                AppendText(HWND hEditWnd, std::string str);
void                PrintMatrix();
DWORD WINAPI        MyThreadFunction(LPVOID lpParam);
void                PrintThreadsInfo();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDC_LAB3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB3));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;// создаём экземпляр для обращения к членам класса WNDCLASSEX

    wcex.cbSize = sizeof(WNDCLASSEX);// размер структуры (в байтах)

    wcex.style = CS_HREDRAW | CS_VREDRAW;// стиль класса окошка
    wcex.lpfnWndProc = WndProc;// указатель на функцию обработки сообщений
    wcex.cbClsExtra = 0; // число дополнительных байтов при создании экземпляра приложения
    wcex.cbWndExtra = 0;// число дополнительных байтов в конце структуры
    wcex.hInstance = hInstance;// указатель на строку, содержащую имя меню, применяемого для класса
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB3));// декриптор пиктограммы
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);// дескриптор курсора
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);// дескриптор кисти для закраски фона окна
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LAB3);// указатель на имя меню (у нас его нет)
    wcex.lpszClassName = szWindowClass;// указатель на имя класса
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));// дескриптор маленькой пиктограммы

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(
       szWindowClass,  // имя класса
       szTitle, // заголовок окошка
       WS_OVERLAPPEDWINDOW | WS_VSCROLL, // режимы отображения окошка
       CW_USEDEFAULT, // позиция окошка по оси х
       NULL, // позиция окошка по оси у (у нас х по умолчанию и этот параметр писать не нужно)
       CW_USEDEFAULT, // ширина окошка
       NULL, // высота окошка (раз ширина по умолчанию, то писать не нужно)
       (HWND)NULL, // дескриптор родительского окна
       NULL, // дескриптор меню
       HINSTANCE(hInst), // дескриптор экземпляра приложения
       NULL); // ничего не передаём из WndProc

   if (!hWnd)
   {
       // в случае некорректного создания окошка (неверные параметры и т.п.):
       MessageBox(NULL, L"Не получилось создать окно!", L"Ошибка", MB_OK);
       return FALSE;
   }

   hColEdit = CreateWindow(L"edit", L"2",
       WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT, 50, 50, 50, 20,
       hWnd, 0, hInst, NULL);

   hRowEdit = CreateWindow(L"edit", L"2",
       WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT, 50, 100, 50, 20,
       hWnd, 0, hInst, NULL);

   hMatrixEdit = CreateWindow(L"edit", L"",
       ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL, 250, 50, 300, 200,
       hWnd, 0, hInst, NULL);

   hThreadInfoEdit = CreateWindow(L"edit", L"",
       ES_MULTILINE | WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL, 250, 300, 300, 200,
       hWnd, 0, hInst, NULL);

   HWND hStartButtonWnd = CreateWindow(_T("BUTTON"), _T("Start"), WS_CHILD | WS_VISIBLE,
       50, 150, 75, 20, hWnd, (HMENU)IDM_STARTBUTTON, hInst, NULL);

   HWND hRefreshButtonWnd = CreateWindow(_T("BUTTON"), _T("Refresh"), WS_CHILD | WS_VISIBLE,
       50, 200, 75, 20, hWnd, (HMENU)IDM_REFREASHBUTTON, hInst, NULL);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        SetTimer(hWnd, TIMER_ID, 5000, NULL);
        break;
    }
    case WM_TIMER:
    {
        if (row != 0)
        {
            PrintThreadsInfo();
        }
        InvalidateRect(hWnd, NULL, TRUE);
    }
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            const int MAX_THREADS = 2;
            TCHAR buffer[20];
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_STARTBUTTON:
                GetWindowText(hColEdit, buffer, 20);
                col = _tstoi(buffer);
                GetWindowText(hRowEdit, buffer, 20);
                row = _tstoi(buffer);

                hThreadArray = new HANDLE[row];
                dwThreadIdArray = new DWORD[row];
                pDataArray = new PROWINFO[row];

                matrix = new int*[row];
                for (int i = 0;i < row;i++)
                {
                    matrix[i] = new int[col];
                }

                for (int i = 0,j;i < row;i++)
                {
                    for (j=0;j<col;j++)
                    {
                        matrix[i][j] = rand();
                    }                   
                }

                PrintMatrix();

                for (int i = 0; i < row; i++)
                {
                    pDataArray[i] = (PROWINFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ROWINFO));
                    if (pDataArray[i] == NULL)
                    {
                        // If the array allocation fails, the system is out of memory
                        // so there is no point in trying to print an error message.
                        // Just terminate execution.
                        ExitProcess(2);
                    }
                    pDataArray[i]->row = matrix[i];
                    pDataArray[i]->size = col;
                    pDataArray[i]->stateInfo = TRUE;
                    hThreadArray[i] = CreateThread(
                        NULL,                   // default security attributes
                        0,                      // use default stack size  
                        MyThreadFunction,       // thread function name
                        pDataArray[i],          // argument to thread function 
                        0,                      // use default creation flags 
                        &dwThreadIdArray[i]);   // returns the thread identifier 
                    if (hThreadArray[i] == NULL)
                    {
                        MessageBox(NULL, TEXT("CreateThread error"), TEXT("Error"), MB_OK);
                        ExitProcess(3);
                    }
                }

                PrintThreadsInfo();
                InvalidateRect(hWnd, NULL, TRUE);
                break;
            case IDM_REFREASHBUTTON:
                PrintMatrix();
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        for (int i = 0;i < row;i++)
        {
            delete [] matrix[i];
        }
        delete [] matrix;
        delete [] hThreadArray;
        delete [] dwThreadIdArray;
        delete [] pDataArray;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void PrintMatrix()//вывести матрицу
{
    std::string strMatrix;
    for (int i = 0, j;i < row;i++)
    {
        for (j = 0;j < col;j++)
        {
            strMatrix.append(std::to_string(matrix[i][j]));
            strMatrix.append(" ");
        }
        strMatrix.append("\r\n");
    }

    SetWindowText(hMatrixEdit, L"");

    AppendText(hMatrixEdit, strMatrix);
}


void AppendText(HWND hEditWnd, std::string str)//для выведения текста в окно
{
    std::basic_string<TCHAR> converted(str.begin(), str.end());
    LPCTSTR Text = converted.c_str();
    int idx = GetWindowTextLength(hEditWnd);
    SendMessage(hEditWnd, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
    SendMessage(hEditWnd, EM_REPLACESEL, 0, (LPARAM)Text);
}

DWORD WINAPI MyThreadFunction(LPVOID lpParam)//сортировка строки матрицы
{
    PROWINFO matrixRow = (PROWINFO)lpParam;
    int buffer, size = matrixRow->size;

    auto start = std::chrono::system_clock::now();

    for (int i = 0, j;i < size - 1;i++)
    {
        for (j=0;j < size - 1 - i;j++)
        {
            if (matrixRow->row[j] > matrixRow->row[j+1])
            {
                buffer = matrixRow->row[j];
                matrixRow->row[j] = matrixRow->row[j + 1];
                matrixRow->row[j + 1] = buffer;
            }
        }
    }
    matrixRow->stateInfo = FALSE;
    auto end = std::chrono::system_clock::now();
    matrixRow->time = end - start;
    return 0;
}

void PrintThreadsInfo()//вывести информация о состоянии потока и время его завершения
{
    std::string buffer;

    SetWindowText(hThreadInfoEdit, L"");

    for (int i = 0;i < row;i++)
    {
        buffer.append("Thread ");
        buffer.append(std::to_string(i+1));
        if (pDataArray[i]->stateInfo)
        {
            buffer.append("Active");
        }
        else
        {
            buffer.append("Over ");
            buffer.append(std::to_string(pDataArray[i]->time.count()));
            CloseHandle(hThreadArray[i]);
        }
        buffer.append("\r\n");
        AppendText(hThreadInfoEdit, buffer);
        buffer.clear();
    }
}