#include <windows.h>
#include <time.h>
#include <stdlib.h>

// Определяем 4 направления
#define UP		1  // Вверх, координата y головы змеи продолжает уменьшаться
#define DOWN	2  // Вниз, координата y головы змеи продолжает увеличиваться
#define LEFT	3  // Слева координата x головы змеи продолжает уменьшаться
#define RIGHT	4  // Справа координата x головы змеи продолжает увеличиваться

#define SNAKEWIDTH	10  // Размер одиночной змеи
#define XWIDTH		50  // Ширина игровой области
#define YHEIGHT		30  // Высота игровой зоны
#define GETSCORE	10	// Счет еды
#define ID_TIMER	1   // ID таймера

static int timechunk = 300;
static int PrevScore = 0;
static int direct = RIGHT;  // Направление (Примечание: направление определяется головой змеи)
static int Score;  // Количество съеденной еды

bool IsOver = false;
bool bEat = false;

struct Snake
{
	Snake() :next(NULL), before(NULL)
	{
		pt.x = 0; pt.y = 0;
	}
	Snake(const POINT& npt, Snake*, Snake*);
	POINT	pt;
	Snake* next, * before;
} snake;  // Определение головы змеи

Snake::Snake(const POINT& npt, Snake* nnext, Snake* nbefore) :
	pt(npt), next(nnext), before(nbefore) {}

Snake* psLast = NULL,* food = NULL;

bool GameClient[XWIDTH][YHEIGHT]; // Определяем логическое значение координат игровой области, указывающее тело змеи в соответствующей точке и наличие еды.

