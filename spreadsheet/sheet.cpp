#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>
#include <variant>
#include <algorithm>

using namespace std::literals;

void Sheet::SetCell(Position pos, std::string text) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is invalid"s);
    }
    
    if (cells_.size() < static_cast<size_t>(pos.row + 1)) {
        cells_.resize(pos.row + 1);
    }
    
    if (cells_.at(pos.row).size() < static_cast<size_t>(pos.col + 1)) {
        cells_.at(pos.row).resize(pos.col + 1);
    }
    
    auto cell = GetCell(pos);
    
    if (cell != nullptr) {
        for (const auto& entry: cell->GetReferencedCells()) {
            auto cell_ptr  = GetConcreteCell(entry);
            cell_ptr->RemoveDependentCell(pos);
        }
    }
    
    auto cell_ptr = std::make_unique<Cell>(*this);
    cell_ptr->Set(text);
    
    std::set<Position> visited;
    if (cell_ptr->HaveCircularDependencies(pos, visited)) {
        throw CircularDependencyException("Circular dependency found"s);
    } 
    
    for (const auto& entry: cell_ptr->GetReferencedCells()) {
        auto cell = GetConcreteCell(entry);
        cell->AddDependentCell(pos);
    }
    
    cells_[pos.row][pos.col] = std::move(cell_ptr);
    UpdatePrintableSize(pos, false);
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return GetConcreteCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    return GetConcreteCell(pos);
}

void Sheet::ClearCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is invalid"s);
    }
    
    if (pos.row >= static_cast<int>(cells_.size()) || pos.col >= static_cast<int>(cells_[pos.row].size())) {
        return;
    }
    
    auto cell = GetConcreteCell(pos);
    if (cell != nullptr) {
        for (const auto& entry: cell->GetReferencedCells()) {
            auto cell_ptr  = GetConcreteCell(entry);
            cell_ptr->RemoveDependentCell(pos);
        }
    }
    
    cells_[pos.row][pos.col] = nullptr;
    UpdatePrintableSize(pos, true);
}

Size Sheet::GetPrintableSize() const {
    return printable_size_;
}

void Sheet::PrintTexts(std::ostream& output) const {
    auto printCell = [&output](const CellInterface* cell) {
        if (cell != nullptr) {
            output << cell->GetText() ;
        }    
    };
        
    PrintCells(output, printCell);
}

void Sheet::PrintValues(std::ostream& output) const {
    auto printCell = [&output](const CellInterface* cell) {
        if (cell != nullptr) {
            std::visit(
                 [&](const auto& x) {
                    output << x;
                },
                cell->GetValue());
        }
    };
    
    PrintCells(output, printCell);
}

void Sheet::UpdatePrintableSize(Position pos, bool on_delete) {
    if (on_delete) {
        auto& row = cells_[pos.row];
        if (!row.empty() && pos.col == (static_cast<int>(row.size()) - 1)) {
         
            for (int i = pos.col; i > -1; --i) {
                if (row[i] != nullptr) {
                    break;
                }
                
                row.pop_back();
            } 
        }
            
        if (pos.row == printable_size_.rows - 1) {
            
            for (int i = pos.row; i > -1; --i) {
                if (!cells_[i].empty()) {
                    break;
                }
                
                cells_.pop_back();
            }
        }
        
        
        printable_size_.rows = cells_.size();
    
        auto longest_row = std::max_element(cells_.begin(), cells_.end(), 
                                            [](const auto& lhs, const auto& rhs) {
                                                return lhs.size() < rhs.size();
                                            });
        printable_size_.cols = longest_row->size();
        
    } else {
        if (pos.row >= printable_size_.rows) {
            printable_size_.rows = pos.row + 1;
        }
        
        if (pos.col >= printable_size_.cols) {
            printable_size_.cols = pos.col + 1;
        }
    }
}

const Cell* Sheet::GetConcreteCell(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is invalid"s);
    }
    
    if (pos.row >= static_cast<int>(cells_.size()) || pos.col >= static_cast<int>(cells_[pos.row].size())) {
        return nullptr;
    }
    
    return cells_.at(pos.row).at(pos.col).get();
}

Cell* Sheet::GetConcreteCell(Position pos) {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is invalid"s);
    }
    
    if (pos.row >= static_cast<int>(cells_.size()) || pos.col >= static_cast<int>(cells_[pos.row].size())) {
        return nullptr;
    }
    
    return cells_[pos.row][pos.col].get();
}

void Sheet::PrintCells(std::ostream& output,
                    const std::function<void(const CellInterface*)>& printCell) const {
    for (int i = 0; i != printable_size_.rows; ++i) {
        for (int j = 0; j != printable_size_.cols; ++j) {
            
            auto cell = GetCell(Position{i, j});
            printCell(cell);
            
            if (j != printable_size_.cols - 1) {
                output << "\t"s;
            }   
        }
        
        output << "\n"s;
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}

