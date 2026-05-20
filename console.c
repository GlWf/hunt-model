#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <time.h>
//#include <sys/stat.h>
//#include <sys/types.h>

#include <windows.h>
#include "config.h"

#define DEBUG_CONSOLE_RESIZE  0

static int MAX_X, MAX_Y;

static HANDLE hStdout, hStdin;
static CONSOLE_SCREEN_BUFFER_INFO consoleInfo;

CHAR_INFO FrameBuffer[(MAX_FSIZE+2) * (MAX_FSIZE+2) * 2];

#if DEBUG_CONSOLE_RESIZE
static void print_coord (const char *prefix, COORD *val);
static void print_rect (const char *prefix, SMALL_RECT *val);
static void print_console_info (void);
#endif
/*********************************************************************/
void cursor_parameters (DWORD dwSize, BOOL bVisible)
{
  CONSOLE_CURSOR_INFO curs = {.dwSize = dwSize, .bVisible = bVisible};
  SetConsoleCursorInfo (hStdout, &curs);
}

/*********************************************************************/
void cursor_position (SHORT x, SHORT y)
{
  COORD coord = {.X = x, .Y = y};
  SetConsoleCursorPosition (hStdout, coord);
}

/*********************************************************************/
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

/*********************************************************************/
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

/*********************************************************************/
int kbhit (void)
{
  DWORD num;
  char res;
  INPUT_RECORD inr;
  int result = -1;

  if (!GetNumberOfConsoleInputEvents(hStdin, &num))  {
    fprintf (stderr, "cannot get number of events\n");
  }
  if (!num)  return result;

  //ReadConsole(hStdin, &res, 1, NULL, NULL);

  ReadConsoleInput(hStdin, &inr, 1, &num);
  if (inr.EventType == KEY_EVENT)  {
    if (inr.Event.KeyEvent.bKeyDown)  {
      res = inr.Event.KeyEvent.wVirtualKeyCode;
      result = res;
    }
  }
  return result;
}

/*********************************************************************/
void console_title (const char *title)
{
  SetConsoleTitle (title);
}

/*********************************************************************/
static void console_set_mode (void)
{
  DWORD dwOriginalOutMode = 0;
  DWORD dwOriginalInMode = 0;
  if (!GetConsoleMode(hStdout, &dwOriginalOutMode)) {
      return;
  }
  if (!GetConsoleMode(hStdin, &dwOriginalInMode)) {
      return;
  }
  DWORD dwRequestedOutModes = 0; //ENABLE_VIRTUAL_TERMINAL_PROCESSING;
  //DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

  DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
  if (!SetConsoleMode(hStdout, dwOutMode)) {
    fprintf(stderr, "Failed to set any VT mode, can't do anything here.\n");
    return;
  }

  //DWORD dwInMode = dwOriginalInMode | dwRequestedInModes;
  //if (!SetConsoleMode(hStdin, dwInMode)) {
  //  fprintf(stderr, "Failed to set VT input mode, can't do anything here.\n");
  //  return;
  //}
}

/*********************************************************************/
void console_clear (void)
{
  cursor_position(0, 0);
  fprintf(stdout, "\033[0J");
  fflush(stdout);
}

