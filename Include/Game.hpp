#pragma once

#include "Vector2D.hpp"

#include <random>

struct Cell
{
    bool isFlagged = false;
    bool isMine = false;
    bool isRevealed = false;

    int nearbyMinesCount = 0;
};

class Minesweeper
{
public:
    Minesweeper(const def::Vector2i& boardSize, int minesCount);
    ~Minesweeper();

    // Returns a state of the cell
    Cell& GetCell(const def::Vector2i& cell);
    const Cell& GetCell(const def::Vector2i& cell) const;
    
    // Checks if the player has flagged all mines
    bool Won() const;

    /*
        Returns the number of mines that are
        within one row and column of a given cell,
        not including the cell itself.
    */
    int CountNearbyMines(const def::Vector2i& cell) const;

private:
    def::Vector2i m_BoardSize;

    // Here true stands for a mine and false stands for a no mine
    Cell* m_Board;

    int m_MinesCount;

};