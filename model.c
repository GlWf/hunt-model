#include <stdio.h>
#include <string.h>
#include "config.h"
#include "model.h"
#include "console.h"

#define MAX_ANIMALS    (MAX_FSIZE)*(MAX_FSIZE)

animal_list_t TheList = {0}; /* переменная-список животных */

animal_t Animal[MAX_ANIMALS] = {0}; /* массив где хранятся структуры */

animal_t *GameField[MAX_FSIZE+2][MAX_FSIZE+2]; /* игровое поле */

animal_t Cat; /* структура животного-кошки, одна на всех,
ее нет в списке и в массиве. Нужна для инициализации модели */

int CntAnimals = 0;  /* количество животных */
int StepCounter = 0; /* номер шага моделирования */

FILE *logfile; /* Создание файла для отладки */
#define LOG_PRINTF  fprintf

  /* Прототипы локальных функций
   * (описание для каждой функции находится рядом с самими функциями) */
static void animal_add_to_list (animal_t *an);
static void animal_remove_from_list (animal_t *an);
static int get_free_field (int sx, int sy, int *dx, int *dy);

static void animal_jump (animal_t *an);
static animal_t *animal_find(animal_t *an, int type);
static void animal_born(animal_t *parent, int born_chance);
static void animal_die (animal_t *an);
static void animal_lifetime (animal_t *wolf);
static void wolf_eat_rabbit (animal_t *wolf, animal_t *rabbit);

static void model_rabbit (animal_t *an);
static void model_wolf_f (animal_t *an);
static void model_wolf_m (animal_t *an);

/******************************************************************************
  static void animal_add_to_list (animal_t *an)

  Функция добавления животного в список.

  Параметр - указатель на структуру, описывающую конкретное животное.
******************************************************************************/
static void animal_add_to_list (animal_t *an)
{
  if (an == NULL) return;  /* NULL argument */

  animal_t *last_animal = TheList.last;

  if (TheList.first == NULL) { /* empty list */
    TheList.first = an;
  }
  else {
    last_animal->next = an;
  }
  an->prev = last_animal;
  TheList.last = an;
  an->next = NULL;
  CntAnimals++;
}

/******************************************************************************
  static void animal_remove_from_list (animal_t *an)

  Функция удаления животного из списка.

  Параметр - указатель на структуру, описывающую конкретное животное.
******************************************************************************/
static void animal_remove_from_list (animal_t *an)
{
  if (an == NULL) return;  /* NULL argument */

  animal_t *prev_animal = an->prev;
  animal_t *next_animal = an->next;

  CntAnimals--;

  if ((next_animal == NULL) && (prev_animal == NULL)) { /* one elem in list */
    TheList.first = NULL;
    TheList.last = NULL;
    return;
  }

  if (next_animal != NULL) {
    next_animal->prev = prev_animal;
  }
  else { /* removing last element */
    TheList.last = prev_animal;
  }

  if (prev_animal != NULL) {
    prev_animal->next = next_animal;
  }
  else { /* removing first element */
    next_animal->prev = NULL;
    TheList.first = next_animal;
  }
}

/*******************************************************************************
  int add_animal (int x, int y, int type, int vitality)

  Функция добавления животного

  Параметры:
    x, y - координаты точки появления
    type - тип животного
    vitality - количество очков жизненной силы

  Возвращает
*******************************************************************************/
int add_animal (int x, int y, int type, int vitality)
{
  animal_t *an = NULL;
  int i;
  for (i = 0; i < MAX_ANIMALS; i++) {
    if (Animal[i].type == ANIMAL_NONE) {
      an = &Animal[i];
      break;
    }
  }

  if (an == NULL) {
    /* вывод в журнал */
    LOG_PRINTF (logfile, "**Animals overflow\n");
    return 0;
  }

  an->x = x;
  an->y = y;
  an->type = type;
  an->vitality = vitality;
  an->age = 0;

  animal_add_to_list (an);
  GameField[x][y] = an;
  return 1;
}

/*******************************************************************************
  void add_animal_at_random_place (int type)

  Функция добавления животного в случайное место. Если место уже занято,
  то животное не создаётся.

  Параметры:
    type - тип животного

*******************************************************************************/
void add_animal_at_random_place (int type)
{
  int x = (rand() % FSIZE) + 1;
  int y = (rand() % FSIZE) + 1;
  if (GameField[x][y] == NULL) {
    add_animal (x, y, type, vitality);
    LOG_PRINTF (logfile, "  animal %d at random (%d,%d)\n", type, x, y);
  }
  else {
    LOG_PRINTF (logfile, "  animal %d collision at (%d,%d)\n", type, x, y);
  }
}

