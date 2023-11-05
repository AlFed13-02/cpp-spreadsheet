#pragma once

#include "common.h"
#include "formula.h"

#include <memory>
#include <string>
#include <variant>
#include <set>
#include <vector>
#include <iostream>

using namespace std::literals;

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);

    void Set(std::string text);
    void Clear();

    Value GetValue() const override;
    std::string GetText() const override;
    
    std::vector<Position> GetReferencedCells() const override;
    
    void AddDependentCell(Position pos);
    void RemoveDependentCell(Position pos);

    void InvalidateCache();

    bool HaveCircularDependencies(Position pos, std::set<Position>& visited) const;

private:
    class Impl {
    public:
        using Value = std::variant<std::string, double, FormulaError>;
    
        virtual ~Impl() = default;
    
        virtual void Set(std::string text) = 0;
    
        virtual Value GetValue() const = 0;
        virtual std::string GetText() const = 0;
    };

    class EmptyImpl : public Impl {
    public:
        void Set(std::string text) override {}
    
        Value GetValue() const override {
            return 0.;
        }
        
        std::string GetText() const override {
            return ""s;
        }
    };

    class TextImpl : public Impl {
    public:
        void Set(std::string text) override {
            content_ = std::move(text);
        }
    
        Value GetValue() const override {
            if (content_[0] == ESCAPE_SIGN) {
                return content_.substr(1);
            }
        
            return content_;
        }
        
        std::string GetText() const override {
            return content_;
        }
    
    private:
        std::string content_;
    };
    
    class FormulaImpl : public Impl {
    public:
        FormulaImpl(SheetInterface& sheet) : sheet_(sheet) {}
        
        void Set(std::string text) override {
            formula_ = std::move(ParseFormula(std::move(text)));
        }
    
        Value GetValue() const override {
            auto value = formula_->Evaluate(sheet_);
        
            if (std::holds_alternative<double>(value)) {
                return std::get<double>(value);
            }
        
            return std::get<FormulaError>(value);
        }
    
        std::string GetText() const override {
            return '=' + formula_->GetExpression();
        }
        
        std::vector<Position> GetReferencedCells() {
            return formula_->GetReferencedCells();
        }
    
    private:
        SheetInterface& sheet_;
        std::unique_ptr<FormulaInterface> formula_;    
    };

    SheetInterface& sheet_;
    std::unique_ptr<Impl> impl_;
    std::set<Position> dependent_cells_;
    std::vector<Position> referenced_cells_;
    mutable std::optional<double> cache_;
};