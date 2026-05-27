#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
//#include <sys/stat.h>
//#include <sys/types.h>

#include <windows.h>
#include "config.h"

static int MAX_X, MAX_Y;        /* размеры FrameBuffer в символах */

static HANDLE hStdout, hStdin;  /* Переменные под дескрипторы */
static CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

#define FRAM_BUFFER_SIZE    ((MAX_FSIZE+2) * (MAX_FSIZE+2) * 2)
CHAR_INFO FrameBuffer[FRAM_BUFFER_SIZE];

/*******************************************************************************
  void cursor_parameters (DWORD dwSize, BOOL bVisible)

  Функция задавания параметров курсора

  Параметры:
    dwSize - Процент заполнения символьной ячейки курсором.
    Это значение находится в диапазоне от 1 до 100.

    bVisible - Видимость курсора.
    Если курсор виден, то значение этого параметра равно TRUE.
*******************************************************************************/
void cursor_parameters (DWORD dwSize, BOOL bVisible)
{
  CONSOLE_CURSOR_INFO curs = {.dwSize = dwSize, .bVisible = bVisible};
  SetConsoleCursorInfo (hStdout, &curs);
}

/*******************************************************************************
  void cursor_position (SHORT x, SHORT y)

  Функция перемещения курсора в заданные координаты

  Параметры:
    x, y - координаты символа
*******************************************************************************/
void cursor_position (SHORT x, SHORT y)
{
  COORD coord = {.X = x, .Y = y};
  SetConsoleCursorPosition (hStdout, coord);
}

/*******************************************************************************
  void clear (void)

  Функция очистки FrameBuffer.

  Каждый символ в массиве структур становится пробелом,
  а цвет символа - белым (на черном фоне).
*******************************************************************************/
void clear (void)
{
  int i;
  CHAR_INFO *ch;
  for (i = 0; i < (MAX_Y * MAX_X); i++)  {
    ch = &FrameBuffer[i];
    ch->Char.AsciiChar = ' ';
    ch->Attributes = FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE;
  }
}

/*******************************************************************************
  void redraw (void)

  Функция перерисовки консольного окна из FrameBuffer.
*******************************************************************************/
void redraw (void)
{
  COORD size, pos;
  SMALL_RECT rect;
  size.X = MAX_X;
  size.Y = MAX_Y;
  pos.X = 0;
  pos.Y = 0;
  rect.Left = 0;
  rect.Top = 0;
  rect.Right = MAX_X;
  rect.Bottom = MAX_Y;
  WriteConsoleOutput(hStdout, FrameBuffer, size, pos, &rect);
}

/*******************************************************************************
  int kbhit (void)

  Функция считывания нажатия клавиши

  Сначала считывает события, потом сравнивает с нажатием клавиши.
  Если совпадает - выводит номер в шестнадцатеричном коде
  и переводит в десятичный.

  Возвращает:
    код клавиши, если та нажата, или -1 если нет нажатий клавиши.
*******************************************************************************/
int kbhit (void)
{
  DWORD num;
  INPUT_RECORD inr;
  int result = -1;

  /* В переменную num считывается количество событий во входном буфере*/
  if (!GetNumberOfConsoleInputEvents(hStdin, &num))  {
    /* если не удалось получить количество входных событий */
    fprintf (stderr, "cannot get number of events\n");
  }

  /* Если входных событий нет (их количество равно 0) */
  if (num == 0) {
    /* возврат -1 (нет нажатий) */
    return -1;
  }

  /* Считывает событие из входного буфера консоли */
  ReadConsoleInput(hStdin, &inr, 1, &num);

  /* Проверка типа события (Клавиша) */
  if (inr.EventType == KEY_EVENT)  {
    /* Проверка что событие на нажатие клавиши (не на отпускание) */
    if (inr.Event.KeyEvent.bKeyDown)  {
      /* Если клавиша нажата, то присваиваем возвращаемому значению код
       * нажатой клавиши */
      result = inr.Event.KeyEvent.wVirtualKeyCode;
    }
  }
  return result;
}

