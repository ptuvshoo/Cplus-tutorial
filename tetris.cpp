// Tetris using SFML
#include <SFML/Graphics.hpp>
#include <array>
#include <vector>
#include <random>
#include <chrono>
#include <algorithm>
#include <optional>

using namespace sf;
using std::array;
using std::optional;
using std::size_t;
using std::vector;

// Board configuration
static constexpr int CELL_SIZE = 28;
static constexpr int COLS = 10;
static constexpr int ROWS = 20;
static constexpr int SIDE_PANEL_WIDTH = 200;
static constexpr int MARGIN = 10;
static constexpr int WINDOW_WIDTH = COLS * CELL_SIZE + SIDE_PANEL_WIDTH + MARGIN * 3;
static constexpr int WINDOW_HEIGHT = ROWS * CELL_SIZE + MARGIN * 2;
static constexpr int WINDOW_STYLE = Style::Titlebar | Style::Close;

// Gravity timings (seconds per cell); speeds up as level increases
static constexpr float GRAVITY_LEVELS[] = {
    0.8f, 0.7f, 0.6f, 0.5f, 0.4f, 0.35f, 0.3f, 0.25f, 0.20f, 0.18f,
    0.16f, 0.14f, 0.12f, 0.10f, 0.09f, 0.08f, 0.075f, 0.07f, 0.065f, 0.06f
};

// Tetromino shapes defined as 4 rotation states × 4 cells (x,y)
// Coordinates are in a 4x4 local grid
struct Offset { int x; int y; };
using Shape = array<array<Offset, 4>, 4>;

// I, O, T, S, Z, J, L
static const array<Shape, 7> SHAPES = {
    // I
    Shape{ array<Offset, 4>{ Offset{0,1}, {1,1}, {2,1}, {3,1} },
           array<Offset, 4>{ Offset{2,0}, {2,1}, {2,2}, {2,3} },
           array<Offset, 4>{ Offset{0,2}, {1,2}, {2,2}, {3,2} },
           array<Offset, 4>{ Offset{1,0}, {1,1}, {1,2}, {1,3} } },
    // O
    Shape{ array<Offset, 4>{ Offset{1,1}, {2,1}, {1,2}, {2,2} },
           array<Offset, 4>{ Offset{1,1}, {2,1}, {1,2}, {2,2} },
           array<Offset, 4>{ Offset{1,1}, {2,1}, {1,2}, {2,2} },
           array<Offset, 4>{ Offset{1,1}, {2,1}, {1,2}, {2,2} } },
    // T
    Shape{ array<Offset, 4>{ Offset{1,1}, {0,2}, {1,2}, {2,2} },
           array<Offset, 4>{ Offset{1,1}, {1,2}, {2,2}, {1,3} },
           array<Offset, 4>{ Offset{0,2}, {1,2}, {2,2}, {1,3} },
           array<Offset, 4>{ Offset{1,1}, {0,2}, {1,2}, {1,3} } },
    // S
    Shape{ array<Offset, 4>{ Offset{1,1}, {2,1}, {0,2}, {1,2} },
           array<Offset, 4>{ Offset{1,1}, {1,2}, {2,2}, {2,3} },
           array<Offset, 4>{ Offset{1,2}, {2,2}, {0,3}, {1,3} },
           array<Offset, 4>{ Offset{0,1}, {0,2}, {1,2}, {1,3} } },
    // Z
    Shape{ array<Offset, 4>{ Offset{0,1}, {1,1}, {1,2}, {2,2} },
           array<Offset, 4>{ Offset{2,1}, {1,2}, {2,2}, {1,3} },
           array<Offset, 4>{ Offset{0,2}, {1,2}, {1,3}, {2,3} },
           array<Offset, 4>{ Offset{1,1}, {0,2}, {1,2}, {0,3} } },
    // J
    Shape{ array<Offset, 4>{ Offset{0,1}, {0,2}, {1,2}, {2,2} },
           array<Offset, 4>{ Offset{1,1}, {2,1}, {1,2}, {1,3} },
           array<Offset, 4>{ Offset{0,2}, {1,2}, {2,2}, {2,3} },
           array<Offset, 4>{ Offset{1,1}, {1,2}, {0,3}, {1,3} } },
    // L
    Shape{ array<Offset, 4>{ Offset{2,1}, {0,2}, {1,2}, {2,2} },
           array<Offset, 4>{ Offset{1,1}, {1,2}, {1,3}, {2,3} },
           array<Offset, 4>{ Offset{0,2}, {1,2}, {2,2}, {0,3} },
           array<Offset, 4>{ Offset{0,1}, {1,1}, {1,2}, {1,3} } }
};