/*******************************************************************************
  static int get_free_field (int sx, int sy, int *dx, int *dy)

  Функция для нахождения свободной ячейки рядом с заданной. Если таких ячеек
  несколько, то выбирается случайная.

  Параметры:
    sx, sy - координаты ячейки вокруг которой ищется свободная ячейка.
    dx, dy - указатели, по которым сохраняются координаты найденной свободной
             ячейки.

  Возвращает:
    0 - нет свободных ячеек около заданной.
    1 - свободная ячейка найдена.
*******************************************************************************/
static int get_free_field (int sx, int sy, int *dx, int *dy)
{
  int x[9], y[9];   /* массивы координат свободных ячеек */
  int free_cnt = 0; /* количество свободных ячеек вокруг */

  /* перебор всех соседних ячеек и сохранение координат свободных из них
   * в массивы x[] и y[] */
  for(int i = -1; i <= 1; i++) {
    for(int j = -1; j <= 1; j++) {
      if (GameField[sx+i][sy+j] == NULL) {
        x[free_cnt] = sx+i;
        y[free_cnt] = sy+j;
        free_cnt++;
      }
    }
  }
  if (free_cnt == 0) return 0;  /* если количество свободных ячеек = 0 */

  int free_point = rand() % free_cnt; /* выбор случайной ячейки из свободных */

  /* сохранение координат выбранной ячейки */
  *dx = x[free_point];
  *dy = y[free_point];
  return 1;
}

/*******************************************************************************
  void model_init (void)

  Функция инициализации модели

  Заполнение неиспользуемыми животными (в данной модели - котами) по краям
  для корректной работы модели
*******************************************************************************/
void model_init (void)
{
  Cat.type = ANIMAL_CAT; /* тип = кошка */
  /* очистка игрового поля */
  memset(GameField, 0, sizeof(GameField));

  /* заполнение периметра игрового поля (за пределами рабочей области) кошками
   * (чтоб были не пустые ячейки и чтоб в модели не проверять координаты на
   * предмет выхода за границы - модель просто будет всегда натыкаться на
   * непустые ячейки до того как выйдет за границы */
  int i;
  for (i = 0; i <= FSIZE+1; i++) {
    GameField[0][i] = &Cat;
    GameField[i][0] = &Cat;
    GameField[FSIZE+1][i] = &Cat;
    GameField[i][FSIZE+1] = &Cat;
  }
  logfile = fopen("__log.txt", "w");
}


