#include <iostream>
#include <xlnt/xlnt.hpp>

int main() {
    try {
        // 创建一个工作簿对象
        xlnt::workbook wb;

        // 加载 Excel 文件
        wb.load("test.xlsx");

        // 获取第一个工作表
        auto ws = wb.active_sheet();

        // 遍历工作表中的每一行和每一列
        for (auto row : ws.rows(false)) {
            for (auto cell : row) {
                std::cout << cell.to_string() << "\t";
            }
            std::cout << std::endl;
        }
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}