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
    float movement_timer;
    bool is_growing;
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
static const float cell_size = ((window_width > window_height) ? window_height : window_width) * 0.1f;
static const float snake_vel = 10;

static Game g;

void InitSnake(Snake *s, int max_length)
{
    s->pos = malloc(max_length * sizeof(Vector2));
    s->pos[0] = (Vector2){0, 0};
    s->head_pos = &s->pos[0];
    s->curr_length = 1;
    s->max_length = max_length;
    s->current_dir = DIR_RIGHT;
    s->movement_timer = 0.0f;
    s->is_growing = false;
}

void ResetSnake(Snake *s)
{
    s->curr_length = 1;
    s->pos[0] = (Vector2){0, 0};
    s->current_dir = DIR_RIGHT;
    s->movement_timer = 0.0f;
    s->is_growing = false;
}

Vector2 GetRandomFoodPosition(Snake *s, int grid_width, int grid_height)
{
    Vector2 pos;
    bool is_valid = false;
    while (!is_valid) {
        pos = (Vector2){GetRandomValue(0, grid_width - 1), GetRandomValue(0, grid_height - 1)};
        if ((pos.x == g.food_pos.x) && (pos.y == g.food_pos.y)) continue;

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

void ResetGame()
{
    ResetSnake(&g.snake);
    g.food_pos = GetRandomFoodPosition(&g.snake, g.grid_width, g.grid_height);
}

void UpdateSnake(Snake *s)
{
    // Snake growing
    if (s->is_growing) {
        s->pos[s->curr_length] = s->pos[s->curr_length-1];
    }

    // Move snake body
    for (int i = s->curr_length - 1; i > 0; i--) {
        s->pos[i] = s->pos[i-1];
    }

    if (s->is_growing) {
        s->curr_length++;
        s->is_growing = false;
    }

    // Move head in correct direction
    float *px = &s->head_pos->x, *py = &s->head_pos->y;
    switch (s->current_dir) {
        case DIR_UP:
            if (*py > 0) (*py)--;
            else g.current_state = STATE_GAMEOVER;
            break;
        case DIR_DOWN:
            if (*py < g.grid_height - 1) (*py)++;
            else g.current_state = STATE_GAMEOVER;
            break;
        case DIR_RIGHT:
            if (*px < g.grid_width - 1) (*px)++;
            else g.current_state = STATE_GAMEOVER;
            break;
        case DIR_LEFT:
            if (*px > 0) (*px)--;
            else g.current_state = STATE_GAMEOVER;
            break;
    }

    if (g.current_state == STATE_GAMEOVER) return;

    // Check collision with body
    for (int i = 1; i < s->curr_length; i++) {
        if ((s->head_pos->x == s->pos[i].x) && (s->head_pos->y == s->pos[i].y)) {
            g.current_state = STATE_GAMEOVER;
            return;
        }
    }
}

void UpdateTitle(int key)
{
    if (key != 0) g.current_state = STATE_GAMEPLAY;
}

void UpdateGameplay(int key)
{
    Snake *s = &g.snake;

    switch (key) {
        case KEY_W:
        case KEY_UP:
            if (s->current_dir != DIR_DOWN) s->current_dir = DIR_UP;
            break;
        case KEY_DOWN:
        case KEY_S:
            if (s->current_dir != DIR_UP) s->current_dir = DIR_DOWN;
            break;
        case KEY_RIGHT:
        case KEY_D:
            if (s->current_dir != DIR_LEFT) s->current_dir = DIR_RIGHT;
            break;
        case KEY_LEFT:
        case KEY_A:
            if (s->current_dir != DIR_RIGHT) s->current_dir = DIR_LEFT;
            break;
        case KEY_R: ResetGame(); break;
        default:
            break;
    }

    float delta_time = GetFrameTime();

    s->movement_timer += delta_time;

    if (s->movement_timer >= 1 / snake_vel) {
        UpdateSnake(s);
        s->movement_timer = 0.0f;
    }

    // Check food collision
    if ((s->head_pos->x == g.food_pos.x) && (s->head_pos->y == g.food_pos.y)) {
        s->is_growing = true;
        g.food_pos = GetRandomFoodPosition(s, g.grid_width, g.grid_height);
    }
}

void UpdateGameOver(int key)
{
    if (key == KEY_SPACE) {
        ResetGame();
        g.current_state = STATE_GAMEPLAY;
    }
}

void UpdateGame()
{
    int key = GetKeyPressed();
    g.is_running = !WindowShouldClose() && key != KEY_Q;

    switch (g.current_state) {
        case STATE_TITLE:
            UpdateTitle(key);
            break;
        case STATE_GAMEPLAY:
            UpdateGameplay(key);
            break;
        case STATE_GAMEOVER:
            UpdateGameOver(key);
            break;
    }
}

void DrawTitle()
{
    int w = window_width, h = window_height;

    float title_sz = h * 0.25f;
    float title_w = MeasureText(game_name, title_sz);
    float title_x = (w - title_w) * 0.5f;
    float title_y = (h - title_sz) * 0.5f;

    DrawText(game_name, title_x + 2, title_y + 6, title_sz + 1, DARKGRAY);
    DrawText(game_name, title_x, title_y, title_sz, LIGHTGRAY);
    ClearBackground(BLACK);
}

void DrawGameplay()
{
    Vector2 grid_offset = {(window_width - (g.grid_width * cell_size)) * 0.5f, ((window_height - (g.grid_height * cell_size)) * 0.5f)};

    float food_size = cell_size * 0.6f;
    float food_offset = (cell_size - food_size) * 0.5f;

    ClearBackground(BLACK);

    DrawText(TextFormat("Grid: w = %d, h = %d | Snake: curr_length = %d", g.grid_width, g.grid_height, g.snake.curr_length), 0, 0, 25, DARKGRAY);

    // Grid
    for (int i = 1; i < g.grid_width; i++) {
        DrawLineEx((Vector2){grid_offset.x + i * cell_size, grid_offset.y},
                   (Vector2){grid_offset.x + i * cell_size, grid_offset.y + cell_size * g.grid_height}, 1, GRAY);
    }
    for (int i = 1; i < g.grid_height; i++) {
        DrawLineEx((Vector2){grid_offset.x, grid_offset.y + i * cell_size},
                   (Vector2){grid_offset.x + cell_size * g.grid_width, grid_offset.y + i * cell_size}, 1, RAYWHITE);
    }

    // Snake
    Snake *s = &g.snake;
    for (int i = 1; i < s->curr_length; i++)
        DrawRectangle(s->pos[i].x * cell_size + grid_offset.x, s->pos[i].y * cell_size + grid_offset.y, cell_size, cell_size, SKYBLUE);

    DrawRectangle(s->head_pos->x * cell_size + grid_offset.x, s->head_pos->y * cell_size + grid_offset.y, cell_size,
                  cell_size, BLUE);

    // Food
    DrawRectangle(g.food_pos.x * cell_size + grid_offset.x + food_offset, g.food_pos.y * cell_size + grid_offset.y + food_offset,
                  food_size, food_size, GREEN);

    // Grid border
    DrawRectangleLinesEx((Rectangle){grid_offset.x, grid_offset.y, g.grid_width * cell_size, g.grid_height * cell_size}, 3, WHITE);
}

void DrawGameOver()
{
    const char *gameover_txt = "Game Over";
    const char *keys_txt[] = {"Press SPACE to continue", "Press Q to quit"};

    Rectangle gameover_rec;
    gameover_rec.height = window_height * 0.25f;
    gameover_rec.width = MeasureText(gameover_txt, gameover_rec.height);
    gameover_rec.x = (window_width - gameover_rec.width) * 0.5f;
    gameover_rec.y = (window_height - gameover_rec.height) * 0.5f;

    Rectangle keys_rec[2];
    keys_rec[0].height = window_height * 0.05f;
    keys_rec[0].width = MeasureText(keys_txt[0], keys_rec[0].height);
    keys_rec[0].x = (window_width - keys_rec[0].width) * 0.5f;
    keys_rec[0].y = gameover_rec.y + gameover_rec.height;
    keys_rec[1].height = keys_rec[0].height;
    keys_rec[1].width = MeasureText(keys_txt[1], keys_rec[1].height);
    keys_rec[1].x = (window_width - keys_rec[1].width) * 0.5f;
    keys_rec[1].y = keys_rec[0].y + keys_rec[0].height * 1.2f;

    DrawText(gameover_txt, gameover_rec.x + 2, gameover_rec.y + 6, gameover_rec.height + 1, DARKGRAY);
    DrawText(gameover_txt, gameover_rec.x, gameover_rec.y, gameover_rec.height, LIGHTGRAY);

    for (int i = 0; i < 2; i++) DrawText(keys_txt[i], keys_rec[i].x, keys_rec[i].y, keys_rec[i].height, DARKGRAY);
}

void DrawGame()
{
    BeginDrawing();
    switch (g.current_state) {
        case STATE_TITLE:
            DrawTitle();
            break;
        case STATE_GAMEPLAY:
            DrawGameplay();
            break;
        case STATE_GAMEOVER:
            DrawGameplay();
            DrawGameOver();
            break;
    }
    EndDrawing();
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
