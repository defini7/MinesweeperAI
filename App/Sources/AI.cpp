#include "../Include/AI.hpp"

#include <ranges>

Sentence::Sentence(const std::unordered_set<def::Vector2i> cells, int minesCount)
    : cells(cells), minesCount(minesCount) {}

bool Sentence::operator==(const Sentence& other) const
{
    return minesCount == other.minesCount && cells == other.cells;
}

std::unordered_set<def::Vector2i> Sentence::GetKnownMines() const
{
    /*
    Some of the cells that we have in our sentence
    are probably mines but we don't know which are,
    so the best assumption we can make here is that
    if all of the cells are mines then we can return these cells
    but otherwise we can't tell anything else so just return empty set.
    */

    if (cells.size() == minesCount)
        return cells;

    return {};
}

std::unordered_set<def::Vector2i> Sentence::GetKnownSafes() const
{
    // If the number of mines among these cells is 0 then we know
    // for sure that there are no mines at all (logical, huh?).

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
    m_Mines.insert(cell);

    for (auto& sentence : m_Knowledge)
        sentence.MarkMine(cell);
}

void MinesweeperAI::MarkSafe(const def::Vector2i& cell)
{
    m_Safes.insert(cell);

    for (auto& sentence : m_Knowledge)
        sentence.MarkSafe(cell);
}

void MinesweeperAI::AddKnowledge(const def::Vector2i& cell, int minesCount)
{
    m_Moves.insert(cell);
    MarkSafe(cell);

    // Get neighbouring cells but exclude the safe ones
    // and decrement the "count" if the cell is known to be mine

    static const def::Vector2i ORIGIN(0, 0);
    std::unordered_set<def::Vector2i> undetermined;

    def::Vector2i offset;
    for (offset.y = -1; offset.y <= 1; offset.y++)
        for (offset.x = -1; offset.x <= 1; offset.x++)
        {
            if (offset == ORIGIN)
                continue;

            def::Vector2i neigh = cell + offset;

            if (ORIGIN <= neigh && neigh < m_BoardSize)
            {
                bool isSafe = m_Safes.contains(neigh);
                bool isMine = m_Mines.contains(neigh);

                // If the state of the neighbour is undetermined then
                // add it to the new sentence
                if (!isSafe && !isMine)
                    undetermined.insert(neigh);

                // And decrease the number of mines if the cell is known to be mine
                if (isMine)
                    minesCount--;
            }
        }

    if (!undetermined.empty())
    {
        // Add a new sentence to the AI's knowledge base based
        // on the value of 'cell' and 'count'
        Sentence newSentence(undetermined, minesCount);

        if (!newSentence.cells.empty() && !Vector_Contains(m_Knowledge, newSentence))
            m_Knowledge.push_back(newSentence);
    }

    // Mark any additional cells as safe or as mines
    // if it can be concluded based on the AI's knowledge base
    MarkCells();

    // Add any new sentences to the AI's knowledge base
    // if they can be inferred from existing knowledge
    std::vector<Sentence> newSentences;

    for (const auto& sentence1 : m_Knowledge)
        for (const auto& sentence2 : m_Knowledge)
        {
            // We don't compare a sentence with itself
            if (sentence1.minesCount < 0 || sentence2.minesCount < 0 || sentence1 == sentence2)
                continue;

            if (UnorderedSet_IsSubset(sentence2.cells, sentence1.cells))
            {
                int newCount = sentence2.minesCount - sentence1.minesCount;

                // Ensure that the difference between the number
                // of mines isn't negative
                if (newCount >= 0)
                {
                    std::unordered_set<def::Vector2i> newCells;
                    newCells.reserve(sentence2.cells.size() - sentence1.cells.size());
                    
                    for (const auto& cell : sentence2.cells)
                    {
                        if (!sentence1.cells.contains(cell))
                            newCells.insert(cell);
                    }

                    // Ensure that the differene between 2 sets of cells
                    // is not a blank set
                    if (!newCells.empty())
                    {
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
        if (!m_Moves.contains(move))
            return move;
    }

    return std::nullopt;
}

std::optional<def::Vector2i> MinesweeperAI::MakeRandomMove()
{
    std::unordered_set<def::Vector2i> explored;
    int availableCells = m_BoardSize.x * m_BoardSize.y - m_Mines.size() - m_Safes.size();

    if (availableCells == 0)
        return std::nullopt;

    srand(time(nullptr));

    while (true)
    {
        def::Vector2i cell(rand() % m_BoardSize.x, rand() % m_BoardSize.y);
        
        if (!explored.contains(cell))
        {
            if (!m_Moves.contains(cell) && !m_Mines.contains(cell))
                return cell;

            explored.insert(cell);
        }
    }
}

void MinesweeperAI::MarkCells()
{
    std::unordered_set<def::Vector2i> newSafes;
    std::unordered_set<def::Vector2i> newMines;

    for (const auto& sentence : m_Knowledge)
    {
        for (const auto& safe : sentence.GetKnownSafes())
            newSafes.insert(safe);

        for (const auto& mine : sentence.GetKnownMines())
            newMines.insert(mine);
    }

    for (const auto& safe : newSafes)
        MarkSafe(safe);

    for (const auto& mine : newMines)
        MarkMine(mine);
}

const std::unordered_set<def::Vector2i>& MinesweeperAI::GetKnownMines() const
{
    return m_Mines;
}
