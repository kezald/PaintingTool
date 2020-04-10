#include <Windows.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <crtdbg.h>
#include <time.h>

typedef enum Menus
{
	MENU_SHAPE_HAND_DRAWING = 1000,
	MENU_SHAPE_LINE,
	MENU_SHAPE_RECTANGLE, 
	MENU_SHAPE_ELLIPSE,
	MENU_SHAPE_FLAKE_OFFSET_15,
	MENU_SHAPE_FLAKE_OFFSET_22,
	MENU_SHAPE_FLAKE_OFFSET_35,
	MENU_SHAPE_FLAKE_OFFSET_46,

	MENU_SHAPE_ERASER_SIZE_10,
	MENU_SHAPE_ERASER_SIZE_15,
	MENU_SHAPE_ERASER_SIZE_20,
	MENU_SHAPE_ERASER_SIZE_25,
	MENU_SHAPE_ERASER_SIZE_30,

	MENU_PEN_THICKNESS_1,
	MENU_PEN_THICKNESS_2,
	MENU_PEN_THICKNESS_3,
	MENU_PEN_THICKNESS_4,
	MENU_PEN_THICKNESS_5,
	MENU_PEN_COLOUR,
	MENU_PEN_SOLID,
	MENU_PEN_DASHED,

	MENU_BRUSH_COLOUR,
	MENU_BRUSH_HATCHED_STYLES_BDIAGONAL,
	MENU_BRUSH_HATCHED_STYLES_FDIAGONAL,
	MENU_BRUSH_HATCHED_STYLES_DIAGCROSS,
	MENU_BRUSH_HATCHED_STYLES_VERTICAL,
	MENU_BRUSH_HATCHED_STYLES_HORIZONTAL,
	MENU_BRUSH_HATCHED_STYLES_CROSS,
	MENU_BRUSH_SOLID,

	MENU_OPTIONS_WHITE_BG,
	MENU_OPTIONS_BLACK_BG,
	MENU_OPTIONS_ERASE_AREA,
	MENU_OPTIONS_ENABLE_TIMER,
	MENU_OPTIONS_DISABLE_TIMER
};

//#define MENU_SHAPE_HAND_DRAWING 994
//#define MENU_SHAPE_LINE 995
//#define MENU_SHAPE_RECTANGLE 996
//#define MENU_SHAPE_ELLIPSE 997
//#define MENU_SHAPE_FLAKE_OFFSET_15 998
//#define MENU_SHAPE_FLAKE_OFFSET_22 999
//#define MENU_SHAPE_FLAKE_OFFSET_35 1000
//#define MENU_SHAPE_FLAKE_OFFSET_46 1001
//
//#define MENU_SHAPE_ERASER_SIZE_10 1002
//#define MENU_SHAPE_ERASER_SIZE_15 1003
//#define MENU_SHAPE_ERASER_SIZE_20 1004
//#define MENU_SHAPE_ERASER_SIZE_25 1005
//#define MENU_SHAPE_ERASER_SIZE_30 1006
//
//#define MENU_PEN_THICKNESS_1 1007
//#define MENU_PEN_THICKNESS_2 1008
//#define MENU_PEN_THICKNESS_3 1009
//#define MENU_PEN_THICKNESS_4 1010
//#define MENU_PEN_THICKNESS_5 1011
//#define MENU_PEN_COLOUR 1012
//#define MENU_PEN_SOLID 1013
//#define MENU_PEN_DASHED 1014
//
//#define MENU_BRUSH_COLOUR 1015
//#define MENU_BRUSH_HATCHED_STYLES_BDIAGONAL 1016
//#define MENU_BRUSH_HATCHED_STYLES_FDIAGONAL 1017
//#define MENU_BRUSH_HATCHED_STYLES_DIAGCROSS 1018
//#define MENU_BRUSH_HATCHED_STYLES_VERTICAL 1019
//#define MENU_BRUSH_HATCHED_STYLES_HORIZONTAL 1020
//#define MENU_BRUSH_HATCHED_STYLES_CROSS 1021
//#define MENU_BRUSH_SOLID 1022
//
//#define MENU_OPTIONS_WHITE_BG 1023
//#define MENU_OPTIONS_BLACK_BG 1024
//#define MENU_OPTIONS_ERASE_AREA 1025
//#define MENU_OPTIONS_ENABLE_TIMER 1026
//#define MENU_OPTIONS_DISABLE_TIMER 1027

#define IDT_TIMER1 600

//=========================
//========STATES===========
//=========================
static BOOL timer_draw_dots_enabled = FALSE;
static UINT id_selected_pen_style = MENU_PEN_SOLID;
static UINT id_selected_pen_thickness = MENU_PEN_THICKNESS_1;
static UINT id_selected_brush_style = MENU_BRUSH_SOLID;
static UINT id_selected_drawing_mode = MENU_SHAPE_LINE;
static UINT id_selected_flake_offset = 0;
static UINT id_selected_eraser_size = 0;
static BOOL time_to_draw = FALSE;
static BOOL time_to_erase = FALSE;
static BOOL time_to_display_dynamically = FALSE;

//=============================
//======STUFF FOR DRAWING======
//=============================
static RECT rectangle_to_draw;
static RECT prev_rectangle_to_draw;
static POINTS mouse_down_coord;
static POINTS mouse_move_coord;
static POINTS mouse_up_coord;
static POINTS ptPrevious; //For hand drawing
static int eraser_size = 12;
static int window_bg_brush;
static float theta_flake_offset = 0.0f; // 15, 22, 35, 46 are good values

