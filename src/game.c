#include "game.h"

#include <raylib.h>
#include <stdlib.h>

typedef enum { STATE_TITLE, STATE_GAMEPLAY, STATE_GAMEOVER } GameState;
typedef enum { DIR_UP, DIR_DOWN, DIR_RIGHT, DIR_LEFT } Direction;

typedef struct {
    int x, y;
} Position;

typedef struct {
    Position *body_pos;
    Position *head_pos;
    int curr_length, max_length;
    Direction current_dir;
    Direction buffered_dir;
    float movement_timer;
    bool is_growing;
} Snake;

typedef struct {
    bool is_running;
    GameState current_state;
    int grid_width;
    int grid_height;
    Snake snake;
    Position food_pos;
} Game;

static const int window_width = 1280;
static const int window_height = 720;
static const char *game_name = "raysnake";
static const float cell_size = ((window_width > window_height) ? window_height : window_width) * 0.125f;
static const float snake_vel = 6;

static Game game;

void InitSnake(Snake *s, int max_length)
{
    s->body_pos = malloc(max_length * sizeof(Position));
    if (s->body_pos == NULL) {
        TraceLog(LOG_ERROR, "InitSnake error: fail in memory allocation");
        exit(EXIT_FAILURE);
    }
    s->body_pos[0] = (Position){0, 0};
    s->head_pos = &s->body_pos[0];
    s->curr_length = 1;
    s->max_length = max_length;

    s->current_dir = DIR_RIGHT;
    s->buffered_dir = s->current_dir;

    s->movement_timer = 0.0f;
    s->is_growing = false;
}

void ResetSnake(Snake *s)
{
    s->curr_length = 1;
    s->body_pos[0] = (Position){0, 0};
    s->current_dir = DIR_RIGHT;
    s->buffered_dir = DIR_RIGHT;
    s->movement_timer = 0.0f;
    s->is_growing = false;
}

Position GetRandomFoodPosition(Snake *s, int grid_width, int grid_height)
{
    Position food_pos;
    bool is_valid = false;
    while (!is_valid) {
        food_pos = (Position){GetRandomValue(0, grid_width - 1), GetRandomValue(0, grid_height - 1)};

        is_valid = true;
        for (int i = 0; i < s->curr_length; i++) {
            if ((food_pos.x == s->body_pos[i].x) && (food_pos.y == s->body_pos[i].y)) {
                is_valid = false;
                break;
            }
        }
    }
    return food_pos;
}

void InitGame()
{
    game.grid_width = (window_width / cell_size) - 1;
    game.grid_height = (window_height / cell_size) - 1;
    if (game.grid_width < 1 || game.grid_height < 1) {
        TraceLog(LOG_FATAL, "InitGame fatal error: grid dimensions not valid (choosed resolution %dx%d not valid", window_width,
                 window_height);
    }

    InitSnake(&game.snake, game.grid_width * game.grid_height);
    game.food_pos = GetRandomFoodPosition(&game.snake, game.grid_width, game.grid_height);

    game.is_running = true;
    game.current_state = STATE_TITLE;

    SetTraceLogLevel(LOG_WARNING);
    InitWindow(window_width, window_height, game_name);
    SetTargetFPS(60);
}

void ResetGame()
{
    ResetSnake(&game.snake);
    game.food_pos = GetRandomFoodPosition(&game.snake, game.grid_width, game.grid_height);
}

void CloseGame()
{
    CloseWindow();

    free(game.snake.body_pos);
}

void UpdateSnake(Snake *s)
{
    // Snake growing
    if (s->is_growing) {
        s->body_pos[s->curr_length] = s->body_pos[s->curr_length - 1];
    }

    // Move snake body
    for (int i = s->curr_length - 1; i > 0; i--) {
        s->body_pos[i] = s->body_pos[i - 1];
    }

    // Update snake length
    if (s->is_growing) {
        s->curr_length++;
        s->is_growing = false;
    }

    // Move head in correct direction
    int *px = &s->head_pos->x, *py = &s->head_pos->y;
    switch (s->current_dir) {
        case DIR_UP:
            if (*py > 0)
                (*py)--;
            else
                game.current_state = STATE_GAMEOVER;
            break;
        case DIR_DOWN:
            if (*py < game.grid_height - 1)
                (*py)++;
            else
                game.current_state = STATE_GAMEOVER;
            break;
        case DIR_RIGHT:
            if (*px < game.grid_width - 1)
                (*px)++;
            else
                game.current_state = STATE_GAMEOVER;
            break;
        case DIR_LEFT:
            if (*px > 0)
                (*px)--;
            else
                game.current_state = STATE_GAMEOVER;
            break;
    }

    if (game.current_state == STATE_GAMEOVER) return;

    // Check collision with body
    for (int i = 1; i < s->curr_length; i++) {
        if ((s->head_pos->x == s->body_pos[i].x) && (s->head_pos->y == s->body_pos[i].y)) {
            game.current_state = STATE_GAMEOVER;
            return;
        }
    }
}

void UpdateTitle(int key)
{
    if (key != 0) game.current_state = STATE_GAMEPLAY;
}

