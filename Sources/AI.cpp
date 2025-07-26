#include "../Include/AI.hpp"

#include <random>
#include <ranges>

Sentence::Sentence(const std::vector<def::Vector2i> cells, int minesCount)
    : cells(cells), minesCount(minesCount) {}

bool Sentence::operator==(const Sentence& other) const
{
    return minesCount == other.minesCount && cells == other.cells;
}

std::vector<def::Vector2i> Sentence::GetKnownMines() const
{
    /*
    Some of the cells that we have in our sentence
    are probably mines but we don't know which are,
    so the best assumption we can make here is that
    if all of the cells are mines then we can return these cells
    but otherwise we can't tell anything else so just return empty set
    */
    if (cells.size() == minesCount)
        return cells;

    return {};
}

std::vector<def::Vector2i> Sentence::GetKnownSafes() const
{
    if (minesCount == 0)
        return cells;

    return {};
}

void Sentence::MarkMine(const def::Vector2i& cell)
{
    // Marks a cell as a mine, and updates all knowledge
    // to mark that cell as a mine as well.

    const auto it = std::ranges::find(cells, cell);
    
    if (it == cells.end())
        return;

    cells.erase(it);
    minesCount--;
}

void Sentence::MarkSafe(const def::Vector2i& cell)
{
    // Marks a cell as safe, and updates all knowledge
    // to mark that cell as safe as well.

    const auto it = std::ranges::find(cells, cell);
    
    if (it == cells.end())
        return;

    cells.erase(it);
}

MinesweeperAI::MinesweeperAI(const def::Vector2i& boardSize)
    : m_BoardSize(boardSize) {}

void MinesweeperAI::MarkMine(const def::Vector2i& cell)
{
    if (!Vector_Contains(m_Mines, cell))
    {
        m_Mines.push_back(cell);

        for (auto& sentence : m_Knowledge)
            sentence.MarkMine(cell);
    }
}

void MinesweeperAI::MarkSafe(const def::Vector2i& cell)
{
    if (!Vector_Contains(m_Safes, cell))
    {
        m_Safes.push_back(cell);

        for (auto& sentence : m_Knowledge)
            sentence.MarkSafe(cell);
    }
}

void MinesweeperAI::AddKnowledge(const def::Vector2i& cell, int minesCount)
{
    m_Moves.push_back(cell);
    MarkSafe(cell);

    // Get neighbouring cells but exclude the safe ones
    // and decrement the "count" if the cell is known to be mine

    static const def::Vector2i ORIGIN(0, 0);
    std::vector<def::Vector2i> undetermined;

    def::Vector2i offset;
    for (offset.y = -1; offset.y <= 1; offset.y++)
        for (offset.x = -1; offset.x <= 1; offset.x++)
        {
            if (offset == ORIGIN)
                continue;

            def::Vector2i neigh = cell + offset;

            if (ORIGIN <= neigh && neigh < m_BoardSize)
            {
                bool isSafe = Vector_Contains(m_Safes, neigh);
                bool isMine = Vector_Contains(m_Mines, neigh);

                // If the state of the neighbour is undetermined then
                // add it to the new sentence
                if (!isSafe && !isMine)
                    undetermined.push_back(neigh);

                // And decrease the number of mines if the cell is know to be mine
                if (isMine)
                    minesCount--;
            }
        }

    std::ranges::sort(undetermined);

    // Add a new sentence to the AI's knowledge base based
    // on the value of 'cell' and 'count'
    Sentence newSentence(undetermined, minesCount);

    if (!Vector_Contains(m_Knowledge, newSentence))
        m_Knowledge.push_back(newSentence);

    // Mark any additional cells as safe or as mines
    // if it can be concluded based on the AI's knowledge base
    MarkCells();

    // Add any new sentences to the AI's knowledge base
    // if they can be inferred from existing knowledge
    std::vector<Sentence> newSentences;

    for (auto& sentence1 : m_Knowledge)
        for (auto& sentence2 : m_Knowledge)
        {
            // We don't compare a sentence with itself
            if (sentence1 == sentence2)
                continue;

            if (std::ranges::includes(sentence2.cells, sentence1.cells))
            {
                int newCount = sentence2.minesCount - sentence1.minesCount;

                // Ensure that the difference between the number
                // of mines isn't negative
                if (newCount >= 0)
                {
                    std::vector<def::Vector2i> newCells;
                    newCells.reserve(sentence2.cells.size() - sentence1.cells.size());
                    
                    for (const auto& cell : sentence2.cells)
                    {
                        if (!Vector_Contains(sentence1.cells, cell))
                            newCells.push_back(cell);
                    }

                    // Ensure that the differene between 2 sets of cells
                    // is not a blank set
                    if (!newCells.empty())
                    {
                        std::ranges::sort(newCells);
                        Sentence newSentence(newCells, newCount);

                        // Add a new sentence to our knowledge base if it
                        // is not already there
                        if (!Vector_Contains(newSentences, newSentence) && !Vector_Contains(m_Knowledge, newSentence))
                            newSentences.push_back(newSentence);
                    }
                }
            }
        }

    for (const auto& sentence : newSentences)
        m_Knowledge.push_back(sentence);

    MarkCells();
}
    
std::optional<def::Vector2i> MinesweeperAI::MakeSafeMove()
{
    for (const auto& move : m_Safes)
    {
        if (!Vector_Contains(m_Moves, move))
            return move;
    }

    return std::nullopt;
}

std::optional<def::Vector2i> MinesweeperAI::MakeRandomMove()
{
    std::vector<def::Vector2i> explored;
    int availableCells = m_BoardSize.x * m_BoardSize.y - m_Mines.size() - m_Safes.size();

    if (availableCells == 0)
        return std::nullopt;

    using IntDist = std::uniform_int_distribution<>;

    std::mt19937 mt;
    IntDist xRange(0, m_BoardSize.x - 1);
    IntDist yRange(0, m_BoardSize.y - 1);

    while (true)
    {
        def::Vector2i cell(xRange(mt), yRange(mt));
        
        if (!Vector_Contains(explored, cell))
        {
            if (!Vector_Contains(m_Moves, cell) && !Vector_Contains(m_Mines, cell))
                return cell;

            explored.push_back(cell);
        }
    }
}

void MinesweeperAI::MarkCells()
{
    std::vector<def::Vector2i> newSafes;
    std::vector<def::Vector2i> newMines;

    for (const auto& sentence : m_Knowledge)
    {
        for (const auto& safe : sentence.GetKnownSafes())
        {
            if (!Vector_Contains(newSafes, safe))
                newSafes.push_back(safe);
        }

        for (const auto& mine : sentence.GetKnownMines())
        {
            if (!Vector_Contains(newMines, mine))
                newMines.push_back(mine);
        }
    }

    for (const auto& safe : newSafes)
        MarkSafe(safe);

    for (const auto& mine : newMines)
        MarkMine(mine);
}

const std::vector<def::Vector2i>& MinesweeperAI::GetKnownMines() const
{
    return m_Mines;
}