static struct Pen_Data
{
	int iStyle;
	int thickness;
	COLORREF colour;
} pen_data_drawing = { PS_SOLID, 1, RGB(187, 0, 187) };

static LOGBRUSH brush_drawing = { BS_SOLID, RGB(0, 0, 255), HS_DIAGCROSS };

//====================
// GLOBAL VARIABLES
//====================
static HDC hdcMem = NULL;
static HBITMAP hbSavedDrawing = NULL;

static HDC hdcDoubleBuffer = NULL;
static HBITMAP hbDoubleBuffer = NULL;

//static HDC hdcAntialiasing = NULL;
//static HBITMAP hbAntialiasing = NULL;

//====================
// CONSTANT VARIABLES
//====================
static HMENU hMenuBar;
static SIZE screen_size;

//=========================================================
// FUNCTION PROTOTYPES
//==========================================================
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
COLORREF ShowColourDialog(HWND hwnd);
void WindowPaint(HDC hdc);
void CheckAndUncheckMenus(int id_menu_to_check, UINT* id_menu_to_uncheck);
void hdc_with_eraser_ellipse(HDC hdcToSetUp);
void compute_rect_to_draw(RECT* resulting_rect, POINTS first_point, POINTS second_point);
void bitmap_transfer(HDC dest, HDC source, SIZE* rectSize, HWND window);
void TrackMouseLeave(HWND hwnd);

//=====================================================================================
// ENTRY POINT
//=====================================================================================
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prev, LPSTR params, int nCmdShow)
{
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	screen_size.cx = GetSystemMetrics(SM_CXSCREEN);
	screen_size.cy = GetSystemMetrics(SM_CYSCREEN);
	window_bg_brush = BLACK_BRUSH;
	int window_width = 700;
	int window_height = 500;
	srand(time(0));

	const wchar_t g_szClassName[] = L"Painting Tool";

	WNDCLASSEX wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.cbClsExtra = 0;
	wc.cbSize = sizeof(wc);
	wc.cbWndExtra = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(window_bg_brush);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wc.hInstance = hInstance;
	wc.lpfnWndProc = WindowProc;
	wc.lpszClassName = g_szClassName;
	wc.lpszMenuName = NULL;
	wc.style = CS_OWNDC;

	if (!RegisterClassEx(&wc))
	{
		MessageBox(NULL, L"Error Registering Class", L"Error", MB_OK);

		return 0;
	}

	HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,
		g_szClassName,
		L"Window Components",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, window_width, window_height,
		NULL, NULL, hInstance, NULL);

	if (hwnd == 0)
	{
		MessageBox(NULL, L"Error Creating Window", L"Error", MB_OK);

		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	//====HBITMAPS and HDC====
	HDC hdcWindow = GetDC(hwnd);

	hdcMem = CreateCompatibleDC(hdcWindow); //We will use hMemDC to save window surface to the bitmap.
	hbSavedDrawing = CreateCompatibleBitmap(hdcWindow, screen_size.cx, screen_size.cy);
	HBITMAP hbmOld = SelectObject(hdcMem, hbSavedDrawing); //Select the bitmap to use by hMemDC.
	DeleteObject(hbmOld);
	ReleaseDC(hwnd, hdcWindow); //hdcWindow is no longer needed

	hdcDoubleBuffer = CreateCompatibleDC(hdcMem);
	hbDoubleBuffer = CreateCompatibleBitmap(hdcMem, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
	HBITMAP hb_double_buffer_old = SelectObject(hdcDoubleBuffer, hbDoubleBuffer);
	DeleteObject(hb_double_buffer_old);

	//hdcAntialiasing = CreateCompatibleDC(hdcMem);
	//hbAntialiasing = CreateCompatibleBitmap(hdcMem, GetSystemMetrics(SM_CXSCREEN) * 2, GetSystemMetrics(SM_CYSCREEN) * 2);
	//HBITMAP hb_antialiasing_old = SelectObject(hdcAntialiasing, hbAntialiasing);
	//DeleteObject(hb_antialiasing_old);
	//=========================

	MSG message;
	while (GetMessage(&message, NULL, 0, 0) > 0)
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	DWORD GDI_obj_count = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
	DWORD GDI_peak_obj_count = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS_PEAK);
	DWORD User_obj_count = GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);
	DWORD User_peak_obj_count = GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS_PEAK);

	return message.wParam;
}

