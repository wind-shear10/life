
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#ifdef _WIN32
    #include<windows.h>
    #include <SDL2/SDL.h>
#else
    #include<SDL.h>
#endif


#define CELL_SIZE 10
#define MAX_ROWS 100
#define MAX_COLS 170

typedef unsigned short ushort;

typedef struct 
{
    int generation;
    int running;
    int paused;
    int delay;
    Uint32 paused_time;
    Uint32 simulation_start;
    Uint32 elapsed;
    ushort ROWS;
    ushort COLS;
}game_config;


void free_grid(ushort ***grid, ushort ***new_grid, ushort ROWS)
{
    for (int i = 0; i < ROWS; i++)
    {
        free((*grid)[i]);
        free((*new_grid)[i]);
    }
    free(*grid);
    free(*new_grid);
}

int allocate_grid(ushort ***grid, ushort ***new_grid, ushort ROWS, ushort COLS)
{ // создаем два массива размера rows*cols
    int i;
    *grid = malloc(ROWS * sizeof(unsigned short *));
    *new_grid = malloc(ROWS * sizeof(unsigned short *)); // выделяем память и создаем массив указателей, число указателей - число строк в массиве

    if (!(*grid) || !(*new_grid))
    {
        printf("ошибка выделения памяти\n");
        free(*grid);
        free(*new_grid);
        return 0;
    }

    for (i = 0; i < ROWS; i++)
    {
        (*grid)[i] = malloc(COLS * sizeof(unsigned short)); // каждому указателю в массиве передаем адрес, где выделена память в количестве cols, тем самым получается двумерный массив
        (*new_grid)[i] = malloc(COLS * sizeof(unsigned short));
        if (!(*grid)[i] || !(*new_grid)[i])
        {
            printf("ошибка выделения памяти\n");
            free_grid(grid, new_grid, ROWS);
            return 0;
        }
    }

    return 1;
}

void randomize_grid(ushort **grid, ushort ROWS, ushort COLS)
{
    if (!grid)
    {
        printf("ошибка, указатель нулевой\n");
        return;
    }
    for (int y = 0; y < ROWS; y++)
        for (int x = 0; x < COLS; x++)
            grid[y][x] = rand() % 2;
}

int count_neighbors(int y, int x, ushort **grid, ushort ROWS, ushort COLS)
{
    if (!grid)
    {
        printf("ошибка, указатель нулевой\n");
        return 0;
    }
    int count = 0;
    int dy, dx;
    for (dy = -1; dy <= 1; dy++)
    {
        for (dx = -1; dx <= 1; dx++)
        {
            if (dy == 0 && dx == 0)
                continue;
            int ny = (y + dy + ROWS) % ROWS;
            int nx = (x + dx + COLS) % COLS;
            count += grid[ny][nx];
        }
    }
    return count;
}

void update_grid(ushort **grid, ushort **new_grid, ushort ROWS, ushort COLS)
{
    if (!grid || !new_grid)
    {
        printf("ошибка, указатель нулевой\n");
        return;
    }
    int x, y;
    for (y = 0; y < ROWS; y++)
    {
        for (x = 0; x < COLS; x++)
        {
            int neighbors = count_neighbors(y, x, grid, ROWS, COLS);
            if (grid[y][x] && (neighbors == 2 || neighbors == 3))
                new_grid[y][x] = 1;
            else if (!grid[y][x] && neighbors == 3)
                new_grid[y][x] = 1;
            else
                new_grid[y][x] = 0;
        }
    }

    for (y = 0; y < ROWS; y++)
        for (x = 0; x < COLS; x++)
            grid[y][x] = new_grid[y][x];
}