/*******************************************************************************
  static void animal_jump (animal_t *an)

  Функция моделирования перемещения животного в случайную соседнюю точку.

  Параметр - указатель на структуру, описывающую конкретное животное.
*******************************************************************************/
static void animal_jump (animal_t *an)
{
/* вычисление величин случайного изменения координатам: -1, 0 или 1 */
  int dx = (rand() % 3) - 1;
  int dy = (rand() % 3) - 1;

  /* вычисление новых координат */
  int nx = an->x + dx;
  int ny = an->y + dy;

  /* Если по новым координатам свободно, то кролик прыгает туда.
   * проверки на выход за границы поля нет, так как по периметру за границами
   * сидят кошки, то есть ячейки занята и прыжка на самый край не будет */
  if (GameField[nx][ny] == NULL) {
    GameField[an->x][an->y] = NULL;
    an->x = nx;
    an->y = ny;
    GameField[nx][ny] = an;
  }
}
/*******************************************************************************
  static animal_t *animal_find(animal_t *an, int type)

  Функция поиска конкретного животного в соседних клетках.

  Параметры:
    an - указатель на структуру, описывающую конкретное животное.
    type - тип животного, которое необходимо найти.

  Возвращает случайный указатель из найденных соседних животных.
*******************************************************************************/
static animal_t *animal_find(animal_t *an, int type)
{
  animal_t *neighbor[9]; /* массив указателей на найденных животных */
  int animal_cnt = 0;    /* количество найденных животных */
  int sx = an->x; /* координаты, относительно которых искать соседей */
  int sy = an->y;

  /* перебор всех соседних ячеек и сохранение координат свободных из них
   * в массивы x[] и y[] */
  for(int i = -1; i <= 1; i++) {
    for(int j = -1; j <= 1; j++) {
      if ((i == 0) && (j == 0)) { /* не проверять самого себя (центр) */
        continue;
      }
      /* если ячейка пустая (нет животного) */
      if (GameField[sx+i][sy+j] == NULL) {
        continue;
      }

      /* проверка типа */
      if (GameField[sx+i][sy+j]->type == type) {
        /* сохранение указателя на найденное животное в массив указателей
         * найденных животных */
        neighbor[animal_cnt] = GameField[sx+i][sy+j];
        animal_cnt++;
      }
    }
  }
  if (animal_cnt == 0) return NULL;  /* если количество животных = 0 */

  int curr_animal = rand() % animal_cnt; /* выбор случайного животного
                                            из найденных */
  /* вернуть указатель на случайное животное из найденных */
  return neighbor[curr_animal];
}
/*******************************************************************************
  static void animal_born(animal_t *parent, int born_chance)

  Функция моделирования случая рождения нового животного.

  Параметр - указатель на структуру, описывающую конкретное животное.
*******************************************************************************/
static void animal_born(animal_t *parent, int born_chance)
{
  int nx, ny;
  /* случайным образом с заданной вероятностью определяется не пора ли
   * животному размножаться */
  if ((rand () % 100) < born_chance){
    /* если размножаться пора, то определяются координаты случайной свободной
     * ячейки по соседству */
    if (get_free_field(parent->x, parent->y, &nx, &ny)) {
      /* если свободная ячейка по соседству имеется, то создается новое
       * животное в зависимости от типа родителя и помещается в эту ячейку */
      if (parent->type == ANIMAL_RABBIT){
        add_animal(nx, ny, ANIMAL_RABBIT, vitality);
        /* вывод в журнал координат нового кролика (для отладки) */
        LOG_PRINTF (logfile, "  new rabbit at (%d,%d). parent (%d,%d)\n",
                                              nx, ny, parent->x, parent->y);
      }
      else if (parent->type == ANIMAL_WOLF_M){
        /* случайный выбор пола волчонка */
        int child_type;
        if ((rand() % 2) == 0) child_type = ANIMAL_WOLF_M;
        else                   child_type = ANIMAL_WOLF_F;

        /* волк при смерти не размножается */
        if (parent->vitality == 1) return;
        /* новорожденные и малые дети не размножаются */
        if (parent->age <= 2) return;

        int base_vitality = vitality;
        if (parent->vitality < base_vitality) {
          base_vitality = parent->vitality;
        }

        /* добавление животного-потомка */
        add_animal(nx, ny, child_type, base_vitality);
        /* вывод в журнал координат нового волчонка (для отладки) */
        LOG_PRINTF (logfile,
                    "  new wolf %d at (%d,%d). parent (%d,%d). vitality = %d\n",
                    child_type, nx, ny, parent->x, parent->y, base_vitality);
      }
    }
  }
}

/*******************************************************************************
  static void animal_die (animal_t *an)

  Функция моделирования случая смерти животного.

  Параметр - указатель на структуру, описывающую конкретное животное.
*******************************************************************************/
static void animal_die (animal_t *an)
{
  GameField[an->x][an->y] = NULL;
  animal_remove_from_list(an);
  an->type = ANIMAL_NONE;
}
/*******************************************************************************
  static void wolf_eat_rabbit (animal_t *wolf, animal_t *rabbit)

  Функция моделирования случая съедания кролика волком.

  Параметры - указатели на структуры, описывающие конкретных волка и кролика.
*******************************************************************************/
static void wolf_eat_rabbit (animal_t *wolf, animal_t *rabbit)
{
  /* сохранить координаты кролика */
  int x = rabbit->x;
  int y = rabbit->y;
  animal_die (rabbit);  /* кролик умер */

  wolf->vitality += vitality; /* добавление очков жизни волку */
  /* ограничение на количество очков волка */
  if (wolf->vitality > MAX_WOLF_VITALITY) {
    wolf->vitality = MAX_WOLF_VITALITY;
  }

  /* вывод в журнал */
  LOG_PRINTF (logfile, "  wolf (%d,%d) eat rabbit (%d,%d) - vitality = %d\n",
                                        wolf->x, wolf->y, x, y, wolf->vitality);

  /* волк перемещается туда где был кролик */
  GameField[wolf->x][wolf->y] = NULL;
  wolf->x = x;
  wolf->y = y;
  GameField[x][y] = wolf;
}