//=====================================================================================
// WINDOW PROCEDURE EVENT CALLBACK
//=====================================================================================
LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL tracking_mouse_leave = FALSE;

	switch (message)
	{
	case WM_SIZE:
	{
		time_to_draw = FALSE;
		time_to_display_dynamically = FALSE;

		//Restore the whole picture
		HDC hdcWin = GetDC(hwnd);
		bitmap_transfer(hdcWin, hdcMem, NULL, hwnd);
		ReleaseDC(hwnd, hdcWin);
	}
	break;
	case WM_MOUSELEAVE:
	{
		if (id_selected_eraser_size != 0)
		{
			HDC hdcWin = GetDC(hwnd);
			bitmap_transfer(hdcWin, hdcMem, NULL, hwnd);
			ReleaseDC(hwnd, hdcWin);
		}

		tracking_mouse_leave = FALSE;
	}
	break;
	case WM_LBUTTONUP:
	{
		mouse_up_coord = MAKEPOINTS(lParam);

		time_to_erase = FALSE;
		time_to_draw = FALSE;
		time_to_display_dynamically = FALSE;

		if (id_selected_eraser_size == 0 && id_selected_flake_offset == 0)
		{
			//Save the last result to bitmap
			HDC hdcWin = GetDC(hwnd);
			bitmap_transfer(hdcMem, hdcWin, NULL, hwnd);
			ReleaseDC(hwnd, hdcWin);

			//HDC hdcWin = GetDC(hwnd);

			//RECT client_rect;
			//GetClientRect(hwnd, &client_rect);

			//StretchBlt(hdcAntialiasing, 0, 0, screen_size.cx * 2, screen_size.cy * 2, 
			//	hdcWin, 0, 0, client_rect.right, client_rect.bottom, SRCCOPY);

			//int prevMod = SetStretchBltMode(hdcWin, HALFTONE);
			//StretchBlt(hdcWin, 0, 0, client_rect.right, client_rect.bottom,
			//		   hdcAntialiasing, 0, 0, screen_size.cx * 2, screen_size.cy * 2, SRCCOPY);
			//SetStretchBltMode(hdcWin, prevMod);

			//ReleaseDC(hwnd, hdcWin);
		}
	}
	break;
	case WM_MOUSEMOVE:
	{
		if (!tracking_mouse_leave)
		{
			TrackMouseLeave(hwnd);
			tracking_mouse_leave = TRUE;
		}

		mouse_move_coord = MAKEPOINTS(lParam);

		if (time_to_draw || time_to_display_dynamically)
		{
			if (id_selected_drawing_mode == MENU_SHAPE_LINE || id_selected_drawing_mode == MENU_SHAPE_ELLIPSE || id_selected_drawing_mode == MENU_SHAPE_RECTANGLE)
			{
				compute_rect_to_draw(&rectangle_to_draw, mouse_down_coord, mouse_move_coord);
				InvalidateRect(hwnd, NULL, FALSE);
			}
			else if (id_selected_drawing_mode == MENU_SHAPE_HAND_DRAWING)
			{
				InvalidateRect(hwnd, NULL, FALSE);
			}
		}
		else if (id_selected_eraser_size != 0)
		{
			InvalidateRect(hwnd, NULL, FALSE); //Invalidate Area for erasing stuff
		}
	}
	break;
	case WM_LBUTTONDOWN:
	{
		mouse_down_coord = MAKEPOINTS(lParam);

		if (id_selected_drawing_mode == MENU_SHAPE_LINE || id_selected_drawing_mode == MENU_SHAPE_ELLIPSE || id_selected_drawing_mode == MENU_SHAPE_RECTANGLE)
		{
			time_to_display_dynamically = TRUE;
		}
		else if (id_selected_flake_offset != 0)
		{
			time_to_draw = TRUE;
			InvalidateRect(hwnd, NULL, FALSE); //Draw a flake
		}
		else if (id_selected_eraser_size != 0)
		{
			ptPrevious = mouse_down_coord;
			time_to_erase = TRUE;

			rectangle_to_draw.left = mouse_move_coord.x - eraser_size / 2;
			rectangle_to_draw.top = mouse_move_coord.y - eraser_size / 2;
			rectangle_to_draw.right = mouse_move_coord.x + eraser_size / 2;
			rectangle_to_draw.bottom = mouse_move_coord.y + eraser_size / 2;

			InvalidateRect(hwnd, &rectangle_to_draw, FALSE); //Invalidate Area for erasing stuff
		}
		else if (id_selected_drawing_mode == MENU_SHAPE_HAND_DRAWING)
		{
			ptPrevious = mouse_down_coord;
			time_to_draw = TRUE;

			InvalidateRect(hwnd, NULL, FALSE);
		}
	}
	break;
	case WM_TIMER:
	{
		if (wParam == IDT_TIMER1)
		{
			//Fill background with randomly-colored dots
			HDC hdcWin = GetDC(hwnd);
			RECT rect;
			GetClientRect(hwnd, &rect);
			if (rect.right != 0 && rect.bottom != 0)
			{
				for (int i = 0; i < 30; i++)
				{
					short x = rand() % (rect.right);
					short y = rand() % (rect.bottom);
					unsigned char red = rand() % 256;
					unsigned char green = rand() % 256;
					unsigned char blue = rand() % 256;
					SetPixel(hdcMem, x, y, RGB(red, green, blue));
					SetPixel(hdcWin, x, y, RGB(red, green, blue));
				}
			}
			ReleaseDC(hwnd, hdcWin);
		}
	}
	break;
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == MENU_SHAPE_ERASER_SIZE_10)
		{
			CheckAndUncheckMenus(MENU_SHAPE_ERASER_SIZE_10, &id_selected_drawing_mode);
			id_selected_eraser_size = MENU_SHAPE_ERASER_SIZE_10;
			eraser_size = 10;
			id_selected_flake_offset = 0;
		}
		if (LOWORD(wParam) == MENU_SHAPE_ERASER_SIZE_15)
		{
			CheckAndUncheckMenus(MENU_SHAPE_ERASER_SIZE_15, &id_selected_drawing_mode);
			id_selected_eraser_size = MENU_SHAPE_ERASER_SIZE_15;
			eraser_size = 15;
			id_selected_flake_offset = 0;
		}
		if (LOWORD(wParam) == MENU_SHAPE_ERASER_SIZE_20)
		{
			CheckAndUncheckMenus(MENU_SHAPE_ERASER_SIZE_20, &id_selected_drawing_mode);
			id_selected_eraser_size = MENU_SHAPE_ERASER_SIZE_20;
			eraser_size = 20;
			id_selected_flake_offset = 0;
		}
		if (LOWORD(wParam) == MENU_SHAPE_ERASER_SIZE_25)
		{
			CheckAndUncheckMenus(MENU_SHAPE_ERASER_SIZE_25, &id_selected_drawing_mode);
			id_selected_eraser_size = MENU_SHAPE_ERASER_SIZE_25;
			eraser_size = 25;
			id_selected_flake_offset = 0;
		}
		if (LOWORD(wParam) == MENU_SHAPE_ERASER_SIZE_30)
		{
			CheckAndUncheckMenus(MENU_SHAPE_ERASER_SIZE_30, &id_selected_drawing_mode);
			id_selected_eraser_size = MENU_SHAPE_ERASER_SIZE_30;
			eraser_size = 30;
			id_selected_flake_offset = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_HAND_DRAWING)
		{
			CheckAndUncheckMenus(MENU_SHAPE_HAND_DRAWING, &id_selected_drawing_mode);
			id_selected_flake_offset = 0;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_LINE)
		{
			CheckAndUncheckMenus(MENU_SHAPE_LINE, &id_selected_drawing_mode);
			id_selected_flake_offset = 0;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_RECTANGLE)
		{
			CheckAndUncheckMenus(MENU_SHAPE_RECTANGLE, &id_selected_drawing_mode);
			id_selected_flake_offset = 0;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_ELLIPSE)
		{
			CheckAndUncheckMenus(MENU_SHAPE_ELLIPSE, &id_selected_drawing_mode);
			id_selected_flake_offset = 0;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_FLAKE_OFFSET_15)
		{
			CheckAndUncheckMenus(MENU_SHAPE_FLAKE_OFFSET_15, &id_selected_drawing_mode);
			id_selected_flake_offset = MENU_SHAPE_FLAKE_OFFSET_15;
			theta_flake_offset = 15.0f;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_FLAKE_OFFSET_22)
		{
			CheckAndUncheckMenus(MENU_SHAPE_FLAKE_OFFSET_22, &id_selected_drawing_mode);
			id_selected_flake_offset = MENU_SHAPE_FLAKE_OFFSET_22;
			theta_flake_offset = 22.0f;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_FLAKE_OFFSET_35)
		{
			CheckAndUncheckMenus(MENU_SHAPE_FLAKE_OFFSET_35, &id_selected_drawing_mode);
			id_selected_flake_offset = MENU_SHAPE_FLAKE_OFFSET_35;
			theta_flake_offset = 35.0f;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_SHAPE_FLAKE_OFFSET_46)
		{
			CheckAndUncheckMenus(MENU_SHAPE_FLAKE_OFFSET_46, &id_selected_drawing_mode);
			id_selected_flake_offset = MENU_SHAPE_FLAKE_OFFSET_46;
			theta_flake_offset = 46.0f;
			id_selected_eraser_size = 0;
		}
		else if (LOWORD(wParam) == MENU_PEN_THICKNESS_1)
		{
			CheckAndUncheckMenus(MENU_PEN_THICKNESS_1, &id_selected_pen_thickness);
			pen_data_drawing.thickness = 1;
		}
		else if (LOWORD(wParam) == MENU_PEN_THICKNESS_2)
		{
			CheckAndUncheckMenus(MENU_PEN_THICKNESS_2, &id_selected_pen_thickness);
			pen_data_drawing.thickness = 2;
		}
		else if (LOWORD(wParam) == MENU_PEN_THICKNESS_3)
		{
			CheckAndUncheckMenus(MENU_PEN_THICKNESS_3, &id_selected_pen_thickness);
			pen_data_drawing.thickness = 3;
		}
		else if (LOWORD(wParam) == MENU_PEN_THICKNESS_4)
		{
			CheckAndUncheckMenus(MENU_PEN_THICKNESS_4, &id_selected_pen_thickness);
			pen_data_drawing.thickness = 4;
		}
		else if (LOWORD(wParam) == MENU_PEN_THICKNESS_5)
		{
			CheckAndUncheckMenus(MENU_PEN_THICKNESS_5, &id_selected_pen_thickness);
			pen_data_drawing.thickness = 5;
		}
		else if (LOWORD(wParam) == MENU_PEN_COLOUR)
		{
			pen_data_drawing.colour = ShowColourDialog(hwnd, pen_data_drawing.colour);
		}
		else if (LOWORD(wParam) == MENU_PEN_SOLID)
		{
			CheckAndUncheckMenus(MENU_PEN_SOLID, &id_selected_pen_style);
			pen_data_drawing.iStyle = PS_SOLID;
		}
		else if (LOWORD(wParam) == MENU_PEN_DASHED)
		{
			CheckAndUncheckMenus(MENU_PEN_DASHED, &id_selected_pen_style);
			pen_data_drawing.iStyle = PS_DASH;
		}
		else if (LOWORD(wParam) == MENU_BRUSH_COLOUR)
		{
			brush_drawing.lbColor = ShowColourDialog(hwnd, brush_drawing.lbColor);
		}
		else if (LOWORD(wParam) == MENU_BRUSH_HATCHED_STYLES_BDIAGONAL)
		{
			CheckAndUncheckMenus(MENU_BRUSH_HATCHED_STYLES_BDIAGONAL, &id_selected_brush_style);
			brush_drawing.lbStyle = BS_HATCHED;
			brush_drawing.lbHatch = HS_BDIAGONAL;
		}
		else if (LOWORD(wParam) == MENU_BRUSH_HATCHED_STYLES_CROSS)
		{
			CheckAndUncheckMenus(MENU_BRUSH_HATCHED_STYLES_CROSS, &id_selected_brush_style);
			brush_drawing.lbStyle = BS_HATCHED;
			brush_drawing.lbHatch = HS_CROSS;
		}
		else if (LOWORD(wParam) == MENU_BRUSH_HATCHED_STYLES_DIAGCROSS)
		{
			CheckAndUncheckMenus(MENU_BRUSH_HATCHED_STYLES_DIAGCROSS, &id_selected_brush_style);
			brush_drawing.lbStyle = BS_HATCHED;
			brush_drawing.lbHatch = HS_DIAGCROSS;
		}
		else if (LOWORD(wParam) == MENU_BRUSH_HATCHED_STYLES_FDIAGONAL)
		{
			CheckAndUncheckMenus(MENU_BRUSH_HATCHED_STYLES_FDIAGONAL, &id_selected_brush_style);
			brush_drawing.lbStyle = BS_HATCHED;
			brush_drawing.lbHatch = HS_FDIAGONAL;
		}
		else if (LOWORD(wParam) == MENU_BRUSH_HATCHED_STYLES_HORIZONTAL)
		{
			CheckAndUncheckMenus(MENU_BRUSH_HATCHED_STYLES_HORIZONTAL, &id_selected_brush_style);
			brush_drawing.lbStyle = BS_HATCHED;
			brush_drawing.lbHatch = HS_HORIZONTAL;
		}
		else if (LOWORD(wParam) == MENU_BRUSH_HATCHED_STYLES_VERTICAL)
		{
			CheckAndUncheckMenus(MENU_BRUSH_HATCHED_STYLES_VERTICAL, &id_selected_brush_style);
			brush_drawing.lbStyle = BS_HATCHED;
			brush_drawing.lbHatch = HS_VERTICAL;
		}
		else if (LOWORD(wParam) == MENU_BRUSH_SOLID)
		{
			CheckAndUncheckMenus(MENU_BRUSH_SOLID, &id_selected_brush_style);
			brush_drawing.lbStyle = BS_SOLID;
		}
		else if (LOWORD(wParam) == MENU_OPTIONS_WHITE_BG)
		{
			window_bg_brush = WHITE_BRUSH;
			EnableMenuItem(hMenuBar, MENU_OPTIONS_WHITE_BG, MF_GRAYED);
			EnableMenuItem(hMenuBar, MENU_OPTIONS_BLACK_BG, MF_ENABLED);
			SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (HBRUSH)GetStockObject(window_bg_brush));

			RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
			FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(window_bg_brush));
			InvalidateRect(hwnd, NULL, TRUE);
		}
		else if (LOWORD(wParam) == MENU_OPTIONS_BLACK_BG)
		{
			window_bg_brush = BLACK_BRUSH;
			EnableMenuItem(hMenuBar, MENU_OPTIONS_WHITE_BG, MF_ENABLED);
			EnableMenuItem(hMenuBar, MENU_OPTIONS_BLACK_BG, MF_GRAYED);
			SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (HBRUSH)GetStockObject(window_bg_brush));

			RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
			FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(window_bg_brush));
			InvalidateRect(hwnd, NULL, TRUE);
		}
		else if (LOWORD(wParam) == MENU_OPTIONS_ERASE_AREA)
		{
			InvalidateRect(hwnd, NULL, TRUE);
			UpdateWindow(hwnd);
			RECT rect = { 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
			FillRect(hdcMem, &rect, (HBRUSH)GetStockObject(window_bg_brush));
		}
		else if (LOWORD(wParam) == MENU_OPTIONS_ENABLE_TIMER)
		{
			EnableMenuItem(hMenuBar, MENU_OPTIONS_ENABLE_TIMER, MF_GRAYED);
			EnableMenuItem(hMenuBar, MENU_OPTIONS_DISABLE_TIMER, MF_ENABLED);
			SetTimer(hwnd, IDT_TIMER1, 100, (TIMERPROC)NULL);
			timer_draw_dots_enabled = TRUE;
		}
		else if (LOWORD(wParam) == MENU_OPTIONS_DISABLE_TIMER)
		{
			EnableMenuItem(hMenuBar, MENU_OPTIONS_DISABLE_TIMER, MF_GRAYED);
			EnableMenuItem(hMenuBar, MENU_OPTIONS_ENABLE_TIMER, MF_ENABLED);
			KillTimer(hwnd, IDT_TIMER1);
			timer_draw_dots_enabled = FALSE;
		}
	}
	break;
	case WM_CREATE:
	{
		hMenuBar = CreateMenu();

		HMENU hFileMenu = CreateMenu();
		HMENU hShapeMenu = CreateMenu();
		HMENU hPenMenu = CreateMenu();
		HMENU hBrushMenu = CreateMenu();
		HMENU hOptionsMenu = CreateMenu();

		//===FOR SHAPE MENU===
		HMENU hShapeEraserSize = CreateMenu();
		HMENU hShapeFlakeOffsets = CreateMenu();
		//====================

		//====FOR PEN MENU====
		HMENU hPenThickness = CreateMenu();
		//====================

		//====FOR BRUSH MENU===
		HMENU hBrushHatchedStyles = CreateMenu();
		//=====================

		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hShapeMenu, L"&Shape");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hPenMenu, L"&Pen");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hBrushMenu, L"&Brush");
		AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hOptionsMenu, L"&More Options");

		AppendMenu(hShapeMenu, MF_POPUP, (UINT_PTR)hShapeEraserSize, L"&Eraser");
		AppendMenu(hShapeMenu, MF_UNCHECKED, MENU_SHAPE_HAND_DRAWING, L"&Hand Drawing");
		AppendMenu(hShapeMenu, MF_CHECKED, MENU_SHAPE_LINE, L"&Line");
		AppendMenu(hShapeMenu, MF_UNCHECKED, MENU_SHAPE_RECTANGLE, L"&Rectangle");
		AppendMenu(hShapeMenu, MF_UNCHECKED, MENU_SHAPE_ELLIPSE, L"&Ellipse");
		AppendMenu(hShapeMenu, MF_POPUP, (UINT_PTR)hShapeFlakeOffsets, L"&Flake");

		AppendMenu(hPenMenu, MF_POPUP, (UINT_PTR)hPenThickness, L"&Thickness");
		AppendMenu(hPenMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hPenMenu, MF_STRING, MENU_PEN_COLOUR, L"&Colour");
		AppendMenu(hPenMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hPenMenu, MF_CHECKED, MENU_PEN_SOLID, L"&Solid");
		AppendMenu(hPenMenu, MF_UNCHECKED, MENU_PEN_DASHED, L"&Dashed");

		AppendMenu(hBrushMenu, MF_POPUP, MENU_BRUSH_COLOUR, L"&Colour");
		AppendMenu(hBrushMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hBrushMenu, MF_POPUP, (UINT_PTR)hBrushHatchedStyles, L"&Hatched Styles");
		AppendMenu(hBrushMenu, MF_STRING | MF_CHECKED, MENU_BRUSH_SOLID, L"&Solid");

		AppendMenu(hOptionsMenu, MF_STRING, MENU_OPTIONS_WHITE_BG, L"&White Background");
		AppendMenu(hOptionsMenu, MF_STRING | MF_GRAYED, MENU_OPTIONS_BLACK_BG, L"&Black Background");
		AppendMenu(hOptionsMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hOptionsMenu, MF_STRING, MENU_OPTIONS_ERASE_AREA, L"&Erase Client Area");
		AppendMenu(hOptionsMenu, MF_SEPARATOR, 0, 0);
		AppendMenu(hOptionsMenu, MF_STRING, MENU_OPTIONS_ENABLE_TIMER, L"&Enable Timer");
		AppendMenu(hOptionsMenu, MF_STRING | MF_GRAYED, MENU_OPTIONS_DISABLE_TIMER, L"&Disable Timer");

		//===SHAPE MENU -> ERASER SIZE and FLAKE OFFSETS===
		AppendMenu(hShapeEraserSize, MF_STRING, MENU_SHAPE_ERASER_SIZE_10, L"&10");
		AppendMenu(hShapeEraserSize, MF_STRING, MENU_SHAPE_ERASER_SIZE_15, L"&15");
		AppendMenu(hShapeEraserSize, MF_STRING, MENU_SHAPE_ERASER_SIZE_20, L"&20");
		AppendMenu(hShapeEraserSize, MF_STRING, MENU_SHAPE_ERASER_SIZE_25, L"&25");
		AppendMenu(hShapeEraserSize, MF_STRING, MENU_SHAPE_ERASER_SIZE_30, L"&30");

		AppendMenu(hShapeFlakeOffsets, MF_STRING | MF_UNCHECKED, MENU_SHAPE_FLAKE_OFFSET_15, L"&Offset 15");
		AppendMenu(hShapeFlakeOffsets, MF_STRING | MF_UNCHECKED, MENU_SHAPE_FLAKE_OFFSET_22, L"&Offset 22");
		AppendMenu(hShapeFlakeOffsets, MF_STRING | MF_UNCHECKED, MENU_SHAPE_FLAKE_OFFSET_35, L"&Offset 35");
		AppendMenu(hShapeFlakeOffsets, MF_STRING | MF_UNCHECKED, MENU_SHAPE_FLAKE_OFFSET_46, L"&Offset 46");
		//====================

		//====PEN MENU -> PEN THICKNESS====
		AppendMenu(hPenThickness, MF_STRING | MF_CHECKED, MENU_PEN_THICKNESS_1, L"&1");
		AppendMenu(hPenThickness, MF_STRING, MENU_PEN_THICKNESS_2, L"&2");
		AppendMenu(hPenThickness, MF_STRING, MENU_PEN_THICKNESS_3, L"&3");
		AppendMenu(hPenThickness, MF_STRING, MENU_PEN_THICKNESS_4, L"&4");
		AppendMenu(hPenThickness, MF_STRING, MENU_PEN_THICKNESS_5, L"&5");
		//====================

		//====BRUSH MENU -> BRUSH STYLES===
		AppendMenu(hBrushHatchedStyles, MF_STRING, MENU_BRUSH_HATCHED_STYLES_BDIAGONAL, L"&BDiagonal");
		AppendMenu(hBrushHatchedStyles, MF_STRING, MENU_BRUSH_HATCHED_STYLES_FDIAGONAL, L"&FDiagonal");
		AppendMenu(hBrushHatchedStyles, MF_STRING, MENU_BRUSH_HATCHED_STYLES_DIAGCROSS, L"&Diagcross");
		AppendMenu(hBrushHatchedStyles, MF_STRING, MENU_BRUSH_HATCHED_STYLES_VERTICAL, L"&Vertical");
		AppendMenu(hBrushHatchedStyles, MF_STRING, MENU_BRUSH_HATCHED_STYLES_HORIZONTAL, L"&Horizontal");
		AppendMenu(hBrushHatchedStyles, MF_STRING, MENU_BRUSH_HATCHED_STYLES_CROSS, L"&Cross");
		//=====================

		SetMenu(hwnd, hMenuBar);
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		HBRUSH hUserBrush = CreateBrushIndirect(&brush_drawing);
		HPEN hUserPen = CreatePen(pen_data_drawing.iStyle, pen_data_drawing.thickness, pen_data_drawing.colour);

		HBRUSH hOldBrush = SelectObject(hdc, hUserBrush);
		HPEN hOldPen = SelectObject(hdc, hUserPen);

		WindowPaint(hdc, hwnd);

		SelectObject(hdc, hOldBrush);
		SelectObject(hdc, hOldPen);
		DeleteObject(hUserBrush);
		DeleteObject(hUserPen);

		EndPaint(hwnd, &ps);
	}
	break;
	case WM_CLOSE:
	{
		DestroyWindow(hwnd);
	}
	break;
	case WM_DESTROY:
	{
		KillTimer(hwnd, IDT_TIMER1);
		DeleteDC(hdcMem);
		DeleteObject(hbSavedDrawing);

		DeleteDC(hdcDoubleBuffer);
		DeleteObject(hbDoubleBuffer);

		//DeleteDC(hdcAntialiasing);
		//DeleteObject(hbAntialiasing);

		PostQuitMessage(0);
	}
	break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}

	return 0;
}


