#pragma once

#include "defGameEngine.hpp"

#include "Game.hpp"
#include "AI.hpp"

#include <array>

namespace config
{
    constexpr int BOARD_PADDING = 20;
    const def::Vector2i BOARD_ORIGIN(BOARD_PADDING, BOARD_PADDING);
    const def::Vector2i BOARD_SIZE(8, 8);

    const std::array<std::string, 3> RULES
    {
        "Click a cell to reveal it.",
        "Right-click a cell to mark it as a mine.",
        "Mark all mines successfully to win!"
    };

    constexpr int MINES_COUNT = 8;
}

struct Rect2i
{
    def::Vector2i pos;
    def::Vector2i size;

    bool Contains(const def::Vector2i& p);
};

class App : public def::GameEngine
{
public:
    App();
    ~App();

private:
    bool OnUserCreate() override;
    bool OnUserUpdate(float deltaTime) override;

    void DrawCenteredText(const def::Vector2i& center, const std::string_view text, const def::Pixel& col);
    void DrawButton(const Rect2i& rect, const std::string_view text);

    void DrawWithinCell(const def::Vector2i& pos, const def::Graphic& gfx);

private:
    bool m_ShowInstructions = true;
    
    def::Graphic m_FlagImage;
    def::Graphic m_MineImage;

    def::Vector2i m_BoardScreenSize;
    def::Vector2i m_CellSize;

    std::unique_ptr<Minesweeper> m_Game;
    std::unique_ptr<MinesweeperAI> m_AI;

    bool m_Lost = false;

};