/*******************************************************************************
  static void animal_lifetime (animal_t *wolf)

  Функция моделирования жизнеспособности волка.

  Параметр - указатель на структуру, описывающую конкретного волка.
*******************************************************************************/
static void animal_lifetime (animal_t *wolf)
{
  wolf->vitality--;
  if (wolf->vitality == 0) {
    /* вывод в журнал */
    LOG_PRINTF (logfile, "  wolf (%d,%d) died in hunger\n", wolf->x, wolf->y);
    animal_die(wolf);
  }
}
/*******************************************************************************
  static void model_rabbit (animal_t *an)

  Функция моделирования поведения кролика.

  Параметр - указатель на структуру, описывающую конкретного кролика.
*******************************************************************************/
static void model_rabbit (animal_t *an)
{
  animal_jump (an); /* прыжок на случайную свободную соседнюю ячейку */
  animal_born (an, chance); /* размножение */
}

/*******************************************************************************
  static void model_wolf_m (animal_t *an)

  Функция моделирования поведения волка.
  Параметр - указатель на структуру, описывающую конкретного волка.
*******************************************************************************/
static void model_wolf_m (animal_t *an)
{
  animal_t *rabbit, *female;
  rabbit = animal_find (an, ANIMAL_RABBIT); /* поиск кролика по соседству */
  female = animal_find (an, ANIMAL_WOLF_F); /* поиск волчицы по соседству */
  if ((rabbit != NULL) && (female != NULL)) {
    /* если рядом есть и кролик и волчица */
    if (an->vitality > vitality) { /* если очков много - размножение */
      animal_born (an, 100);
    }
    else { /* если очков мало - еда */
      wolf_eat_rabbit (an, rabbit);
    }
  }
  else if (female != NULL) {
    /* если рядом есть только волчица - размножение */
    animal_born (an, 100);
  }
  else if (rabbit != NULL) {
    /* если рядом есть только кролик - прыгнуть на него и съесть */
    wolf_eat_rabbit (an, rabbit);
  }
  else {
    /* если рядом нет ни волчицы, ни кролика */
    animal_jump(an); /* прыжок на случайную свободную соседнюю ячейку */
  }
  animal_lifetime (an);
}

/*******************************************************************************
  static void model_wolf_f (animal_t *an)

  Функция моделирования поведения волчицы.
  Параметр - указатель на структуру, описывающую конкретную волчицу.
*******************************************************************************/
static void model_wolf_f (animal_t *an)
{
  animal_t *rabbit;
  rabbit = animal_find (an, ANIMAL_RABBIT); /* поиск кролика по соседству */
  if (rabbit != NULL) {
    /* если рядом есть кролик - прыгнуть на него и съесть */
    wolf_eat_rabbit (an, rabbit);
  }
  else {
    /* если рядом нет кролика */
    animal_jump(an); /* прыжок на случайную свободную соседнюю ячейку */
  }
  animal_lifetime (an);
}

/*******************************************************************************
  void model (void)

  Функция моделирования одного шага.

  Проходит по списку животных и для каждого вызывает функцию моделирования
  его поведения на этом шаге.
*******************************************************************************/
void model (void)
{
  StepCounter++; /* увеличение номера шага */
  LOG_PRINTF (logfile, "Step step: %d\n", StepCounter);

  animal_t *an = TheList.first;  /* берется первое животное в списке */
  while (an != NULL) {           /* цикл пока список не закончится */
    /* моделирование животного в зависимости от его типа */
    if (an->type == ANIMAL_RABBIT) {
      model_rabbit(an);
    }
    else if (an->type == ANIMAL_WOLF_M) {
      model_wolf_m(an);
    }
    else if (an->type == ANIMAL_WOLF_F) {
      model_wolf_f(an);
    }
    an->age++;      /* увеличиваем возраст животного */
    an = an->next;  /* переход к следующему животному в списке */
  }
}

/*******************************************************************************
  void model_finish(void)

  Функция, вызывающаяся в конце модели.

  Сохраняет данные и закрывает отладочный файл на запись.
*******************************************************************************/
void model_finish(void)
{
  fclose(logfile);
}

/*******************************************************************************
  int is_model_finished(void)

  Функция проверки на наличие животных.

  Возвращает 1, если Количество животных = 0.
*******************************************************************************/
int is_model_finished(void){
  if (CntAnimals == 0) {
    return 1;
  }
  return 0;
}