/*******************************************************************************
  void console_title (const char *title)

  Функция добавления заголовка для окна.

  Параметр title - текст заголовка.
*******************************************************************************/
void console_title (const char *title)
{
  SetConsoleTitle (title);
}

/*******************************************************************************
  static void console_set_mode (void)

  Функция настройки режима работы консоли

  Объединяет исходный режим работы и режим поддержки ANSI-последовательностей
  (специальных наборов символов) для очистки экрана консоли -
  это самый простой способ ее очистить (см. console_clear() ).
*******************************************************************************/
static void console_set_mode (void)
{
  DWORD dwOriginalOutMode = 0;
  /* Если не получен режим работы буфера вывода, то выход из функции */
  if (!GetConsoleMode(hStdout, &dwOriginalOutMode)) {
      return;
  }
  /* Инициализация переменной флагом, который отвечает
   * за обработку ANSI-последовательностей */
  DWORD dwOutMode = dwOriginalOutMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

  /* Если объединение ни к чему не привело, то вывод ошибки */
  if (!SetConsoleMode(hStdout, dwOutMode)) {
    fprintf(stderr, "Failed to set any VT mode, can't do anything here.\n");
    return;
  }
}

/*******************************************************************************
  void console_clear (void)

  Функция очистки экрана консоли

  Для этого курсор устанавливается в начало координат и выводится специальная
  escape-последовательность ESC [ 0 J (заполняет пробелами консоль
                                       от курсора до конца экрана)
*******************************************************************************/
void console_clear (void)
{
  cursor_position(0, 0);
  fprintf(stdout, "\033[0J");
  fflush(stdout);
}

/*******************************************************************************
  void console_resize (int max_x, int max_y)

  Функция, задающая размер окна консоли

  Параметры:
    width  - желаемый размер окна по горизонтали в символах
    height - желаемый размер окна по вертикали в символах
*******************************************************************************/
void console_resize (int width, int height)
{
  /* Если не получены данные об указанном буфере экрана консоли,
   * то вывод ошибки */
  if (!GetConsoleScreenBufferInfo (hStdout, &consoleInfo)) {
    fprintf (stderr, "**Unable to get console buffer info.\n");
    return;
  }

  /* желаемые ширина и высота с небольшим запасом */
  int w = (width + 10);
  int h = (height + 8);

  /* Сравнение текущих размеров окна с желаемыми.
   * Если меньше, то будем пытаться увеличивать*/
  if ((consoleInfo.srWindow.Right < w) ||
      (consoleInfo.srWindow.Bottom < h)) {

    /* Вычисление желаемого размера буфера консоли.
     * Если буфер достаточный, то оставляем как есть */
    int buf_w = (w <= consoleInfo.dwSize.X) ? consoleInfo.dwSize.X : w;
    int buf_h = (h <= consoleInfo.dwSize.Y) ? consoleInfo.dwSize.Y : h;

    /* устанавливаем размер буфера консоли */
    COORD nc = {.X = buf_w, .Y = buf_h};
    SetConsoleScreenBufferSize(hStdout, nc);

    /* желаемый размер окна (почти равен размеру буфера. Ширина на 1 меньше,
     * чтобы не возникло полосы прокрутки */
    int win_w = buf_w - 1;
    int win_h = h;

    /* получить максимально допустимые размеры окна */
    COORD scrsize = GetLargestConsoleWindowSize(hStdout);
    /* ограничить размеры окна при превышении максимально допустимых значений */
    /* если желаемый размер окна больше чем максимально допустимый, то
     * берем максимально допустимый */
    win_w = (win_w > scrsize.X) ? scrsize.X-1 : win_w;
    win_h = (win_h > scrsize.Y) ? scrsize.Y-1 : win_h;

    /* устанавливаем размер окна */
    SMALL_RECT sr = {.Left = 0, .Top = 0, .Right = win_w, .Bottom = win_h};
    SetConsoleWindowInfo(hStdout, TRUE, &sr);
  }
}
/*******************************************************************************
  void console_init (int max_x, int max_y)

  Функция инициализации консоли

  Параметры:
    max_x - размер FrameBuffer по Х в символах[0...(MAX_FSIZE + 2)*2]
    max_y - размер FrameBuffer по Y в символах[0...(MAX_FSIZE + 2)]
*******************************************************************************/
void console_init (int max_x, int max_y)
{

  /* Получение дескриптора вывода консоли */
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  /* Получение дескриптора ввода консоли */
  hStdin  = GetStdHandle(STD_INPUT_HANDLE);

  /* Если не удалось получить дескриптор вывода, то выводится ошибка */
  if (hStdout == INVALID_HANDLE_VALUE)  {
    fprintf (stderr, "**Unable to get stdout handle.\n");
    return;
  }

  /* Если не удалось получить дескриптор ввода, то выводится ошибка */
  if (hStdin == INVALID_HANDLE_VALUE)  {
    fprintf (stderr, "**Unable to get stdin handle.\n");
    return;
  }

  /* Настройка режима работы консоли */
  console_set_mode();
  /* Настройка размера окна консоли, чтобы туда по возможности
   * убиралось игровое поле */
  console_resize (max_x, max_y);

  /* Очистка входного буфера консоли */
  FlushConsoleInputBuffer(hStdin);

  MAX_X = max_x;
  MAX_Y = max_y;
}


