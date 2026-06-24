/**
 * @file SparseMatrix.hpp
 * @brief Двумерная разреженная бесконечная матрица на основе паттерна Proxy.
 *
 * Матрица хранит только ячейки, которым было явно присвоено значение,
 * отличное от значения по умолчанию. Присвоение значения по умолчанию
 * освобождает ячейку.
 *
 * @tparam T    Тип элементов матрицы.
 * @tparam Default Значение по умолчанию (возвращается для свободных ячеек).
 *
 * @example
 * @code
 * SparseMatrix<int, -1> matrix;
 * matrix[0][0] = 42;
 * int val = matrix[0][0]; // 42
 * int def = matrix[5][5]; // -1, ячейка не создаётся
 * std::cout << matrix.size(); // 1
 * @endcode
 */

#ifndef OTUS_LAB6_SPARSEMATRIX_HPP
#define OTUS_LAB6_SPARSEMATRIX_HPP

#include <cstddef>
#include <map>
#include <tuple>
#include <utility>

template <typename T, T Default>
class SparseMatrix;

namespace detail {

/**
 * @brief Proxy-объект для доступа к отдельной ячейке матрицы.
 *
 * Реализует паттерн Proxy (Заместитель): перехватывает операции
 * чтения и записи, позволяя различить их без немедленного обращения
 * к хранилищу.
 *
 * - При записи (`operator=`) вставляет или удаляет элемент из map.
 * - При чтении (`operator T`) возвращает значение или Default.
 *
 * @tparam T       Тип элементов матрицы.
 * @tparam Default Значение по умолчанию.
 */
template <typename T, T Default>
struct CellProxy {
    SparseMatrix<T, Default>& matrix; ///< Ссылка на матрицу-владелец.
    int row_;                          ///< Строка ячейки.
    int col_;                          ///< Столбец ячейки.

    /**
     * @brief Записывает значение в ячейку.
     *
     * Если @p val равен Default — ячейка освобождается (удаляется из map).
     * Возвращает `*this` для поддержки цепочек присваивания:
     * `((matrix[0][0] = 5) = 0) = 3`.
     *
     * @param val Значение для записи.
     * @return Ссылка на себя.
     */
    CellProxy& operator=(T val) {
        if (val == Default) {
            matrix.data_.erase({row_, col_});
        } else {
            matrix.data_[{row_, col_}] = val;
        }
        return *this;
    }

    /**
     * @brief Неявное преобразование к типу T (чтение ячейки).
     *
     * Если ячейка существует в хранилище — возвращает её значение.
     * Иначе возвращает Default без создания ячейки.
     *
     * @return Значение ячейки или Default.
     */
    operator T() const {
        auto it = matrix.data_.find({row_, col_});
        if (it != matrix.data_.end()) {
            return it->second;
        }
        return Default;
    }
};

/**
 * @brief Proxy-объект для доступа к строке матрицы.
 *
 * Промежуточный объект, возвращаемый `SparseMatrix::operator[]`.
 * Хранит номер строки и ссылку на матрицу; при повторном `operator[]`
 * создаёт CellProxy для конкретной ячейки.
 *
 * @tparam T       Тип элементов матрицы.
 * @tparam Default Значение по умолчанию.
 */
template <typename T, T Default>
struct RowProxy {
    SparseMatrix<T, Default>& matrix; ///< Ссылка на матрицу-владелец.
    int row_;                          ///< Номер строки.

    /**
     * @brief Возвращает Proxy для ячейки в заданном столбце.
     *
     * @param col Номер столбца.
     * @return CellProxy для ячейки (row_, col).
     */
    CellProxy<T, Default> operator[](int col) {
        return {matrix, row_, col};
    }
};

} // namespace detail

/**
 * @brief Двумерная разреженная бесконечная матрица.
 *
 * Хранит только занятые ячейки в `std::map`. Индексация через
 * цепочку Proxy-объектов позволяет корректно различать чтение и запись:
 * чтение свободной ячейки не создаёт запись в хранилище.
 *
 * Поддерживает range-based for — итерация по занятым ячейкам
 * возвращает `std::tuple<int, int, T>` (row, col, value).
 *
 * @tparam T       Тип элементов матрицы.
 * @tparam Default Значение по умолчанию (по умолчанию T{}).
 */
template <typename T, T Default = T{}>
class SparseMatrix {
    friend struct detail::CellProxy<T, Default>;

public:
    /**
     * @brief Итератор по занятым ячейкам матрицы.
     *
     * Оборачивает итератор внутреннего `std::map` и преобразует
     * каждый элемент в `std::tuple<int, int, T>`.
     */
    struct Iterator {
        typename std::map<std::pair<int, int>, T>::iterator it_; ///< Итератор map.

        /**
         * @brief Разыменование — возвращает (row, col, value).
         * @return Кортеж с координатами и значением ячейки.
         */
        std::tuple<int, int, T> operator*() {
            return {it_->first.first, it_->first.second, it_->second};
        }

        /**
         * @brief Переход к следующей занятой ячейке.
         * @return Ссылка на себя.
         */
        Iterator& operator++() {
            ++it_;
            return *this;
        }

        /**
         * @brief Сравнение итераторов.
         * @param other Итератор для сравнения.
         * @return true если итераторы не равны.
         */
        bool operator!=(const Iterator& other) const {
            return it_ != other.it_;
        }
    };

    /**
     * @brief Возвращает Proxy для заданной строки.
     *
     * @param row Номер строки.
     * @return RowProxy для строки row.
     */
    detail::RowProxy<T, Default> operator[](int row) {
        return {*this, row};
    }

    /**
     * @brief Возвращает количество занятых ячеек.
     * @return Число элементов в хранилище.
     */
    size_t size() const {
        return data_.size();
    }

    /**
     * @brief Итератор на первую занятую ячейку.
     * @return Iterator указывающий на начало.
     */
    Iterator begin() { return {data_.begin()}; }

    /**
     * @brief Итератор за последней занятой ячейкой.
     * @return Iterator указывающий на конец.
     */
    Iterator end() { return {data_.end()}; }

private:
    std::map<std::pair<int, int>, T> data_; ///< Хранилище занятых ячеек.
};

#endif // OTUS_LAB6_SPARSEMATRIX_HPP