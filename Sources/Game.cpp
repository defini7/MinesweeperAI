#include "../Include/Game.hpp"

Minesweeper::Minesweeper(const def::Vector2i &boardSize, int minesCount)
    : m_BoardSize(boardSize), m_MinesCount(minesCount)
{
    // Creating the board
    m_Board = new Cell[boardSize.x * boardSize.y];

    // Populating the board with mines

    using IntDist = std::uniform_int_distribution<>;
    
    std::mt19937 mt(time(nullptr));
    IntDist widthRange(0, boardSize.x - 1);
    IntDist heightRange(0, boardSize.y - 1);

    size_t minesCreated = 0;
    while (minesCreated != minesCount)
    {
        int x = widthRange(mt);
        int y = heightRange(mt);

        int i = y * boardSize.x + x;

        if (!m_Board[i].isMine)
        {
            m_Board[i].isMine = true;
            minesCreated++;
        }
    }

    for (int y = 0; y < boardSize.y; y++)
        for (int x = 0; x < boardSize.x; x++)
        {
            int i = y * boardSize.x + x;
            m_Board[i].nearbyMinesCount = CountNearbyMines({ x, y });
        }
}

Minesweeper::~Minesweeper()
{
    delete m_Board;
}

Cell& Minesweeper::GetCell(const def::Vector2i& cell)
{
    return m_Board[cell.y * m_BoardSize.x + cell.x];
}

const Cell& Minesweeper::GetCell(const def::Vector2i& cell) const
{
    return m_Board[cell.y * m_BoardSize.x + cell.x];
}

bool Minesweeper::Won() const
{
    int boardSize = m_BoardSize.x * m_BoardSize.y;
    int revealedCount = 0;

    for (int i = 0; i < boardSize; i++)
    {
        // If the cell is a mine and it's flagged
        if (m_Board[i].isMine && m_Board[i].isFlagged)
            revealedCount++;
    }

    return revealedCount == m_MinesCount;
}
    
int Minesweeper::CountNearbyMines(const def::Vector2i& cell) const
{
    int count = 0;

    static const def::Vector2i ORIGIN(0, 0);

    def::Vector2i offset;
    for (offset.y = -1; offset.y <= 1; offset.y++)
        for (offset.x = -1; offset.x <= 1; offset.x++)
        {
            // Ignore the cell itself
            if (offset == ORIGIN)
                continue;

            def::Vector2i pos = cell + offset;

            // Check if the cell is within the bounds and has a mine
            if (ORIGIN <= pos && pos < m_BoardSize && GetCell(pos).isMine)
                count++;
        }
    
    return count;
}