#include <kernel/tty.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

#define BOARD_X_OFFSET 10  // Pozycja X planszy na ekranie
#define BOARD_Y_OFFSET 2   // Pozycja Y planszy na ekranie


static uint8_t board[BOARD_HEIGHT][BOARD_WIDTH];

// VGA kolory figur
enum {
    COLOR_I = 9, // light blue
    COLOR_O = 14, // yellow
    COLOR_T = 13, // magenta
    COLOR_J = 1, // blue
    COLOR_L = 6, // brown
    COLOR_S = 10, // green
    COLOR_Z = 12  // red
};


typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_SETTINGS
} GameState;

static GameState game_state = STATE_MENU;

static int menu_selected = 0;
static const char* menu_items[] = {
    "Play",
    "Best Score",
    "Settings"
};
#define MENU_ITEMS_COUNT (sizeof(menu_items)/sizeof(menu_items[0]))

static void draw_menu() {
    terminal_initialize();
    terminal_writestring("=== TETRIS ===\n\n");

    for (int i = 0; i < MENU_ITEMS_COUNT; i++) {
        if (i == menu_selected)
            terminal_writestring("> ");
        else
            terminal_writestring("  ");

        terminal_writestring(menu_items[i]);
        terminal_writestring("\n");
    }
}



static const uint8_t tetromino_colors[7] = {
    COLOR_I, COLOR_O, COLOR_T, COLOR_J, COLOR_L, COLOR_S, COLOR_Z
};

// 4 rotacje dla każdej figury (4x4)
static const uint8_t tetrominoes[7][4][4][4] = {
    // I
    {
        {
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0},
            {0,0,1,0}
        },
        {
            {0,0,0,0},
            {0,0,0,0},
            {1,1,1,1},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,1,0,0}
        }
    },
    // O
    {
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        // O rotacje wszystkie takie same
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0}
        }
    },
    // T
    {
        {
            {0,0,0,0},
            {1,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,1,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    // J
    {
        {
            {0,0,0,0},
            {1,1,1,0},
            {0,0,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {1,1,0,0},
            {0,0,0,0}
        },
        {
            {1,0,0,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,1,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    // L
    {
        {
            {0,0,0,0},
            {1,1,1,0},
            {1,0,0,0},
            {0,0,0,0}
        },
        {
            {1,1,0,0},
            {0,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,0,1,0},
            {1,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {0,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        }
    },
    // S
    {
        {
            {0,0,0,0},
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0}
        },
        {
            {1,0,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {0,1,1,0},
            {1,1,0,0},
            {0,0,0,0}
        },
        {
            {1,0,0,0},
            {1,1,0,0},
            {0,1,0,0},
            {0,0,0,0}
        }
    },
    // Z
    {
        {
            {0,0,0,0},
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {1,0,0,0},
            {0,0,0,0}
        },
        {
            {0,0,0,0},
            {1,1,0,0},
            {0,1,1,0},
            {0,0,0,0}
        },
        {
            {0,1,0,0},
            {1,1,0,0},
            {1,0,0,0},
            {0,0,0,0}
        }
    }
};

typedef struct {
    int x, y;
    int type;
    int rotation;
} Tetromino;

static Tetromino current_piece;
static int score = 0;
static bool paused = false;

static void draw_cell(int x, int y, uint8_t color) {
    terminal_putentryat(' ', (color << 4) | color, (x + BOARD_X_OFFSET)*2, y + BOARD_Y_OFFSET);
    terminal_putentryat(' ', (color << 4) | color, (x + BOARD_X_OFFSET)*2 + 1, y + BOARD_Y_OFFSET);
}

static void draw_border() {
    uint8_t color = 7; // szary
    int left = BOARD_X_OFFSET * 2 - 2;
    int right = (BOARD_X_OFFSET + BOARD_WIDTH) * 2;
    int top = BOARD_Y_OFFSET - 1;
    int bottom = BOARD_Y_OFFSET + BOARD_HEIGHT;

    for (int y = top; y <= bottom; y++) {
        terminal_putentryat('|', color, left, y);
        terminal_putentryat('|', color, right, y);
    }

    for (int x = left; x <= right; x++) {
        terminal_putentryat('-', color, x, top);
        terminal_putentryat('-', color, x, bottom);
    }

    terminal_putentryat('+', color, left, top);
    terminal_putentryat('+', color, right, top);
    terminal_putentryat('+', color, left, bottom);
    terminal_putentryat('+', color, right, bottom);
}



static void draw_board() {
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x]) {
                draw_cell(x, y, tetromino_colors[board[y][x]-1]);
            } else {
                draw_cell(x, y, 0); // czarny
            }
        }
    }
}

static bool check_collision(int x, int y, int type, int rotation) {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (tetrominoes[type][rotation][py][px]) {
                int bx = x + px;
                int by = y + py;
                if (bx < 0 || bx >= BOARD_WIDTH || by >= BOARD_HEIGHT) return true;
                if (by >= 0 && board[by][bx]) return true;
            }
        }
    }
    return false;
}

static void place_piece() {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            if (tetrominoes[current_piece.type][current_piece.rotation][py][px]) {
                int bx = current_piece.x + px;
                int by = current_piece.y + py;
                if (by >= 0 && by < BOARD_HEIGHT && bx >= 0 && bx < BOARD_WIDTH) {
                    board[by][bx] = current_piece.type + 1;
                }
            }
        }
    }
}