/*********************************************************************/
void console_resize (int max_x, int max_y)
{
  if (!GetConsoleScreenBufferInfo (hStdout, &consoleInfo)) {
    fprintf (stderr, "**Unable to get console buffer info.\n");
    return;
  }

  #if DEBUG_CONSOLE_RESIZE
  print_console_info();
  #endif

  int w = (max_x + 10);
  int h = (max_y + 8);

  #if DEBUG_CONSOLE_RESIZE
  printf("w = %d, h = %d\n", w, h);
  #endif

  if ((consoleInfo.srWindow.Right < w) ||
      (consoleInfo.srWindow.Bottom < h)) {

    int buf_w = (w <= consoleInfo.dwSize.X) ? consoleInfo.dwSize.X : w;
    int buf_h = (h <= consoleInfo.dwSize.Y) ? consoleInfo.dwSize.Y : h;

    COORD nc = {.X = buf_w, .Y = buf_h};
    BOOL res1 = SetConsoleScreenBufferSize(hStdout, nc);
    if (!res1) {
      printf("result1 = %d err=%ld\n", res1, GetLastError());
    }
    #if DEBUG_CONSOLE_RESIZE
    printf("buf_w = %d, buf_h = %d\n", buf_w, buf_h);
    print_console_info();
    #endif

    int win_w = buf_w - 1;
    int win_h = h;

    /* получить максимально допустимые размеры окна */
    COORD scrsize = GetLargestConsoleWindowSize(hStdout);
    /* ограничить размеры окна при превышении маскимально допустимых значений */
    win_w = (win_w > scrsize.X) ? scrsize.X-1 : win_w;
    win_h = (win_h > scrsize.Y) ? scrsize.Y-1 : win_h;

    SMALL_RECT sr = {.Left = 0, .Top = 0, .Right = win_w, .Bottom = win_h};
    BOOL res2 = SetConsoleWindowInfo(hStdout, TRUE, &sr);
    if (!res2) {
      printf("result2 = %d err=%ld\n", res2, GetLastError());
    }
    #if DEBUG_CONSOLE_RESIZE
    print_coord ("Largest win size: ", &scrsize);
    printf("win_w = %d, win_h = %d\n", win_w, win_h);
    #endif
  }
  #if DEBUG_CONSOLE_RESIZE
  getchar();
  #endif
}

/*********************************************************************/
void console_init (int max_x, int max_y)
{
  hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  hStdin  = GetStdHandle(STD_INPUT_HANDLE);
  if (hStdout == INVALID_HANDLE_VALUE)  {
    fprintf (stderr, "**Unable to get stdout handle.\n");
    return;
  }

  console_set_mode();
  console_resize (max_x, max_y);

  FlushConsoleInputBuffer(hStdin);
  MAX_X = max_x;
  MAX_Y = max_y;
}


/*********************************************************************/
void console_finish (void)
{
  cursor_parameters (75, 1);
}

/*********************************************************************/
void pset (int x, int y, char ch, WORD attr)
{
  int index = y*MAX_X + x;
  FrameBuffer[index].Char.AsciiChar = ch;
  FrameBuffer[index].Attributes = attr;
}

/*********************************************************************/
void vline (int x, int y0, int y1, char ch, WORD attr)
{
  int y;
  for (y = y0; y <= y1; y++)  {
    pset (x, y, ch, attr);
  }
}

/*********************************************************************/
void hline (int y, int x0, int x1, char ch, WORD attr)
{
  int x;
  for (x = x0; x <= x1; x++)  {
    pset (x, y, ch, attr);
  }
}

/*********************************************************************/
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

/*********************************************************************/
void draw_string (int x, int y, const char *s, int len, WORD attr)
{
  char i;
  int index = y * MAX_X + x;
  for (i = 0; i < len; i++, index++)  {
    if (*s == 0)  break;
    FrameBuffer[index].Char.AsciiChar = *s++;
    FrameBuffer[index].Attributes = attr;
  }
}




#if DEBUG_CONSOLE_RESIZE
/*********************************************************************/
static void print_coord (const char *prefix, COORD *val)
{
  printf("%s: x = %d, y = %d\n", prefix, val->X, val->Y);
}

/*********************************************************************/
static void print_rect (const char *prefix, SMALL_RECT *val)
{
  printf("%s: (%d, %d) - (%d, %d)\n", prefix, val->Left, val->Top,
                                              val->Right, val->Bottom);
}

/*********************************************************************/
static void print_console_info (void)
{
  if (!GetConsoleScreenBufferInfo (hStdout, &consoleInfo)) {
    fprintf (stderr, "**Unable to get console buffer info.\n");
    return;
  }
  print_coord("---------\ndwSize", &consoleInfo.dwSize);
  print_coord("Curs", &consoleInfo.dwCursorPosition);
  print_rect("srWindow", &consoleInfo.srWindow);
  print_coord("MaxWindowSize", &consoleInfo.dwMaximumWindowSize);
  printf ("Attributes = 0x%X\n------------\n", consoleInfo.wAttributes);
}

#endif
