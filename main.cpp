#include <iostream>
#include <vector>
#include <coroutine>
#include <random>

// Структура сопрограми
struct PlayerTask {
    struct promise_type {
        int move = -1;
        PlayerTask get_return_object() { return {std::coroutine_handle<promise_type>::from_promise(*this)}; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(int m) noexcept { move = m; return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
    std::coroutine_handle<promise_type> handle;
    ~PlayerTask() { if (handle) handle.destroy(); }
    int get_move() { return handle.promise().move; }
};

// Клас ігрового поля
class Board {
    char grid[3][3] = { {' ',' ',' '},{' ',' ',' '},{' ',' ',' '} };
public:
    void print() const {
        std::cout << "\n-------------\n";
        for (int i = 0; i < 3; ++i) {
            std::cout << "| ";
            for (int j = 0; j < 3; ++j) std::cout << grid[i][j] << " | ";
            std::cout << "\n-------------\n";
        }
    }

    bool make_move(int pos, char symbol) {
        int r = pos / 3, c = pos % 3;
        if (pos < 0 || pos > 8 || grid[r][c] != ' ') return false;
        grid[r][c] = symbol;
        return true;
    }

    bool is_full() const {
        for (auto& row : grid)
            for (char cell : row) if (cell == ' ') return false;
        return true;
    }

    bool check_win(char s) const {
        for (int i = 0; i < 3; ++i) {
            if ((grid[i][0] == s && grid[i][1] == s && grid[i][2] == s) ||
                (grid[0][i] == s && grid[1][i] == s && grid[2][i] == s)) return true;
        }
        return (grid[0][0] == s && grid[1][1] == s && grid[2][2] == s) ||
               (grid[0][2] == s && grid[1][1] == s && grid[2][0] == s);
    }
};

// Сопрограма гравця, що генерує ходи
PlayerTask player_coroutine(char symbol) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 8);
    while (true) {
        co_yield dis(gen); // Повертаємо випадковий хід
    }
}

int main() {
    std::system("chcp 65001 > nul");

    Board board;
    auto pX = player_coroutine('X');
    auto pO = player_coroutine('O');

    char current_sym = 'X';
    std::cout << "=== Гра Хрестики-нолики (Сопрограми C++20) ===\n";

    while (true) {
        board.print();
        
        auto& active_player = (current_sym == 'X') ? pX : pO;
        active_player.handle.resume(); // Відновлюємо сопрограму
        int move = active_player.get_move();

        if (!board.make_move(move, current_sym)) continue; // Повтор якщо зайнято

        std::cout << "Гравець " << current_sym << " робить хід у клітинку: " << move << "\n";

        if (board.check_win(current_sym)) {
            board.print();
            std::cout << "\n Переможець: Гравець " << current_sym << "!\n";
            break;
        }

        if (board.is_full()) {
            board.print();
            std::cout << "\n Нічия!\n";
            break;
        }

        current_sym = (current_sym == 'X') ? 'O' : 'X';
    }

    return 0;
}