static void clear_lines() {
    int lines_cleared = 0;
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        bool full = true;
        for (int x = 0; x < BOARD_WIDTH; x++) {
            if (board[y][x] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            lines_cleared++;
            // przesuwamy planszę w dół
            for (int ty = y; ty > 0; ty--) {
                for (int tx = 0; tx < BOARD_WIDTH; tx++) {
                    board[ty][tx] = board[ty-1][tx];
                }
            }
            for (int tx = 0; tx < BOARD_WIDTH; tx++) {
                board[0][tx] = 0;
            }
        }
    }
    // punkty wg linii
    if (lines_cleared == 1) score += 100;
    else if (lines_cleared == 2) score += 300;
    else if (lines_cleared == 3) score += 700;
    else if (lines_cleared >= 4) score += 1500;
}

static void spawn_piece() {
    current_piece.type = rand() % 7;
    current_piece.rotation = 0;
    current_piece.x = 3;
    current_piece.y = -2; // start powyżej planszy
    if (check_collision(current_piece.x, current_piece.y, current_piece.type, current_piece.rotation)) {
        // Game Over
        terminal_initialize();
        terminal_writestring("Game Over!\n");
        terminal_writestring("Score: ");
        char buf[20];
        int n = 0;
        int sc = score;
        if (sc == 0) {
            terminal_putchar('0');
        } else {
            while (sc > 0) {
                buf[n++] = '0' + (sc % 10);
                sc /= 10;
            }
            for (int i = n-1; i >= 0; i--) terminal_putchar(buf[i]);
        }
        while (1);
    }
}

static void draw_score() {
    const char *prefix = "Score:";
    int x_start = (BOARD_X_OFFSET + BOARD_WIDTH + 2) * 2;
    int y_pos = BOARD_Y_OFFSET;

    for (int i = 0; prefix[i]; i++) {
        terminal_putentryat(prefix[i], 15, x_start + i, y_pos);
    }

    char buf[10];
    int n = 0;
    int sc = score;

    if (sc == 0) {
        terminal_putentryat('0', 15, x_start + 6, y_pos + 1);
    } else {
        while (sc > 0) {
            buf[n++] = '0' + (sc % 10);
            sc /= 10;
        }
        for (int i = 0; i < 10; i++) terminal_putentryat(' ', 15, x_start + i, y_pos + 1);
        for (int i = n-1; i >= 0; i--) terminal_putentryat(buf[i], 15, x_start + (n - 1 - i), y_pos + 1);
    }
}

static void draw_current_piece() {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            int bx = current_piece.x + px;
            int by = current_piece.y + py;
            if (bx < 0 || bx >= BOARD_WIDTH || by < 0 || by >= BOARD_HEIGHT) continue;

            if (tetrominoes[current_piece.type][current_piece.rotation][py][px]) {
                draw_cell(bx, by, tetromino_colors[current_piece.type]);
            } else if (board[by][bx] == 0) {
                draw_cell(bx, by, 0);
            }
        }
    }
}

static void clear_current_piece() {
    for (int py = 0; py < 4; py++) {
        for (int px = 0; px < 4; px++) {
            int bx = current_piece.x + px;
            int by = current_piece.y + py;
            if (bx < 0 || bx >= BOARD_WIDTH || by < 0 || by >= BOARD_HEIGHT) continue;
            if (tetrominoes[current_piece.type][current_piece.rotation][py][px]) {
                draw_cell(bx, by, 0);
            }
        }
    }
}

static void rotate_piece() {
    int new_rot = (current_piece.rotation + 1) % 4;
    if (!check_collision(current_piece.x, current_piece.y, current_piece.type, new_rot)) {
        current_piece.rotation = new_rot;
    }
}


static void move_piece(int dx, int dy) {
    if (!check_collision(current_piece.x + dx, current_piece.y + dy, current_piece.type, current_piece.rotation)) {
        current_piece.x += dx;
        current_piece.y += dy;
    }
}

static void drop_piece() {
    while (!check_collision(current_piece.x, current_piece.y + 1, current_piece.type, current_piece.rotation)) {
        current_piece.y++;
    }
    place_piece();
    clear_lines();
    spawn_piece();
}

void kernel_main(void) {
    terminal_initialize();

    for (int y = 0; y < BOARD_HEIGHT; y++)
        for (int x = 0; x < BOARD_WIDTH; x++)
            board[y][x] = 0;

    spawn_piece();

    int drop_timer = 0;
    const int drop_delay = 30; // zmniejsz/zwieksz na szybkość opadania

    while (1) {
        keyboard_update();

        if (key_state.p) paused = !paused;

        if (!paused) {
            // Sterowanie
            if (key_state.a) {
                move_piece(-1, 0);
                key_state.a = false;
            }
            if (key_state.d) {
                move_piece(1, 0);
                key_state.d = false;
            }
            if (key_state.s) {
                move_piece(0, 1);
                key_state.s = false;
            }
            if (key_state.w) {
                rotate_piece();
                key_state.w = false;
            }
            if (key_state.space) {
                drop_piece();
                key_state.space = false;
            }

            drop_timer++;
            if (drop_timer >= drop_delay) {
                drop_timer = 0;
                if (!check_collision(current_piece.x, current_piece.y + 1, current_piece.type, current_piece.rotation)) {
                    current_piece.y++;
                } else {
                    place_piece();
                    clear_lines();
                    spawn_piece();
                }
            }
        }
        draw_border();
        draw_board();
        draw_current_piece();
        draw_score();

        sleep(1); // krótka przerwa - możesz zmienić na krótszą
    }
}
