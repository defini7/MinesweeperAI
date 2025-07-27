#include "../Include/App.hpp"

bool Rect2i::Contains(const def::Vector2i& p)
{
    return pos <= p && p < pos + size;
}

App::App()
{
    GetWindow()->SetTitle("Minesweeper");
}

App::~App()
{

}

bool App::OnUserCreate()
{
    def::Vector2i dims = GetWindow()->GetScreenSize();

    // We choose a width of the board so we can fit some UI later
    m_BoardScreenSize.x = 2.0f * dims.x / 3.0f - config::BOARD_PADDING * 2;
    m_BoardScreenSize.y = dims.y - config::BOARD_PADDING * 2;

    // Choose a cell size in screen pixels so the board can fit within the screen
    int cellSize = std::min(m_BoardScreenSize.x / config::BOARD_SIZE.x, m_BoardScreenSize.y / config::BOARD_SIZE.y);
    m_CellSize = { cellSize, cellSize };

    // Load flag and mine sprites
    m_FlagImage.Load("../../App/Assets/flag.png");
    m_MineImage.Load("../../App/Assets/mine.png");

    // Construct the game class and the AI
    m_Game = std::make_unique<Minesweeper>(config::BOARD_SIZE, config::MINES_COUNT);
    m_AI = std::make_unique<MinesweeperAI>(config::BOARD_SIZE);

    return true;
}

