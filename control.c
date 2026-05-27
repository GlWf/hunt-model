#include <stdio.h>
#include "console.h"
#include <string.h>
#include "config.h"
#include "model.h"
#include "view.h"

int AutoMode = 0;

/*------------------------------------------------------------------------*/
int main()
{
    /* Инициализация генератора псевдослучайных чисел
     * в качестве параметра используется время
     * с момента запуска системы (в милисекундах) */
    srand(GetTickCount());

    console_title ("Hunt emulation");  /* Заголовок консоли */
    input_parameters();                /* Введение параметров для модели */

    console_init (FSIZE*2+2, FSIZE+2); /* инициализация консоли */

    cursor_parameters (75, 0);         /* выключение курсора */
    clear();                           /* очистка FrameBuffer */
    console_clear ();                  /* очистка консоли */

    /* рамка вокруг игрового поля во FrameBuffer */
    box(0, 0, FSIZE*2+1, FSIZE+1, TCLR_GREY);

    /* инициализация модели */
    model_init();

    /* Добавление подсказок по управлению моделью */
    cursor_position(0, FSIZE+6);
    printf("press SPACE to step\n"
           "press Q or ESCAPE to exit,\n"
           "press A to turn on automatic mode\n"
           "press R to restart with the same parameters");

    view_all();  /* визуализация модели на текущем шаге в FrameBuffer */
    redraw();    /* вывод FrameBuffer в консоль */

    while (1)  {
      int key = kbhit ();   /* считывание нажатой клавиши без ожидания */

      if ((key == 0x51) || (key == VK_ESCAPE))  { /* клавиша ESC или Q */
        break;  /* выход из программы */
      }
      else if (key == 32) { /* клавиша ПРОБЕЛ - ручной пошаговый режим */
        AutoMode = 0;       /* остановка автоматического режима */
        model();            /* моделирование одного шага */
        view_all();   /* визуализация модели на текущем шаге в FrameBuffer */
        redraw();     /* вывод FrameBuffer в консоль */
        continue;
      }
      else if (key == 0x41) { /* клавиша A */
        AutoMode = 1;         /* запуск автоматического режима */
      }
      else if (key == 0x52) { /* клавиша R */
        model_finish (); /* всё аккуратно завершить */
        model_init ();   /* проинициализировать модель заново */
        view_all();      /* визуализация модели на текущем шаге в FrameBuffer */
        redraw();        /* вывод FrameBuffer в консоль */
      }

      if (AutoMode) {
        model();      /* моделирование одного шага в автоматическом режиме */
        view_all();   /* визуализация модели на текущем шаге в FrameBuffer */
        redraw();     /* вывод FrameBuffer в консоль */
        /* если моделирование завершено, то выключить автоматический режим */
        if (is_model_finished()) {
          AutoMode = 0;
        }
        Sleep (50);   /* задержка 50 мс перед следующим шагом */
      }
    }
    model_finish ();  /* завершение работы с моделью*/
    console_finish(); /* завершение работы с консолью */
    return 0;
}