void GameMenu(HWND hwnd){
	HMENU hmenu1;
	hmenu1 = CreateMenu();

	AppendMenu(hmenu1, MF_STRING, 1, L"&Restart");
	AppendMenu(hmenu1, MF_STRING, 2, L"&Pause");
	AppendMenu(hmenu1, MF_STRING, 3, L"&Exit");

	SetMenu(hwnd, hmenu1);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TCHAR		szAppName[] = TEXT("Snake"),
		szWindowName[] = TEXT("Время поиграть!");
	WNDCLASS	wndcls;
	wndcls.cbClsExtra = 0;
	wndcls.cbWndExtra = 0;
	wndcls.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndcls.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wndcls.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wndcls.hInstance = hInstance;
	wndcls.lpfnWndProc = WndProc;
	wndcls.lpszClassName = szAppName;
	wndcls.lpszMenuName = NULL;
	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	RegisterClass(&wndcls);

	HWND hwnd = CreateWindow(szAppName, szWindowName,
		WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, (XWIDTH + 15) * SNAKEWIDTH, (YHEIGHT + 8) * SNAKEWIDTH,
		NULL, NULL, hInstance, NULL);
	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

// Обновление новой еды
void NewFood(HWND hwnd)
{
	food = new Snake();
	SYSTEMTIME st;
	GetLocalTime(&st);
	srand(st.wMilliseconds);
	food->pt.x = rand() % XWIDTH, food->pt.y = rand() % YHEIGHT;
	while (GameClient[food->pt.x][food->pt.y])
	{
		GetLocalTime(&st);
		srand(st.wMilliseconds);
		food->pt.x = rand() % XWIDTH, food->pt.y = rand() % YHEIGHT;
	}
	GameClient[food->pt.x][food->pt.y] = true;
	InvalidateRect(hwnd, NULL, TRUE);
}

// Определяем, есть ли впереди еда
inline bool IsFood()
{
	if (snake.pt.x == food->pt.x && snake.pt.y == food->pt.y)
		return true;
	else
		return false;
}

// Определяем, ударились ли вы об стену или о тело
bool TouchWall()
{
	Snake* temp = psLast;
	// За гранью суждения
	if (snake.pt.x >= XWIDTH ||
		snake.pt.x < 0 ||
		snake.pt.y < 0 ||
		snake.pt.y >= YHEIGHT)
		return true;
	// Столкновение с вашим телом
	while (temp != &snake)
	{
		if (snake.pt.x == temp->pt.x &&
			snake.pt.y == temp->pt.y)
			return true;
		temp = temp->before;
	}
	return false;
}


void Move(HWND hwnd)
{
	int x, y;  // Используется для записи текущего положения головы змеи (когда не движется вперед)
	// Отмечаем, является ли фронт еда, по умолчанию не еда, указывая, что система обычно вперед
	Snake* temp = psLast;

	// Записываем текущие координаты головы змеи
	x = snake.pt.x;
	y = snake.pt.y;
	// Проверяем, подходит ли это место для еды, и перемещаем голову змеи вперед
	switch (direct)
	{
	case UP:
		--snake.pt.y;
		break;
	case DOWN:
		++snake.pt.y;
		break;
	case LEFT:
		--snake.pt.x;
		break;
	case RIGHT:
		++snake.pt.x;
		break;
	default:
		break;
	}
	// Если это не еда, делаем следующее
	if (!IsFood())
	{
		// Последний раздел - не змеиная голова
		if (temp != &snake)
		{
			GameClient[temp->pt.x][temp->pt.y] = false;
			while (temp != snake.next)
			{
				temp->pt.x = temp->before->pt.x;
				temp->pt.y = temp->before->pt.y;
				temp = temp->before;
			}
			temp->pt.x = x;
			temp->pt.y = y;
			GameClient[temp->pt.x][temp->pt.y] = true;
		}
		else  // psLast == & snake, то есть у змей только змеиные головы
			GameClient[x][y] = false;
		if (TouchWall())
			IsOver = true;						// Затем отмечаем конец игры
		else
			GameClient[snake.pt.x][snake.pt.y] = true;	// В противном случае положение головы змеи после движения помечается квадратом
		InvalidateRect(hwnd, NULL, TRUE);		// Обновляем игровую область
	}
	// Если это еда, сделайте следующее
	else
	{
		++Score;
		if (timechunk >= 40) {
			if (Score > PrevScore + 9) {
				timechunk = timechunk - 10;
				SetTimer(hwnd, ID_TIMER, timechunk, NULL);
				PrevScore = Score;
			}
		}
		food->before = psLast;
		psLast->next = food;

		if (temp != &snake)
		{
			food->pt.x = psLast->pt.x;
			food->pt.y = psLast->pt.y;
			while (temp != snake.next)
			{
				temp->pt.x = temp->before->pt.x;
				temp->pt.y = temp->before->pt.y;
				temp = temp->before;
			}
			temp->pt.x = x;
			temp->pt.y = y;
		}
		else  // psLast == & snake, то есть у змей только змеиные головы
		{
			food->pt.x = x; food->pt.y = y;
		}
		psLast = food;
		NewFood(hwnd);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;
	TEXTMETRIC		tm;
	static int		cxChar, cyChar;
	TCHAR			szScore[] = TEXT("Score:"),
		szGameOver[] = TEXT("Game over"),
		szPause[] = TEXT("Pause"),
		szBuffer[20],
		szCounterFileName[] = L"counter.txt",
		* szText = NULL;
	int				x, y;
	static bool		pause = false;

	switch (message)
	{
	case WM_CREATE:
		GameMenu(hwnd);
		direct = RIGHT;   // Игра запускается, направление по умолчанию правильное
		snake.pt.x = XWIDTH / 2;  // По умолчанию отображается в середине игровой области
		snake.pt.y = YHEIGHT / 2;
		psLast = &snake;
		GameClient[snake.pt.x][snake.pt.y] = true;
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cyChar = tm.tmExternalLeading + tm.tmHeight;
		ReleaseDC(hwnd, hdc);
		NewFood(hwnd);
		SetTimer(hwnd, ID_TIMER, timechunk, NULL);
		return 0;

	case WM_TIMER:
		if (pause) return 0;
		Move(hwnd);
		if (IsOver)
		{
			KillTimer(hwnd, ID_TIMER);
			InvalidateRect(hwnd, NULL, TRUE);
		}
		return 0;

	case WM_COMMAND: {
		switch (LOWORD(wParam))
		{
		case 1:
			for (int x = 0; x < XWIDTH; ++x)
				for (int y = 0; y < YHEIGHT; ++y)
					GameClient[x][y] = false;
			PrevScore = 0;
			Score = 0;
			timechunk = 300;
			SetTimer(hwnd, ID_TIMER, timechunk, NULL);
			direct = RIGHT;   // Игра запускается, направление по умолчанию правильное
			snake.pt.x = XWIDTH / 2;  // По умолчанию отображается в середине игровой области
			snake.pt.y = YHEIGHT / 2;
			psLast = &snake;
			IsOver = false;
			pause = false;
			GameClient[snake.pt.x][snake.pt.y] = true;
			NewFood(hwnd);
			break;

		case 2:
			pause = !pause;
			InvalidateRect(hwnd, NULL, TRUE);
			break;

		case 3:
			DestroyWindow(hwnd);
			break;
		}
	}
	case WM_KEYDOWN:
		if (IsOver || pause) return 0;
		switch (wParam)
		{
		case VK_UP:
			if (direct != DOWN)
			{
				direct = UP;
				Move(hwnd);
			}
			break;
		case VK_DOWN:
			if (direct != UP)
			{
				direct = DOWN;
				Move(hwnd);
			}
			break;
		case VK_LEFT:
			if (direct != RIGHT)
			{
				direct = LEFT;
				Move(hwnd);
			}
			break;
		case VK_RIGHT:
			if (direct != LEFT)
			{
				direct = RIGHT;
				Move(hwnd);
			}
			break;
		default:
			break;
		}
		return 0;

	case WM_CHAR:
		if (IsOver && wParam != 'r') return 0;
		switch (wParam)
		{
		case 'p':
		{
			pause = !pause;
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
		case 'r':
			for (int x = 0; x < XWIDTH; ++x)
				for (int y = 0; y < YHEIGHT; ++y)
					GameClient[x][y] = false;
			PrevScore = 0;
			Score = 0;
			timechunk = 300;
			SetTimer(hwnd, ID_TIMER, timechunk, NULL);
			direct = RIGHT;   // Игра запускается, направление по умолчанию правильное
			snake.pt.x = XWIDTH / 2;  // По умолчанию отображается в середине игровой области
			snake.pt.y = YHEIGHT / 2;
			psLast = &snake;
			IsOver = false;
			pause = false;
			GameClient[snake.pt.x][snake.pt.y] = true;
			NewFood(hwnd);
			break;
		}
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);

		SetViewportOrgEx(hdc, SNAKEWIDTH, SNAKEWIDTH, NULL);
		// Рисуем границу игровой области
		MoveToEx(hdc, -1, -1, NULL);
		LineTo(hdc, XWIDTH * SNAKEWIDTH + 1, -1);
		LineTo(hdc, XWIDTH * SNAKEWIDTH + 1, YHEIGHT * SNAKEWIDTH + 1);
		LineTo(hdc, -1, YHEIGHT * SNAKEWIDTH + 1);
		LineTo(hdc, -1, -1);

		TextOut(hdc, (XWIDTH + 1) * SNAKEWIDTH, 0, szScore, lstrlen(szScore));
		TextOut(hdc, (XWIDTH + 1) * SNAKEWIDTH, cyChar,
			szBuffer, wsprintf(szBuffer, TEXT("%4d"), Score));
		if (pause)
			TextOut(hdc, (XWIDTH + 1) * SNAKEWIDTH, 3 * cyChar, szPause, lstrlen(szPause));
		else if (IsOver)
			TextOut(hdc, (XWIDTH + 1) * SNAKEWIDTH, 3 * cyChar, szGameOver, lstrlen(szGameOver));
		SelectObject(hdc, GetStockObject(BLACK_BRUSH));
		for (x = 0; x < XWIDTH; ++x)
		{
			for (y = 0; y < YHEIGHT; ++y)
			{
				if (GameClient[x][y])
					Rectangle(hdc, x * SNAKEWIDTH, y * SNAKEWIDTH,
						(x + 1) * SNAKEWIDTH, (y + 1) * SNAKEWIDTH);
			}
		}
		return 0;

	case WM_DESTROY:
		KillTimer(hwnd, ID_TIMER);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}