void UpdateGameplay(int key)
{
    Snake *snake = &game.snake;
    if (key == KEY_R) {
        ResetGame();
        return;
    }

    if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && snake->current_dir != DIR_DOWN) {
        snake->buffered_dir = DIR_UP;
    } else if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) && snake->current_dir != DIR_UP) {
        snake->buffered_dir = DIR_DOWN;
    } else if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && snake->current_dir != DIR_LEFT) {
        snake->buffered_dir = DIR_RIGHT;
    } else if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && snake->current_dir != DIR_RIGHT) {
        snake->buffered_dir = DIR_LEFT;
    }

    // Snake movement
    float delta_time = GetFrameTime();
    snake->movement_timer += delta_time;
    if (snake->movement_timer >= 1 / snake_vel) {
        snake->current_dir = snake->buffered_dir;
        snake->movement_timer = 0.0f;
        UpdateSnake(snake);
    }

    // Check food collision
    if ((snake->head_pos->x == game.food_pos.x) && (snake->head_pos->y == game.food_pos.y)) {
        game.food_pos = GetRandomFoodPosition(snake, game.grid_width, game.grid_height);
        snake->is_growing = true;
    }
}

void UpdateGameOver(int key)
{
    if (key == KEY_SPACE) {
        ResetGame();
        game.current_state = STATE_GAMEPLAY;
    }
}

void UpdateGame()
{
    int key = GetKeyPressed();
    game.is_running = !WindowShouldClose() && key != KEY_Q;

    switch (game.current_state) {
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
    Rectangle title_rec;
    title_rec.height = window_height * 0.25f;
    title_rec.width = MeasureText(game_name, title_rec.height);
    title_rec.x = (window_width - title_rec.width) * 0.5f;
    title_rec.y = (window_height - title_rec.height) * 0.5f;

    const char *key_text = "Press any key to continue";
    Rectangle key_rec;
    key_rec.height = title_rec.height * 0.25f;
    key_rec.width = MeasureText(key_text, key_rec.height);
    key_rec.x = (window_width - key_rec.width) * 0.5f;
    key_rec.y = title_rec.y + title_rec.height * 1.25f;

    DrawText(game_name, title_rec.x + 2, title_rec.y + 6, title_rec.height + 1, DARKGRAY);
    DrawText(game_name, title_rec.x, title_rec.y, title_rec.height, WHITE);
    DrawText(key_text, key_rec.x, key_rec.y, key_rec.height, DARKGRAY);

    ClearBackground(BLACK);
}

void DrawGameplay()
{
    Vector2 grid_offset = {(window_width - (game.grid_width * cell_size)) * 0.5f,
                           ((window_height - (game.grid_height * cell_size)) * 0.5f)};

    float food_size = cell_size * 0.6f;
    float food_offset = (cell_size - food_size) * 0.5f;

    ClearBackground(BLACK);

    // Grid
    for (int i = 1; i < game.grid_width; i++) {
        DrawLineEx((Vector2){grid_offset.x + i * cell_size, grid_offset.y},
                   (Vector2){grid_offset.x + i * cell_size, grid_offset.y + cell_size * game.grid_height}, 2, GRAY);
    }
    for (int i = 1; i < game.grid_height; i++) {
        DrawLineEx((Vector2){grid_offset.x, grid_offset.y + i * cell_size},
                   (Vector2){grid_offset.x + cell_size * game.grid_width, grid_offset.y + i * cell_size}, 2, GRAY);
    }

    // Snake
    Snake *snake = &game.snake;
    Rectangle head_rec = {snake->head_pos->x * cell_size + grid_offset.x, snake->head_pos->y * cell_size + grid_offset.y, cell_size,
                          cell_size};

    for (int i = 1; i < snake->curr_length; i++) {
        Rectangle curr_body_rec = {snake->body_pos[i].x * cell_size + grid_offset.x, snake->body_pos[i].y * cell_size + grid_offset.y,
                                   cell_size, cell_size};
        DrawRectangleRec(curr_body_rec, (i % 2) ? SKYBLUE : BLUE);
    }

    DrawRectangleRec(head_rec, (game.current_state == STATE_GAMEOVER) ? RED : BLUE);
    DrawRectangleLinesEx(head_rec, 6, (game.current_state == STATE_GAMEOVER) ? DARKGRAY : DARKBLUE);

    // Food
    DrawRectangle(game.food_pos.x * cell_size + grid_offset.x + food_offset, game.food_pos.y * cell_size + grid_offset.y + food_offset,
                  food_size, food_size, GREEN);

    // Grid border
    DrawRectangleLinesEx((Rectangle){grid_offset.x, grid_offset.y, game.grid_width * cell_size, game.grid_height * cell_size}, 4, WHITE);
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
    DrawText(gameover_txt, gameover_rec.x, gameover_rec.y, gameover_rec.height, WHITE);

    for (int i = 0; i < 2; i++) DrawText(keys_txt[i], keys_rec[i].x, keys_rec[i].y, keys_rec[i].height, WHITE);
}

void DrawGame()
{
    BeginDrawing();
    switch (game.current_state) {
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

bool IsGameRunning()
{
    return game.is_running;
}
