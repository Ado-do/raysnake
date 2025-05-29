#include "game.h"

#include <raylib.h>
#include <stdlib.h>

typedef enum { STATE_TITLE, STATE_GAMEPLAY, STATE_GAMEOVER } GameState;
typedef enum { DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT } Direction;

typedef struct {
    Vector2 *pos;
    Vector2 *head_pos;
    int curr_length, max_length;
    Direction current_dir;
} Snake;

typedef struct {
    bool is_running;
    GameState current_state;
    int grid_width;
    int grid_height;
    Snake snake;
    Vector2 food_pos;
} Game;

static const int window_width = 1280;
static const int window_height = 720;
static const char *game_name = "raysnake";
static const float cell_size = window_height / 12.0f;
static const float snake_mov_delay_s = 0.25f;

static Game g;

void InitSnake(Snake *s, int max_length)
{
    s->pos = malloc(max_length * sizeof(Vector2));
    s->pos[0] = (Vector2){0, 0};
    s->head_pos = &s->pos[0];
    s->curr_length = 1;
    s->max_length = max_length;
    s->current_dir = DIR_RIGHT;
}

Vector2 GetRandomFoodPosition(Snake *s, int grid_width, int grid_height)
{
    Vector2 pos;
    bool is_valid = false;
    while (!is_valid) {
        pos = (Vector2){GetRandomValue(0, grid_width-1), GetRandomValue(0, grid_height-1)};
        for (int i = 0; i < s->curr_length; i++) {
            if ((pos.x != s->pos[i].x) && (pos.y != s->pos[i].y)) {
                is_valid = true;
                break;
            }
        }
    }
    return pos;
}

void InitGame()
{
    g.grid_width = (window_width / cell_size) - 1;
    g.grid_height = (window_height / cell_size) - 1;

    InitSnake(&g.snake, g.grid_width * g.grid_height);
    g.food_pos = GetRandomFoodPosition(&g.snake, g.grid_width, g.grid_height);

    g.is_running = true;
    g.current_state = STATE_TITLE;

    InitWindow(window_width, window_height, game_name);
    SetTargetFPS(60);
}

void UpdateSnake(Snake *s)
{
    float *px = &s->head_pos->x, *py = &s->head_pos->y;
    switch (s->current_dir) {
        case DIR_UP:
            (*py)--;
            break;
        case DIR_DOWN:
            (*py)++;
            break;
        case DIR_RIGHT:
            (*px)++;
            break;
        case DIR_LEFT:
            (*px)--;
            break;
    }
}

void UpdateTitle(int key)
{
    if (key != 0) g.current_state = STATE_GAMEPLAY;
}

void UpdateGameplay(int key)
{
    switch (key) {
        case KEY_W:
        case KEY_UP:
            if (g.snake.current_dir != DIR_DOWN) g.snake.current_dir = DIR_UP;
            break;
        case KEY_DOWN:
        case KEY_S:
            if (g.snake.current_dir != DIR_UP) g.snake.current_dir = DIR_DOWN;
            break;
        case KEY_RIGHT:
        case KEY_D:
            if (g.snake.current_dir != DIR_LEFT) g.snake.current_dir = DIR_RIGHT;
            break;
        case KEY_LEFT:
        case KEY_A:
            if (g.snake.current_dir != DIR_RIGHT) g.snake.current_dir = DIR_LEFT;
            break;
        default:
            break;
    }

    float delta_time = GetFrameTime();

    static float snake_current_move_cnt_s = 0.0f;
    snake_current_move_cnt_s += delta_time;

    if (snake_current_move_cnt_s >= snake_mov_delay_s) {
        UpdateSnake(&g.snake);
        snake_current_move_cnt_s = 0.0f;
    }

    if ((g.snake.head_pos->x < 0 || g.snake.head_pos->x >= g.grid_width) ||
        (g.snake.head_pos->y < 0 || g.snake.head_pos->y >= g.grid_height)) {
        g.current_state = STATE_GAMEOVER;
    }

    if ((g.snake.head_pos->x == g.food_pos.x) && (g.snake.head_pos->y == g.food_pos.y)) {
        // TODO: Snake grow
        g.food_pos = GetRandomFoodPosition(&g.snake, g.grid_width, g.grid_height);
    }
}

void UpdateGameOver(int key)
{
    if (key == KEY_Q) {
        g.is_running = false;
    } else if (key != 0) {
        g.current_state = STATE_GAMEPLAY;
    }
}

void UpdateGame()
{
    int key = GetKeyPressed();
    g.is_running = !WindowShouldClose() && key != KEY_Q;

    switch (g.current_state) {
        case STATE_TITLE:    UpdateTitle(key); break;
        case STATE_GAMEPLAY: UpdateGameplay(key); break;
        case STATE_GAMEOVER: UpdateGameOver(key); break;
    }
}

void DrawTitle()
{
    int w = window_width, h = window_height;

    float title_sz = h * 0.25f;
    float title_w = MeasureText(game_name, title_sz);
    float title_x = (w - title_w) * 0.5f;
    float title_y = (h - title_sz) * 0.5f;

    BeginDrawing();
    {
        DrawText(game_name, title_x + 2, title_y + 6, title_sz + 1, DARKGRAY);
        DrawText(game_name, title_x, title_y, title_sz, LIGHTGRAY);
        DrawText("TITLE", 0, 0, 30, RED);
        ClearBackground(BLACK);
    }
    EndDrawing();
}

void DrawGameplay()
{
    Vector2 grid_offset = {(window_width - (g.grid_width * cell_size)) * 0.5f, ((window_height - (g.grid_height * cell_size)) * 0.5f)};

    float food_size = cell_size * 0.6f;
    float food_offset = (cell_size - food_size) * 0.5f;

    Color c1 = GRAY, c2 = DARKGRAY;

    BeginDrawing();
    {
        ClearBackground(DARKGRAY);

        // Grid
        for (int i = 0; i < g.grid_height; i++) {
            Color tmp = c1;
            c1 = c2, c2 = tmp;
            for (int j = 0; j < g.grid_width; j++) {
                DrawRectangle(j * cell_size + grid_offset.x, i * cell_size + grid_offset.y, cell_size, cell_size, (j % 2) ? c1 : c2);
            }
        }

        // Snake
        DrawRectangle(g.snake.head_pos->x * cell_size + grid_offset.x, g.snake.head_pos->y * cell_size + grid_offset.y, cell_size, cell_size, BLUE);

        // Food
        DrawRectangle(g.food_pos.x * cell_size + grid_offset.x + food_offset, g.food_pos.y * cell_size + grid_offset.y + food_offset, food_size, food_size, GREEN);

        // Grid border
        DrawRectangleLinesEx((Rectangle){grid_offset.x, grid_offset.y, g.grid_width * cell_size, g.grid_height * cell_size}, 3, GRAY);

        DrawText("GAMEPLAY", 0, 0, 30, GREEN);
    }
    EndDrawing();
}

void DrawGameOver()
{
    BeginDrawing();
    {
        ClearBackground(BLACK);
        DrawText("GAMEOVER", 0, 0, 30, GREEN);
    }
    EndDrawing();
}

void DrawGame()
{
    switch (g.current_state) {
        case STATE_TITLE:    DrawTitle(); break;
        case STATE_GAMEPLAY: DrawGameplay(); break;
        case STATE_GAMEOVER: DrawGameOver(); break;
    }
}

void CloseGame()
{
    CloseWindow();
    free(g.snake.pos);
}

bool IsGameRunning()
{
    return g.is_running;
}