/*******************************************************************************
  void console_finish (void)

  Функция отображения курсора

  Вызывается при завершении работы с консолью
*******************************************************************************/
void console_finish (void)
{
  cursor_parameters (75, 1);
}

/*******************************************************************************
  void pset (int x, int y, char ch, WORD attr)

  Функция преобразования координат и заполнения FrameBuffer

  Параметры:
    x, y - координаты символа
    ch - символ
    attr - цвет
*******************************************************************************/
void pset (int x, int y, char ch, WORD attr)
{
  int index = y*MAX_X + x;
  FrameBuffer[index].Char.AsciiChar = ch;
  FrameBuffer[index].Attributes = attr;
}
/*******************************************************************************
  void vline (int x, int y0, int y1, char ch, WORD attr)

  Функция отрисовки вертикальной линии

  Параметры:
    x - координата по горизонтали в символах
    y0, y1 - начальная и конечная координата по вертикали в символах
    ch - символ, которым будет заполнена линия
    attr - цвет символа
*******************************************************************************/
void vline (int x, int y0, int y1, char ch, WORD attr)
{
  int y;
  for (y = y0; y <= y1; y++)  {
    pset (x, y, ch, attr);
  }
}

/*******************************************************************************
  void hline (int y, int x0, int x1, char ch, WORD attr)

  Функция отрисовки горизонтальной линии

  Параметры:
    y - координата по вертикали в символах
    x0, x1 - начальная и конечная координата по горизонтали в символах
    ch - символ, которым будет заполнена линия
    attr - цвет символа
*******************************************************************************/
void hline (int y, int x0, int x1, char ch, WORD attr)
{
  int x;
  for (x = x0; x <= x1; x++)  {
    pset (x, y, ch, attr);
  }
}

/*******************************************************************************
  void box (int x0, int y0, int x1, int y1, WORD attr)

  Функция отрисовки прямоугольника

  Параметры:
    x0, x1 - начальная и конечная координата по X в символах
    y0, y1 - начальная и конечная координата по Y в символах
    attr - цвет
*******************************************************************************/
void box (int x0, int y0, int x1, int y1, WORD attr)
{
  hline (y0, x0+1, x1-1, 196, attr);
  hline (y1, x0+1, x1-1, 196, attr);
  vline (x0, y0+1, y1-1, 179, attr);
  vline (x1, y0+1, y1-1, 179, attr);
  pset (x0, y0, 218, attr);
  pset (x0, y1, 192, attr);
  pset (x1, y0, 191, attr);
  pset (x1, y1, 217, attr);
}