//=====================================================================================
// PAINTING FUNCTION
//=====================================================================================
void WindowPaint(HDC hdc, HWND hwnd)
{
	if (time_to_display_dynamically)
	{
		HBRUSH hNewBrushBuffering = CreateBrushIndirect(&brush_drawing);
		HPEN hNewPenBuffering = CreatePen(pen_data_drawing.iStyle, pen_data_drawing.thickness, pen_data_drawing.colour);
		HBRUSH hOldBrushBuffering = SelectObject(hdcDoubleBuffer, hNewBrushBuffering);
		HPEN hOldPenBuffering = SelectObject(hdcDoubleBuffer, hNewPenBuffering);

		bitmap_transfer(hdcDoubleBuffer, hdcMem, &screen_size, NULL); //Buffering

		if (id_selected_drawing_mode == MENU_SHAPE_LINE)
		{
			MoveToEx(hdcDoubleBuffer, mouse_down_coord.x, mouse_down_coord.y, NULL);
			LineTo(hdcDoubleBuffer, mouse_move_coord.x, mouse_move_coord.y);
		}
		else if (id_selected_drawing_mode == MENU_SHAPE_RECTANGLE)
		{
			Rectangle(hdcDoubleBuffer, rectangle_to_draw.left, rectangle_to_draw.top,
				rectangle_to_draw.right, rectangle_to_draw.bottom);
		}
		else if (id_selected_drawing_mode == MENU_SHAPE_ELLIPSE)
		{
			Ellipse(hdcDoubleBuffer, rectangle_to_draw.left, rectangle_to_draw.top,
				rectangle_to_draw.right, rectangle_to_draw.bottom);
		}

		bitmap_transfer(hdc, hdcDoubleBuffer, NULL, hwnd); //Debuffering

		SelectObject(hdcDoubleBuffer, hOldBrushBuffering);
		SelectObject(hdcDoubleBuffer, hOldPenBuffering);

		DeleteObject(hNewBrushBuffering);
		DeleteObject(hNewPenBuffering);
	}
	else if (id_selected_drawing_mode == MENU_SHAPE_HAND_DRAWING)
	{
		MoveToEx(hdc, ptPrevious.x, ptPrevious.y, NULL);
		LineTo(hdc, mouse_move_coord.x, mouse_move_coord.y);
		ptPrevious = mouse_move_coord;
	}
	else if (id_selected_eraser_size != 0)
	{
		if (time_to_erase)
		{
			HDC hdcWin = GetDC(hwnd);

			HPEN hNewEraserPenWin;
			HPEN hNewEraserPenMem;

			if (window_bg_brush == BLACK_BRUSH)
			{
				hNewEraserPenWin = CreatePen(PS_SOLID, eraser_size, RGB(0, 0, 0));
				hNewEraserPenMem = CreatePen(PS_SOLID, eraser_size, RGB(0, 0, 0));
			}
			else if (window_bg_brush == WHITE_BRUSH)
			{
				hNewEraserPenWin = CreatePen(PS_SOLID, eraser_size, RGB(255, 255, 255));
				hNewEraserPenMem = CreatePen(PS_SOLID, eraser_size, RGB(255, 255, 255));
			}

			HPEN hOldEraserPenWin = SelectObject(hdcWin, hNewEraserPenWin);
			HPEN hOldEraserPenMem = SelectObject(hdcMem, hNewEraserPenMem);

			MoveToEx(hdcWin, ptPrevious.x, ptPrevious.y, NULL);
			MoveToEx(hdcMem, ptPrevious.x, ptPrevious.y, NULL);
			LineTo(hdcWin, mouse_move_coord.x, mouse_move_coord.y);
			LineTo(hdcMem, mouse_move_coord.x, mouse_move_coord.y);
			ptPrevious = mouse_move_coord;

			SelectObject(hdcWin, hOldEraserPenWin);
			SelectObject(hdcMem, hOldEraserPenMem);

			//DeleteObject(hNewEraserPenWin);
			DeleteObject(hNewEraserPenMem);
			ReleaseDC(hwnd, hdcWin);
		}

		//====Remove last drawn ellipse====
		bitmap_transfer(hdcDoubleBuffer, hdcMem, NULL, hwnd);

		//====Draw an eraser ellipse to the buffer====
		hdc_with_eraser_ellipse(hdcDoubleBuffer);

		rectangle_to_draw.left = mouse_move_coord.x - eraser_size / 2;
		rectangle_to_draw.top = mouse_move_coord.y - eraser_size / 2;
		rectangle_to_draw.right = mouse_move_coord.x + eraser_size / 2;
		rectangle_to_draw.bottom = mouse_move_coord.y + eraser_size / 2;

		Ellipse(hdcDoubleBuffer, rectangle_to_draw.left, rectangle_to_draw.top,
			rectangle_to_draw.right, rectangle_to_draw.bottom);

		//====Draw to the window====
		bitmap_transfer(hdc, hdcDoubleBuffer, NULL, hwnd);
	}
	else if (id_selected_flake_offset != 0 && time_to_draw == TRUE) //Only on mouse down
	{
		time_to_draw = FALSE;

		HBRUSH hNewBrushMem = CreateBrushIndirect(&brush_drawing);
		HPEN hNewPenMem = CreatePen(pen_data_drawing.iStyle, pen_data_drawing.thickness, pen_data_drawing.colour);
		HBRUSH hOldBrushMem = SelectObject(hdcMem, hNewBrushMem);
		HPEN hOldPenMem = SelectObject(hdcMem, hNewPenMem);

		float length = 40; //40
		float theta = 0.0f;
		while (theta < 360)
		{
			MoveToEx(hdc, mouse_down_coord.x, mouse_down_coord.y, NULL);
			MoveToEx(hdcMem, mouse_down_coord.x, mouse_down_coord.y, NULL);

			int xNew = mouse_down_coord.x + length*cos((theta / 180) * M_PI);
			int yNew = mouse_down_coord.y - length*sin((theta / 180) * M_PI);

			LineTo(hdc, xNew, yNew);
			LineTo(hdcMem, xNew, yNew);

			LineTo(hdc, xNew + (length / 2)*cos((theta + theta_flake_offset) / 180 * M_PI), yNew - (length / 2)*sin((theta + theta_flake_offset) / 180 * M_PI));
			LineTo(hdcMem, xNew + (length / 2)*cos((theta + theta_flake_offset) / 180 * M_PI), yNew - (length / 2)*sin((theta + theta_flake_offset) / 180 * M_PI));

			MoveToEx(hdc, xNew, yNew, NULL);
			MoveToEx(hdcMem, xNew, yNew, NULL);

			LineTo(hdc, xNew + (length / 2)*cos((theta - theta_flake_offset) / 180 * M_PI), yNew - (length / 2)*sin((theta - theta_flake_offset) / 180 * M_PI));
			LineTo(hdcMem, xNew + (length / 2)*cos((theta - theta_flake_offset) / 180 * M_PI), yNew - (length / 2)*sin((theta - theta_flake_offset) / 180 * M_PI));

			theta += 15; // 15
		}

		SelectObject(hdcMem, hOldBrushMem);
		SelectObject(hdcMem, hOldPenMem);

		DeleteObject(hNewBrushMem);
		DeleteObject(hNewPenMem);
	}
}

