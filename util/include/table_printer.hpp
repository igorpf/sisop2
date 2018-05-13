#ifndef SISOP2_CLIENT_INCLUDE_TABLE_PRINTER_HPP
#define SISOP2_CLIENT_INCLUDE_TABLE_PRINTER_HPP

#include <iostream>
#include <vector>
#include <string>

/**
 * Classe para exibição de dados estilo tabela na tela
 */
class TablePrinter {
public:
    /**
     * Recebe os dados em forma de vetor de vetores de string
     * Cada subvetor é uma linha
     */
    explicit TablePrinter(std::vector<std::vector<std::string>>& content);

    /**
     * Exibe a tabela na tela
     */
    void Print(std::ostream& out = std::cout);

private:
    std::vector<size_t> width_list_;
    std::vector<std::vector<std::string>> content_;
};

#endif // SISOP2_CLIENT_INCLUDE_TABLE_PRINTER_HPP