bool App::OnUserUpdate(float deltaTime)
{
    auto inp = GetInput();
    auto win = GetWindow();

    if (inp->GetKeyState(def::Key::ESCAPE).released)
        return false;

    const def::Vector2i dims = win->GetScreenSize();

    if (m_ShowInstructions)
    {
        // Handle user input

        int halfWidth = dims.x / 2;

        Rect2i playButtonRect
        {
            { halfWidth / 2, int(3.0f / 4.0f * dims.y) },
            { halfWidth, 50 }
        };

        if (inp->GetButtonState(def::Button::LEFT).released)
        {
            def::Vector2i mousePos = inp->GetMousePosition();

            if (playButtonRect.Contains(mousePos))
                m_ShowInstructions = false;
        }

        // Draw the main menu

        Clear(def::BLACK);

        // Title
        DrawCenteredText(
            { halfWidth, 50 },
            "Play Minesweeper", def::WHITE);

        // Rules
        int i = 0;
        for (const auto& rule : config::RULES)
        {
            DrawCenteredText(
                { halfWidth, 150 + i * 30 },
                rule, def::WHITE);
            i++;
        }

        DrawButton(playButtonRect, "Play Game");

        return true;
    }

    Rect2i aiButtonRect
    {
        {
            int(2.0f / 3.0f * dims.x) + config::BOARD_PADDING,
            int(1.0f / 3.0f * dims.y) - 50
        },
        { dims.x / 3 - config::BOARD_PADDING * 2, 50 }
    };

    Rect2i resetButtonRect
    {
        {
            int(2.0f / 3.0f * dims.x) + config::BOARD_PADDING,
            int(1.0f / 3.0f * dims.y) + 20
        },
        aiButtonRect.size
    };

    // Place a flag on the board on the right mouse button click
    if (inp->GetButtonState(def::Button::RIGHT).released)
    {
        def::Vector2i mousePos = inp->GetMousePosition();

        if (config::BOARD_ORIGIN <= mousePos && mousePos < config::BOARD_ORIGIN + m_BoardScreenSize)
        {
            // Calculating the position of the cell on the board
            def::Vector2i cellCoord = (mousePos - config::BOARD_ORIGIN) / m_CellSize;
            
            // Place the flag if it was not here and vice versa
            auto cell = m_Game->GetCell(cellCoord);
            cell.isFlagged = !cell.isFlagged;
        }
    }

    std::optional<def::Vector2i> move = std::nullopt;

    if (inp->GetButtonState(def::Button::LEFT).released)
    {
        def::Vector2i mousePos = inp->GetMousePosition();

        // Make a move if the player hasn't lost the game
        if (aiButtonRect.Contains(mousePos) && !m_Lost)
        {
            // Trying to make a safe move
            if (move = m_AI->MakeSafeMove())
                std::cout << "AI makes a safe move: " << move->ToString() << std::endl;

            // Making a random move
            else
            {
                if (move = m_AI->MakeRandomMove())
                    std::cout << "No known safe moves, making random move: " << move->ToString() << std::endl;
                else
                    std::cout << "No moves left to make" << std::endl;
            }

            // Updating flags in the game based on the AI knowledge
            for (const auto& mine : m_AI->GetKnownMines())
                m_Game->GetCell(mine).isFlagged = true;
        }

        // Reset the game state
        else if (resetButtonRect.Contains(mousePos))
        {
            m_Game.reset(new Minesweeper(config::BOARD_SIZE, config::MINES_COUNT));
            m_AI.reset(new MinesweeperAI(config::BOARD_SIZE));
            m_Lost = false;
        }

        // Open a cell on the left mouse button click
        else if (!m_Lost)
        {
            def::Vector2i mousePos = inp->GetMousePosition();

            if (config::BOARD_ORIGIN <= mousePos && mousePos < config::BOARD_ORIGIN + m_BoardScreenSize)
            {
                // Calculating the position of the cell on the board
                def::Vector2i cellCoord = (mousePos - config::BOARD_ORIGIN) / m_CellSize;
                
                const auto& cell = m_Game->GetCell(cellCoord);

                if (!cell.isFlagged && !cell.isRevealed)
                    move = cellCoord;
            }
        }
    }

    if (move)
    {
        def::Vector2i cellCoord = move.value();
        auto& cell = m_Game->GetCell(cellCoord);

        if (cell.isMine)
            m_Lost = true;
        else
        {
            cell.isRevealed = true;
            m_AI->AddKnowledge(cellCoord, cell.nearbyMinesCount);
        }
    }

    bool won = !m_Lost && m_Game->Won();

    Clear(def::BLACK);

    // Draw the board

    def::Vector2i c;
    for (; c.y < config::BOARD_SIZE.y; c.y++)
        for (c.x = 0; c.x < config::BOARD_SIZE.x; c.x++)
        {
            def::Vector2i cellPos = config::BOARD_ORIGIN + c * m_CellSize;

            // Draw a cell
            FillRectangle(cellPos, m_CellSize, def::GREY);
            DrawRectangle(cellPos, m_CellSize, def::WHITE);

            const Cell& cell = m_Game->GetCell(c);

            // Draw either a mine if we lost the game or a flag or a number
            if (m_Lost && cell.isMine && !cell.isFlagged)
                DrawWithinCell(cellPos, m_MineImage);
            else if (cell.isFlagged)
                DrawWithinCell(cellPos, m_FlagImage);
            else if (cell.isRevealed)
            {
                DrawCenteredText(
                    cellPos + m_CellSize / 2,
                    std::to_string(cell.nearbyMinesCount), def::BLACK);
            }
        }

    DrawButton(aiButtonRect, "AI Move");
    DrawButton(resetButtonRect, "Reset");

    if (m_Lost || won)
    {
        std::string text;
        if (m_Lost) text = "Lost!";
        else if (won) text = "Won!";

        def::Vector2i center
        {
            int(5.0f / 6.0f * dims.x),
            int(2.0f / 3.0f * dims.y)
        };

        DrawCenteredText(center, text, def::WHITE);
    }

    return true;
}

void App::DrawCenteredText(const def::Vector2i& center, const std::string_view text, const def::Pixel& col)
{
    // Since each character on the screen is 8 pixels
    // in width and 8 pixels in height we calculate a new position:
    // new_x = old_x - text_length * 8 / 2
    // new_y = old_y - 8 / 2
    int x = center.x - text.length() * 4;
    int y = center.y - 4;

    DrawString({ x, y }, text, col);
}

void App::DrawButton(const Rect2i& rect, const std::string_view text)
{
    // Draw background
    FillRectangle(rect.pos, rect.size, def::WHITE);

    // Draw text
    def::Vector2i center = rect.pos + rect.size / 2;
    DrawCenteredText(center, text, def::BLACK);
}

void App::DrawWithinCell(const def::Vector2i& pos, const def::Graphic& gfx)
{
    DrawTexture(pos, gfx.texture, m_CellSize / def::Vector2f(gfx.texture->size));
}