static const array<Color, 7> COLORS = {
    Color(0, 240, 240),   // I - cyan
    Color(240, 240, 0),   // O - yellow
    Color(160, 0, 240),   // T - purple
    Color(0, 240, 0),     // S - green
    Color(240, 0, 0),     // Z - red
    Color(0, 0, 240),     // J - blue
    Color(240, 160, 0)    // L - orange
};

struct Piece {
    int kind;      // 0..6
    int rotation;  // 0..3
    int x;         // board column
    int y;         // board row
};

class RandomBag7 {
public:
    RandomBag7() : rng(static_cast<unsigned>(std::chrono::high_resolution_clock::now().time_since_epoch().count())) {
        refill();
    }
    int next() {
        if (bagIndex >= 7) refill();
        return bag[bagIndex++];
    }
private:
    std::mt19937 rng;
    array<int, 7> bag{};
    int bagIndex = 7;
    void refill() {
        for (int i = 0; i < 7; ++i) bag[i] = i;
        std::shuffle(bag.begin(), bag.end(), rng);
        bagIndex = 0;
    }
};

class TetrisGame {
public:
    TetrisGame()
        : window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Tetris", WINDOW_STYLE) {
        window.setFramerateLimit(60);
        clearBoard();
        spawnNewPiece();
    }

    void run() {
        while (window.isOpen()) {
            handleInput();
            update();
            draw();
        }
    }

