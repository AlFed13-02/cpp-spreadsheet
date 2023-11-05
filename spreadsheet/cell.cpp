#include "cell.h"
#include "common.h"


#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <memory>

Cell::Cell(SheetInterface& sheet) 
    : sheet_(sheet)
    , impl_(std::move(std::make_unique<EmptyImpl>())) {}

void Cell::Set(std::string text) {
    referenced_cells_.clear();
    InvalidateCache();
    
    if (text.empty()) {
        auto impl_ptr = std::make_unique<EmptyImpl>();
        impl_ = std::move(impl_ptr);
    }
    if (text[0] == FORMULA_SIGN) {
        auto impl_ptr = std::make_unique<FormulaImpl>(sheet_);
        impl_ptr->Set(text.substr(1));
        referenced_cells_ = std::move(impl_ptr->GetReferencedCells());
        
        for (const auto& pos: referenced_cells_) {
            if (sheet_.GetCell(pos) == nullptr) {
                sheet_.SetCell(pos, ""s);
            }
        }
        
        impl_ = std::move(impl_ptr);
    } else {
        auto impl_ptr = std::make_unique<TextImpl>();
        impl_ptr->Set(text);
        impl_ = std::move(impl_ptr);
    }
}

void Cell::Clear() {
    impl_ = std::move(std::make_unique<EmptyImpl>());
}

Cell::Value Cell::GetValue() const {
    if (cache_) {
        return *cache_;
    }
    
    auto result = impl_->GetValue();
    if (std::holds_alternative<double>(result)) {
        cache_ = std::get<double>(result);
    }
    
    return result;
}

std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    return referenced_cells_;
}

bool Cell::HaveCircularDependencies(Position pos, std::set<Position>& visited) const {
    if (visited.count(pos) != 0) {
        return true;
    }
    
    visited.insert(pos);
    
    for (const auto& pos: GetReferencedCells()) {
        auto cell_ptr = reinterpret_cast<Cell*>(sheet_.GetCell(pos));
        if (cell_ptr->HaveCircularDependencies(pos, visited)) {
            return true;
        }
    }
    
    return false;
}

void Cell::AddDependentCell(Position pos) {
    dependent_cells_.insert(pos);
}

void Cell::RemoveDependentCell(Position pos) {
    dependent_cells_.erase(pos);
}

void Cell::InvalidateCache() {
    cache_.reset();
    for (const auto& pos: dependent_cells_) {
        auto cell_ptr  = reinterpret_cast<Cell*>(sheet_.GetCell(pos));
        cell_ptr->InvalidateCache();
    }
}