void draw_grid(SDL_Renderer *renderer, ushort **grid, int hover_x, int hover_y, game_config current_config)
{
    int x, y;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (y = 0; y < current_config.ROWS; y++)
    {
        for (x = 0; x < current_config.COLS; x++)
        {
            if (grid[y][x])
            {
                SDL_Rect cell = {x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
                SDL_RenderFillRect(renderer, &cell);
            }
        }
    }

    if (current_config.paused && hover_x >= 0 && hover_x < current_config.COLS && hover_y >= 0 && hover_y < current_config.ROWS)
    {

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // белая рамка
        SDL_Rect hover_cell = {hover_x * CELL_SIZE, hover_y * CELL_SIZE, CELL_SIZE, CELL_SIZE};
        SDL_RenderDrawRect(renderer, &hover_cell);
    }

    SDL_RenderPresent(renderer);
}
void stdin_clear()
{
    char c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void input_size(unsigned short *ROWS, unsigned short *COLS)
{
    while (1)
    {
        printf("Введите количество строк: "); // МАКС РАЗМЕР : 100*170
        if (scanf("%hu", ROWS) == 1 && *ROWS <= MAX_ROWS)
            break;
        stdin_clear();
        printf("неправильный ввод\n");
    }
    while (1)
    {
        printf("Введите количество столбцов: ");
        if (scanf("%hu", COLS) == 1 && *COLS <= MAX_COLS)
            break;
        stdin_clear();
        printf("неправильный ввод\n");
    }
}

void save_to_file(ushort ROWS, ushort COLS, int generation, ushort **grid){
    if(!grid){
        return;
    }
    int i;
    char filename[260];
    printf("\nвведите имя файла для сохранения: ");
    scanf("%s", filename);
    stdin_clear();
    FILE *f = fopen(filename, "wb");

    if (!f)
    {
        perror("Ошибка открытия файла");
        return;
    }

    if (fwrite(&ROWS, sizeof(ushort), 1, f) != 1 || fwrite(&COLS, sizeof(ushort), 1, f) != 1 || fwrite(&generation, sizeof(int), 1, f) != 1)
    {
        perror("Ошибка записи в файл");
        fclose(f);
        return;
    }

    for (i = 0; i < ROWS; i++){
        if(fwrite(grid[i], sizeof(ushort), COLS, f) != COLS){
            perror("Ошибка записи в файл");
            fclose(f);
            return;
        }
    }
    
    printf("Данные успешно сохранены в файл \"%s\"\n", filename);

    fclose(f);

}

int load_from_file(ushort *ROWS, ushort *COLS, int *generation, ushort ***grid, ushort ***new_grid){
    if(!grid){
        return 0;
    }
    int i;
    char filename[260];
    printf("\nвведите имя файла для загрузки: ");
    scanf("%s", filename);
    stdin_clear();
    FILE *f = fopen(filename, "rb");
    if (!f)
    {
        perror("\nОшибка открытия файла");
        return 0;
    }

    if(fread(ROWS, sizeof(ushort), 1, f) != 1 || fread(COLS, sizeof(ushort), 1, f) != 1 || fread(generation, sizeof(int), 1, f) != 1){
        perror("\nошибка");
        fclose(f);
        return 0;
    }

    if(!allocate_grid(grid, new_grid, *ROWS, *COLS)){
        printf("Завершение с ошибкой 1\n");
        return 0;
    }
    for (i = 0; i < *ROWS; i++) {
        if(!fread((*grid)[i], sizeof(unsigned short), *COLS, f)){
            printf("\nошибка");
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    return 1;


}

int start_config(ushort *** grid, ushort ***new_grid, game_config *current_config){
    if(!grid || !new_grid){
        return 0;
    }
    int user_choice;
    ushort i, j;
    printf("выберите режим работы\n1 - рандомная симуляция\n2 - загрузить симуляцию из файла\n3 - пустая симуляция\n");
    printf("\nspace + S - сохранить симуляцию\nстрелка вверх - ускорение\nстрелка вниз - замедление\nspace - пауза\n");
    while(1){
        if(scanf("%d", &user_choice) && (user_choice == 1 || user_choice == 2 || user_choice == 3))
            break;
        stdin_clear();
        printf("неправильный ввод\n");
    }
    if(user_choice == 1){
        input_size(&(current_config -> ROWS), &(current_config -> COLS));
        if (!allocate_grid(grid, new_grid, current_config -> ROWS, current_config -> COLS))
        {
            printf("Завершение с ошибкой \n");
            return 0;
        }
        randomize_grid(*grid, current_config -> ROWS, current_config -> COLS);
        current_config -> running = 1;
        current_config -> paused = 1;
        current_config -> generation = 0;
        return 1;
    }
    if(user_choice == 2){
        if(!load_from_file(&(current_config -> ROWS), &(current_config -> COLS), &(current_config -> generation), grid, new_grid))
            return 0;
        current_config -> paused = 1;
        current_config->running = 1;
        return 1;
    }
    input_size(&(current_config -> ROWS), &(current_config -> COLS));
    if(!allocate_grid(grid, new_grid, current_config -> ROWS, current_config -> COLS))
        return 0;
    for(i = 0; i < current_config -> ROWS; i++)
        for(j = 0; j < current_config -> COLS; j++){
            (*grid)[i][j] = 0;
            (*new_grid)[i][j] = 0;
        }
    current_config -> running = 1;
    current_config -> paused = 1;
    current_config -> generation = 0;

    return 1;

}



void event_listener(SDL_Event *event, ushort **grid, int hover_x, int hover_y, game_config *current_config)
{
    if(!grid){
        return;
    }
    while (SDL_PollEvent(event))
    {                               // захватываем наше событие(если оно есть то ретурн 1) и помещаем в структ. event, событие удаляется из очереди
        if (event -> type == SDL_QUIT) // смотрим что за событие (здесь - закрытие окна через крестик)
            current_config -> running = 0;
        else if (event -> type == SDL_KEYDOWN)
        { //(истинна, если мы нажали любую клавишу)
            switch (event -> key.keysym.sym)
            {                 // из структуры event вытаскимаем тип нажатой клавиши
            case SDLK_ESCAPE: // нажали esc
                current_config -> running = 0;
                break;
            case SDLK_SPACE:                      // нажали пробел (пауза)
                current_config -> paused = !(current_config -> paused);                 // переключаем паузу
                if (current_config -> paused)                       // если пауза активна
                    current_config -> paused_time = SDL_GetTicks(); // фиксируем время от начала запуска, когда была нажата пауза(момент, с которого пошла пауза)
                else
                    current_config -> simulation_start += SDL_GetTicks() - current_config -> paused_time; // если убрали паузу, записываем время, сколько шла пауза (sim_start)
                break;
            case SDLK_UP:
                if (current_config -> delay > 10)
                    current_config -> delay -= 10; // ускорение
                break;
            case SDLK_DOWN: // замедление
                current_config -> delay += 10;
                break;
            case SDLK_s:
                if(current_config -> paused)
                    save_to_file(current_config -> ROWS, current_config -> COLS, current_config -> generation, grid);
                break;
            }
        }
        else if (event -> type == SDL_MOUSEBUTTONDOWN && current_config -> paused)
        {
            if (hover_x >= 0 && hover_x < current_config -> COLS && hover_y >= 0 && hover_y < current_config -> ROWS)
            {
                grid[hover_y][hover_x] = !grid[hover_y][hover_x];
            }
        }
    }
}


int main(int argc, char *argv[]){

    #ifdef _WIN32
        system("chcp 65001 > nul");
        SetConsoleOutputCP(CP_UTF8);
    #endif
    
    game_config current_config;
    current_config.delay = 100;
    unsigned short **grid, **new_grid; // указатель на массив указателей, которыt в свою очеред содержат указатели на начало строк
    int mouse_x, mouse_y;
    int hover_x, hover_y; 

    srand(time(NULL)); //

    if(!start_config(&grid, &new_grid, &current_config))
        return 1;

    int window_width = current_config.COLS * CELL_SIZE;
    int window_height = current_config.ROWS * CELL_SIZE;

    SDL_Init(SDL_INIT_VIDEO);                             // инициализация SDL
    SDL_Window *window = SDL_CreateWindow("Game of Life", // создаем окно
                                          SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                          window_width, window_height, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0); // создаем рендер для window, для дальнейшей отрисовки

                                              // время задержки(паузы) между обновлением симуляции в мс
    current_config.simulation_start = SDL_GetTicks(); // начальное время старта симуляции, get_ticks возвращает вреся в мс, прошедщее с инициализации
    current_config.paused_time = 0;

    SDL_Event event; // создаём структуру, хранящую  события
    while (current_config.running)
    {
        SDL_GetMouseState(&mouse_x, &mouse_y); //кооринаты курсора 
        hover_x = mouse_x / CELL_SIZE; //переводим в п/н клетки в массиве 
        hover_y = mouse_y / CELL_SIZE;

        event_listener(&event, grid, hover_x, hover_y, &current_config); //обработчик событий 

        draw_grid(renderer, grid, hover_x, hover_y, current_config); //рендер нужен всегда, даже на паузе

        if (!current_config.paused)
        {
            update_grid(grid, new_grid, current_config.ROWS, current_config.COLS);
            current_config.generation++;
        }


        if (!current_config.paused)
        {
            current_config.elapsed = (SDL_GetTicks() - current_config.simulation_start) / 1000;
            //printf("\r%-60s", ""); //преложил чат gpt, потому что если загрузить симуляцию из файла, время отобрадается некорректно. Если добавить stdin_clear(), программа зависает. 
            printf("\rВремя симуляции: %u сек   Поколение: %u", current_config.elapsed, current_config.generation);
            fflush(stdout);
            SDL_Delay(current_config.delay);
        }
    }

    printf("\nЗавершение...\n");

    free_grid(&grid, &new_grid, current_config.ROWS);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
