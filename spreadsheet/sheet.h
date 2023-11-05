#pragma once

#include "common.h"
#include "cell.h"

#include <functional>
#include <string>
#include <iostream>
#include <vector>
#include <memory>

class Sheet : public SheetInterface {
public:
    void SetCell(Position pos, std::string text) override;

    const CellInterface* GetCell(Position pos) const override;
    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
    const Cell* GetConcreteCell(Position pos) const;
    Cell* GetConcreteCell(Position pos);
    
private: 
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
    Size printable_size_;
    
    void UpdatePrintableSize(Position pos, bool on_delete);
    void PrintCells(std::ostream& output,
                    const std::function<void(const CellInterface*)>& printCell) const;
};