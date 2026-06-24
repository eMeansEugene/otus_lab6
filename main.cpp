
#include "sparse_matrix.hpp"
#include <iostream>

int main() {
    SparseMatrix<int, 0> matrix;

    // главная диагональ
    for (int i = 0; i < 10; i++) {
        matrix[i][i] = i;
    }

    // второстепенная диагональ
    for (int i = 0; i < 10; i++) {
        matrix[i][9 - i] = 9 - i;
    }

    // фрагмент [1][1]..[8][8]
    for (int i = 1; i <= 8; ++i) {
        for (int j = 1; j <= 8; ++j) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << "\n";
    }

    // количество занятых ячеек
    std::cout << "size: " << matrix.size() << "\n";

    // все занятые ячейки
    for (auto c : matrix) {
        auto [x, y, v] = c;
        std::cout << x << " " << y << " " << v << "\n";
    }

    return 0;
}