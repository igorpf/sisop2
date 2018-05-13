#include <iomanip>

#include "../include/table_printer.hpp"

TablePrinter::TablePrinter(std::vector<std::vector<std::string>>& content) : content_(content) {
    size_t max_size = 0;

    // Obtém o tamanho da maior linha
    for (const auto& line : content_) {
        max_size = std::max(max_size, line.size());
    }

    // Zera o valor de largura de cada coluna preenchendo o vetor
    for (int64_t i = 0; i < max_size; ++i) {
        width_list_.emplace_back(0);
    }

    // Obtém a largura de cada coluna
    for (const auto& line : content_) {
        for (int64_t i = 0; i < line.size(); ++i) {
            // Adiciona 1 a cada largura para dar espaço entre cada coluna
            width_list_[i] = std::max(width_list_[i], line[i].size()) + 1;
        }
    }
}

void TablePrinter::Print(std::ostream &out) {
    for (const auto& line : content_) {
        for (int64_t i = 0; i < line.size(); ++i) {
            out << std::left << std::setw(static_cast<int>(width_list_[i])) << line[i];
        }
        out << std::endl;
    }
}
