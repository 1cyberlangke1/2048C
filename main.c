#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
/*
    一共4*4=16个格子, 一个格子12种状态
    状态存储: 2^64 / 16^12 ~= 99.7 (int64状态) / (游戏棋盘所有情况)
    还能存99个状态, 也就是种子
    大概像下面这样存储
    [-----高16位是12进制数存棋盘-----][最低位是99进制存种子]
*/

uint8_t
    board[16],   // 棋盘
    rng;         // 种子
uint64_t STATE;  // 状态, "状态压缩"

// 输入初始状态, 0就是不输入
#define INPUT_STATE 0

const char* texts[] = {  // 显示的字符串
    "   0  ", "   2  ", "   4  ", "   8  ",
    "  16  ", "  32  ", "  64  ", "  128 ",
    "  256 ", "  512 ", " 1024 ", " 2048 "};

// 把游戏状态编码
void cereal(void) {
    STATE = 0;
    for (int8_t i = 15; i >= 0; --i) STATE = STATE * 12 + board[i];
    STATE = STATE * 99 + (rng % 99);
}

// 把游戏状态解码
void decereal(uint64_t val) {
    rng = val % 99;
    val /= 99;
    for (uint8_t i = 0; i < 16; ++i) {
        board[i] = val % 12;
        val /= 12;
    }
}

// 更新随机数 线性同余
void rand__owo() {
    rng = (34 * rng + 40) % 99;
}

// 空白位置输出新数字
void open__uwu() {
    uint8_t
        pos[16],  // 空位置
        cnt = 0;  // 有几个空位置
    for (uint8_t i = 0; i < 16; ++i) {
        if (!board[i]) pos[cnt++] = i;
    }
    if (cnt) {
        rand__owo();
        uint8_t new_num = pos[rng % cnt];
        rand__owo();
        board[new_num] = ((rng % 10) < 9) ? 1 : 2;
    }
}

// 旋转棋盘90°
void rotate_board(void) {
    uint8_t tmp[16];
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            tmp[j * 4 + 3 - i] = board[i * 4 + j];
        }
    }
    for (uint8_t i = 0; i < 16; ++i) board[i] = tmp[i];
}

uint8_t squish(void) {
    uint8_t squished = 0;
    uint8_t tmp[4];

    for (uint8_t j = 0; j < 4; j++) {
        // 压缩空格
        uint8_t idx = 0;
        for (uint8_t i = 0; i < 4; i++) {
            if (board[i * 4 + j]) {
                tmp[idx++] = board[i * 4 + j];
            }
        }
        for (uint8_t i = 0; i < 4; i++) {
            uint8_t old = board[i * 4 + j];
            board[i * 4 + j] = (i < idx) ? tmp[i] : 0;
            if (old != board[i * 4 + j]) squished = 1;
        }

        // 合并相同数字
        uint8_t merged = 0;
        if (board[j] && board[j] == board[j + 4] &&
            board[j + 8] && board[j + 8] == board[j + 12]) {
            board[j]++;
            board[j + 4] = board[j + 8] + 1;
            board[j + 8] = 0;
            board[j + 12] = 0;
            merged = 1;
        } else if (board[j] && board[j] == board[j + 4]) {
            board[j]++;
            board[j + 4] = board[j + 8];
            board[j + 8] = board[j + 12];
            board[j + 12] = 0;
            merged = 1;
        } else if (board[j + 4] && board[j + 4] == board[j + 8]) {
            board[j + 4]++;
            board[j + 8] = board[j + 12];
            board[j + 12] = 0;
            merged = 1;
        } else if (board[j + 8] && board[j + 8] == board[j + 12]) {
            board[j + 8]++;
            board[j + 12] = 0;
            merged = 1;
        }

        squished |= merged;
    }
    return squished;
}

// 处理移动
uint8_t move_board(uint8_t input) {
    uint8_t moved = 0;
    switch (input) {
    case 'w':  // 上
        moved = squish();
        break;
    case 'a':  // 左
        rotate_board();
        moved = squish();
        rotate_board();
        rotate_board();
        rotate_board();
        break;
    case 's':  // 下
        rotate_board();
        rotate_board();
        moved = squish();
        rotate_board();
        rotate_board();
        break;
    case 'd':  // 右
        rotate_board();
        rotate_board();
        rotate_board();
        moved = squish();
        rotate_board();
        break;
    }
    return moved;
}

// 绘制棋盘
void draw_board(void) {
    for (uint8_t i = 0; i < 4; ++i) {
        for (uint8_t j = 0; j < 4; ++j) {
            printf("%s", texts[board[i * 4 + j]]);
        }
        putchar('\n');
    }
    cereal();
    printf("STATE = %" PRIu64 " RNG = %" PRIu8 "\n", STATE, rng);
}

// 游戏结束判断
uint8_t check_end(void) {
    // 判断是不是有2048
    for (uint8_t i = 0; i < 16; ++i) {
        if (board[i] == 11) {
            printf("You WIN! OwO\n");
            return 1;
        }
    }

    uint8_t tmp[16];
    for (uint8_t i = 0; i < 16; ++i) tmp[i] = board[i];
    if (move_board('w') || move_board('a') ||
        move_board('s') || move_board('d')) {
        for (uint8_t i = 0; i < 16; ++i)
            board[i] = tmp[i];
        return 0;
    }
    printf("You LOSE! QwQ\n");
    return 1;
}

// 初始化
void init(void) {
    for (int8_t i = 0; i < 16; ++i) board[i] = 0;
    time_t seed;
    time(&seed);  // 直接拿时间当种子
    rng = seed % 99;
    STATE = 0;

    open__uwu();
    open__uwu();

#if INPUT_STATE
    decereal(INPUT_STATE);

#endif
}

int main(void) {
    init();
    printf("press wasd to move\n");
    while (1) {
        draw_board();
        if (check_end()) {
            break;
        }

        uint8_t input = getchar();
        if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
            getchar();
            if (move_board(input)) {
                open__uwu();
            }
        }
    }
    return 0;
}