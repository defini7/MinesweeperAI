#pragma once

#include "Vector2D.hpp"
#include "Game.hpp"

template <class T>
bool Vector_Contains(const std::vector<T>& vector, const T& value)
{
    return std::ranges::find(vector, value) != vector.end();
}

/*
Logical statement about a Minesweeper game
A sentence consists of a set of board cells,
and a count of the number of those cells which are mines.
*/
struct Sentence
{
    Sentence(const std::vector<def::Vector2i> cells, int minesCount);

    bool operator==(const Sentence& other) const;

    std::vector<def::Vector2i> GetKnownMines() const;
    std::vector<def::Vector2i> GetKnownSafes() const;

    void MarkMine(const def::Vector2i& cell);
    void MarkSafe(const def::Vector2i& cell);

    std::vector<def::Vector2i> cells;
    int minesCount;
};

class MinesweeperAI
{
public:
    MinesweeperAI(const def::Vector2i& boardSize);

    /*
    Marks a cell as a mine, and updates all knowledge
    to mark that cell as a mine as well.
    */
    void MarkMine(const def::Vector2i& cell);

    /*
    Marks a cell as safe, and updates all knowledge
    to mark that cell as safe as well.
    */
    void MarkSafe(const def::Vector2i& cell);

    /*
    Called when the Minesweeper board tells us, for a given
    safe cell, how many neighboring cells have mines in them.
    */
    void AddKnowledge(const def::Vector2i& cell, int minesCount);
    
    /*
    Returns a safe cell to choose on the Minesweeper board.
    The move must be known to be safe, and not already a move
    that has been made.

    This function may use the knowledge in self.mines, self.safes
    and self.moves_made, but should not modify any of those values.
    */
    std::optional<def::Vector2i> MakeSafeMove();

    /*
    Returns a move to make on the Minesweeper board.
    Should choose randomly among cells that:
        1) have not already been chosen, and
        2) are not known to be mines
    */
    std::optional<def::Vector2i> MakeRandomMove();

    /*
    Returns all cells that are known to be mines
    based on the current knowledge.
    */
    const std::vector<def::Vector2i>& GetKnownMines() const;

private:
    void MarkCells();

private:
    def::Vector2i m_BoardSize;

    std::vector<def::Vector2i> m_Moves;
    std::vector<def::Vector2i> m_Safes;
    std::vector<def::Vector2i> m_Mines;

    std::vector<Sentence> m_Knowledge;

};
