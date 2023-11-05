#include "formula.h"
#include "common.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>
#include <string_view>
#include <iostream>

using namespace std::literals;

FormulaError::FormulaError(FormulaError::Category category)
    : category_(category) {}

FormulaError::Category FormulaError::GetCategory() const {
    return category_;
}

bool FormulaError::operator==(FormulaError rhs) const {
    return category_ == rhs.category_;
}

std::string_view FormulaError::ToString() const {
    switch (category_) {
        case Category::Value:
            return "#VALUE!"sv;
        case Category::Ref:
            return "#REF!"sv;
        case Category::Div0:
            return "#ARITHM!"sv;
        default:
            assert(false);
            return ""sv;
    }
}

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << fe.ToString();
}

namespace {
class Formula : public FormulaInterface {
public:
    explicit Formula(std::string expression)
        : ast_(ParseFormulaAST(std::move(expression))) {}
    
    Value Evaluate(const SheetInterface& sheet) const override {
        auto get_cell_value = [&sheet](Position pos) {
            double result = 0.;
            auto cell_ptr = sheet.GetCell(pos);
            
            if (cell_ptr == nullptr) {
                return result;
            }
            
            auto cell_value = cell_ptr->GetValue();
            if (std::holds_alternative<double>(cell_value)) {
                result = std::get<double>(cell_value);
            } else if (std::holds_alternative<std::string>(cell_value)) {
                std::string str = std::get<std::string>(cell_value);
                
                if (str.empty()) {
                    return result;
                }
                
                try {
                    size_t char_processed = 0;
                    result = std::stod(str, &char_processed);
                        
                    if (char_processed != str.size()) {
                        throw FormulaError(FormulaError::Category::Value);
                    }
                } catch (...) {
                    throw FormulaError(FormulaError::Category::Value);
                }
            } else {
                throw std::get<FormulaError>(cell_value);
            }
            return result;      
        };
        
        try {
            return ast_.Execute(get_cell_value);
        } catch (FormulaError& er) {
            return er;
        }
    } 
    
    std::string GetExpression() const override {
        std::ostringstream out;
        ast_.PrintFormula(out);
        return out.str();
    } 
    
    std::vector<Position> GetReferencedCells() const override {
        auto cells_list = ast_.GetCells();
        std::vector<Position> referenced_cells(cells_list.begin(), cells_list.end());
        auto last = std::unique(referenced_cells.begin(), referenced_cells.end());
        referenced_cells.erase(last, referenced_cells.end());
        return referenced_cells;
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}