void compute_rect_to_draw(RECT* resulting_rect, POINTS first_point, POINTS second_point)
{
	if (first_point.x < second_point.x)
	{
		resulting_rect->left = first_point.x;
		resulting_rect->right = second_point.x;
	}
	else
	{
		resulting_rect->left = second_point.x;
		resulting_rect->right = first_point.x;
	}

	if (first_point.y < second_point.y)
	{
		resulting_rect->top = first_point.y;
		resulting_rect->bottom = second_point.y;
	}
	else
	{
		resulting_rect->top = second_point.y;
		resulting_rect->bottom = first_point.y;
	}
}

void bitmap_transfer(HDC dest, HDC source, SIZE* rectSize, HWND window)
{
	if (window != NULL)
	{
		RECT clientAreaSize;
		GetClientRect(window, &clientAreaSize);
		BitBlt(dest, 0, 0, clientAreaSize.right, clientAreaSize.bottom, source, 0, 0, SRCCOPY);
	}
	else
	{
		BitBlt(dest, 0, 0, rectSize->cx, rectSize->cy, source, 0, 0, SRCCOPY);
	}
}

void hdc_with_eraser_ellipse(HDC hdcToSetUp)
{
	//Set Erase Rect (borders and fill)
	HBRUSH hNewRectBrush;
	HPEN hNewRectPen;

	if (window_bg_brush == BLACK_BRUSH)
	{
		hNewRectBrush = CreateSolidBrush(RGB(0, 0, 0));
		hNewRectPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
	}
	else if (window_bg_brush == WHITE_BRUSH)
	{
		hNewRectBrush = CreateSolidBrush(RGB(255, 255, 255));
		hNewRectPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
	}

	HBRUSH hOldBrush = SelectObject(hdcToSetUp, hNewRectBrush);
	HPEN hOldPen = SelectObject(hdcToSetUp, hNewRectPen);
	DeleteObject(hOldBrush);
	DeleteObject(hOldPen);
}

COLORREF ShowColourDialog(HWND hwnd, COLORREF original)
{
	CHOOSECOLOR chooser;
	static COLORREF crCustomColours[16];

	ZeroMemory(&chooser, sizeof(chooser));
	chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
	chooser.hwndOwner = hwnd;
	chooser.lpCustColors = (LPDWORD)crCustomColours;
	chooser.lStructSize = sizeof(chooser);
	chooser.rgbResult = original;
	ChooseColor(&chooser);

	return chooser.rgbResult;
}

void TrackMouseLeave(HWND hwnd)
{
	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE; //Type of events to track & trigger.
	tme.hwndTrack = hwnd;
	TrackMouseEvent(&tme);
}

void CheckAndUncheckMenus(int id_menu_to_check, UINT* id_already_checked_menu)
{
	if (id_menu_to_check != *id_already_checked_menu)
	{
		CheckMenuItem(hMenuBar, *id_already_checked_menu, MF_UNCHECKED);
		CheckMenuItem(hMenuBar, id_menu_to_check, MF_CHECKED);
		*id_already_checked_menu = id_menu_to_check;
	}
}