private:
    RenderWindow window;
    array<array<int, COLS>, ROWS> board{}; // -1 empty, otherwise 0..6 color index
    Piece current{};
    optional<Piece> ghost;
    int score = 0;
    int linesCleared = 0;
    int level = 0;
    bool isGameOver = false;
    bool isPaused = false;
    Clock gravityClock;
    Clock lateralRepeatClock;
    bool leftHeld = false, rightHeld = false, downHeld = false;

    void clearBoard() {
        for (auto &row : board) {
            row.fill(-1);
        }
    }

    void spawnNewPiece() {
        current.kind = bag.next();
        current.rotation = 0;
        current.x = COLS / 2 - 2;
        current.y = -1; // spawn above visible area
        if (!canPlace(current)) {
            isGameOver = true;
        }
        updateGhost();
        gravityClock.restart();
    }

    bool canPlace(const Piece &p) const {
        const auto &cells = SHAPES[p.kind][p.rotation];
        for (const auto &c : cells) {
            int bx = p.x + c.x;
            int by = p.y + c.y;
            if (bx < 0 || bx >= COLS) return false;
            if (by >= ROWS) return false;
            if (by >= 0 && board[by][bx] != -1) return false;
        }
        return true;
    }

    void lockPiece() {
        const auto &cells = SHAPES[current.kind][current.rotation];
        for (const auto &c : cells) {
            int bx = current.x + c.x;
            int by = current.y + c.y;
            if (by >= 0 && by < ROWS && bx >= 0 && bx < COLS) {
                board[by][bx] = current.kind;
            }
        }
        clearLines();
        spawnNewPiece();
    }

    void clearLines() {
        int cleared = 0;
        for (int r = ROWS - 1; r >= 0; --r) {
            bool full = true;
            for (int c = 0; c < COLS; ++c) {
                if (board[r][c] == -1) { full = false; break; }
            }
            if (full) {
                ++cleared;
                for (int rr = r; rr > 0; --rr) {
                    board[rr] = board[rr - 1];
                }
                board[0].fill(-1);
                ++r; // re-check the same row index after collapsing
            }
        }
        if (cleared > 0) {
            linesCleared += cleared;
            score += scoreForClears(cleared, level);
            level = std::min(19, linesCleared / 10);
        }
    }

    static int scoreForClears(int count, int lvl) {
        switch (count) {
            case 1: return 40 * (lvl + 1);
            case 2: return 100 * (lvl + 1);
            case 3: return 300 * (lvl + 1);
            case 4: return 1200 * (lvl + 1);
            default: return 0;
        }
    }

    void softDropStep() {
        Piece moved = current;
        moved.y += 1;
        if (canPlace(moved)) {
            current = moved;
            score += 1; // soft drop point
        } else {
            lockPiece();
        }
        updateGhost();
    }

    void gravityStep() {
        Piece moved = current;
        moved.y += 1;
        if (canPlace(moved)) {
            current = moved;
        } else {
            lockPiece();
        }
        updateGhost();
    }

    void hardDrop() {
        int dist = 0;
        Piece moved = current;
        while (true) {
            Piece next = moved;
            next.y += 1;
            if (canPlace(next)) { moved = next; ++dist; }
            else break;
        }
        current = moved;
        score += dist * 2; // hard drop points
        lockPiece();
        updateGhost();
    }

    void rotate(int dir) { // +1 CW, -1 CCW
        Piece rotated = current;
        rotated.rotation = (rotated.rotation + (dir > 0 ? 1 : 3)) % 4;

        // Simple wall kicks: try small horizontal offsets
        static const int kicks[] = {0, -1, 1, -2, 2};
        for (int k : kicks) {
            Piece test = rotated;
            test.x += k;
            if (canPlace(test)) { current = test; break; }
        }
        updateGhost();
    }

    void moveHorizontal(int dx) {
        Piece moved = current;
        moved.x += dx;
        if (canPlace(moved)) current = moved;
        updateGhost();
    }

    void updateGhost() {
        Piece g = current;
        while (true) {
            Piece next = g;
            next.y += 1;
            if (canPlace(next)) g = next; else break;
        }
        ghost = g;
    }

    float currentGravityInterval() const {
        return GRAVITY_LEVELS[level];
    }

    void handleInput() {
        Event e;
        while (window.pollEvent(e)) {
            if (e.type == Event::Closed) { window.close(); }
            if (e.type == Event::KeyPressed) {
                if (e.key.code == Keyboard::Escape) window.close();
                if (e.key.code == Keyboard::P) isPaused = !isPaused;
                if (isGameOver || isPaused) continue;
                if (e.key.code == Keyboard::Up || e.key.code == Keyboard::X) rotate(+1);
                if (e.key.code == Keyboard::Z) rotate(-1);
                if (e.key.code == Keyboard::Space) hardDrop();
                if (e.key.code == Keyboard::Left) { moveHorizontal(-1); leftHeld = true; rightHeld = false; lateralRepeatClock.restart(); }
                if (e.key.code == Keyboard::Right) { moveHorizontal(+1); rightHeld = true; leftHeld = false; lateralRepeatClock.restart(); }
                if (e.key.code == Keyboard::Down) { downHeld = true; }
            }
            if (e.type == Event::KeyReleased) {
                if (e.key.code == Keyboard::Left) leftHeld = false;
                if (e.key.code == Keyboard::Right) rightHeld = false;
                if (e.key.code == Keyboard::Down) downHeld = false;
            }
        }
    }

    void handleHeldKeys() {
        if (isGameOver || isPaused) return;
        // DAS (delayed auto shift) + ARR (auto repeat rate) approximation
        const float das = 0.18f;
        const float arr = 0.05f;
        if (leftHeld || rightHeld) {
            float t = lateralRepeatClock.getElapsedTime().asSeconds();
            if (t > das) {
                static float carry = 0.0f;
                float steps = (t - das + carry) / arr;
                int moves = static_cast<int>(steps);
                carry = (t - das + carry) - moves * arr;
                lateralRepeatClock.restart();
                for (int i = 0; i < moves; ++i) moveHorizontal(leftHeld ? -1 : +1);
            }
        }
        if (downHeld) {
            static Clock soft;
            if (soft.getElapsedTime().asSeconds() > 0.03f) { // fast soft drop
                soft.restart();
                softDropStep();
            }
        }
    }

    void update() {
        handleHeldKeys();
        if (isGameOver || isPaused) return;
        if (gravityClock.getElapsedTime().asSeconds() >= currentGravityInterval()) {
            gravityClock.restart();
            gravityStep();
        }
    }

    void drawCell(RenderTarget &rt, int gridX, int gridY, Color fill, bool outline = true) {
        RectangleShape rect({static_cast<float>(CELL_SIZE - 2), static_cast<float>(CELL_SIZE - 2)});
        rect.setPosition(MARGIN + gridX * CELL_SIZE + 1, MARGIN + gridY * CELL_SIZE + 1);
        rect.setFillColor(fill);
        if (outline) {
            rect.setOutlineThickness(1);
            rect.setOutlineColor(Color(20, 20, 20));
        }
        rt.draw(rect);
    }

    void drawBoard(RenderTarget &rt) {
        // Background and border
        RectangleShape bg({static_cast<float>(COLS * CELL_SIZE), static_cast<float>(ROWS * CELL_SIZE)});
        bg.setPosition(MARGIN, MARGIN);
        bg.setFillColor(Color(30, 30, 30));
        bg.setOutlineThickness(2);
        bg.setOutlineColor(Color(90, 90, 90));
        rt.draw(bg);

        // Grid
        for (int r = 0; r < ROWS; ++r) {
            for (int c = 0; c < COLS; ++c) {
                if (board[r][c] != -1) {
                    drawCell(rt, c, r, COLORS[board[r][c]]);
                } else {
                    RectangleShape cell({static_cast<float>(CELL_SIZE - 2), static_cast<float>(CELL_SIZE - 2)});
                    cell.setPosition(MARGIN + c * CELL_SIZE + 1, MARGIN + r * CELL_SIZE + 1);
                    cell.setFillColor(Color(40, 40, 40));
                    rt.draw(cell);
                }
            }
        }
    }

    void drawPiece(RenderTarget &rt, const Piece &p, Color tint, bool ghostPiece) {
        const auto &cells = SHAPES[p.kind][p.rotation];
        for (const auto &c : cells) {
            int gx = p.x + c.x;
            int gy = p.y + c.y;
            if (gy < 0) continue;
            Color color = tint;
            if (ghostPiece) color = Color(tint.r, tint.g, tint.b, 60);
            drawCell(rt, gx, gy, color, !ghostPiece);
        }
    }

    void drawTextLine(RenderTarget &rt, const sf::String &s, int px, int py, unsigned size, Color col, bool bold = false) {
        static bool fontLoaded = false;
        static Font font;
        if (!fontLoaded) {
            // Use SFML default if available; fall back to system if not
            // On macOS, SFML provides a default sans-serif
            font.loadFromFile("/System/Library/Fonts/Supplemental/Arial Unicode.ttf");
            fontLoaded = true;
        }
        Text t;
        t.setFont(font);
        t.setString(s);
        t.setCharacterSize(size);
        t.setFillColor(col);
        t.setPosition(static_cast<float>(px), static_cast<float>(py));
        if (bold) t.setStyle(Text::Bold);
        rt.draw(t);
    }

    void drawSidePanel(RenderTarget &rt) {
        int panelX = MARGIN * 2 + COLS * CELL_SIZE;
        RectangleShape panel({static_cast<float>(SIDE_PANEL_WIDTH), static_cast<float>(ROWS * CELL_SIZE)});
        panel.setPosition(panelX, MARGIN);
        panel.setFillColor(Color(20, 20, 26));
        panel.setOutlineThickness(2);
        panel.setOutlineColor(Color(90, 90, 90));
        rt.draw(panel);

        drawTextLine(rt, "TETRIS", panelX + 16, MARGIN + 10, 28, Color::White, true);
        drawTextLine(rt, "Score:", panelX + 16, MARGIN + 60, 18, Color(200,200,200));
        drawTextLine(rt, std::to_string(score), panelX + 16, MARGIN + 80, 24, Color::White, true);

        drawTextLine(rt, "Level:", panelX + 16, MARGIN + 120, 18, Color(200,200,200));
        drawTextLine(rt, std::to_string(level), panelX + 16, MARGIN + 140, 24, Color::White, true);

        drawTextLine(rt, "Lines:", panelX + 16, MARGIN + 180, 18, Color(200,200,200));
        drawTextLine(rt, std::to_string(linesCleared), panelX + 16, MARGIN + 200, 24, Color::White, true);

        drawTextLine(rt, "Controls:", panelX + 16, MARGIN + 250, 18, Color(200,200,200));
        drawTextLine(rt, "←/→ Move", panelX + 16, MARGIN + 272, 16, Color(180,180,180));
        drawTextLine(rt, "↓ Soft Drop", panelX + 16, MARGIN + 292, 16, Color(180,180,180));
        drawTextLine(rt, "Space Hard Drop", panelX + 16, MARGIN + 312, 16, Color(180,180,180));
        drawTextLine(rt, "Z/X Rotate", panelX + 16, MARGIN + 332, 16, Color(180,180,180));
        drawTextLine(rt, "P Pause", panelX + 16, MARGIN + 352, 16, Color(180,180,180));
        drawTextLine(rt, "ESC Quit", panelX + 16, MARGIN + 372, 16, Color(180,180,180));

        // Next piece preview
        int previewY = MARGIN + 410;
        drawTextLine(rt, "Next:", panelX + 16, previewY, 18, Color(200,200,200));
        // Render an approximation of the next piece using the current RNG state is tricky.
        // Instead, keep a one-piece lookahead by peeking from a separate bag copy is complex.
        // For simplicity, skip preview in this minimal version.

        if (isPaused) {
            RectangleShape overlay({static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)});
            overlay.setFillColor(Color(0,0,0,120));
            window.draw(overlay);
            drawTextLine(rt, "PAUSED", MARGIN + COLS*CELL_SIZE/2 - 60, WINDOW_HEIGHT/2 - 20, 36, Color::Yellow, true);
        }

        if (isGameOver) {
            RectangleShape overlay({static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT)});
            overlay.setFillColor(Color(0,0,0,180));
            window.draw(overlay);
            drawTextLine(rt, "GAME OVER", MARGIN + COLS*CELL_SIZE/2 - 100, WINDOW_HEIGHT/2 - 40, 40, Color::Red, true);
            drawTextLine(rt, "ESC to quit", MARGIN + COLS*CELL_SIZE/2 - 70, WINDOW_HEIGHT/2 + 10, 18, Color(220,220,220));
        }
    }

    void draw() {
        window.clear(Color(16, 16, 22));
        drawBoard(window);
        if (ghost.has_value()) drawPiece(window, *ghost, COLORS[current.kind], true);
        drawPiece(window, current, COLORS[current.kind], false);
        drawSidePanel(window);
        window.display();
    }

    RandomBag7 bag;
};

int main() {
    TetrisGame game;
    game.run();
    return 0;
}


