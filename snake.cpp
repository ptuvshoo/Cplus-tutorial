#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;
using namespace sf;

const int CELL_SIZE = 30;
const int GRID_WIDTH = 20;
const int GRID_HEIGHT = 20;
const int WINDOW_WIDTH = CELL_SIZE * GRID_WIDTH;
const int WINDOW_HEIGHT = CELL_SIZE * GRID_HEIGHT + 80; // Extra space for score and instructions
const float GAME_SPEED = 0.15f; // seconds per move
const int WINDOW_STYLE = Style::Titlebar | Style::Close;

enum Direction { UP, DOWN, LEFT, RIGHT, NONE };

struct Position {
    int x, y;
    Position(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

class SnakeGame {
private:
    RenderWindow window;
    vector<Position> snake;
    Position food;
    Direction dir;
    Direction nextDir;
    bool gameOver;
    int score;
    Clock gameClock;

    void generateFood() {
        do {
            food.x = rand() % GRID_WIDTH;
            food.y = rand() % GRID_HEIGHT;
        } while (isSnakePosition(food));
    }

    bool isSnakePosition(const Position& pos) {
        for (const auto& segment : snake) {
            if (segment == pos) return true;
        }
        return false;
    }

    void handleInput() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
                gameOver = true;
            }

            if (event.type == Event::KeyPressed) {
                switch (event.key.code) {
                    case Keyboard::Up:
                    case Keyboard::W:
                        if (dir != DOWN) nextDir = UP;
                        break;
                    case Keyboard::Down:
                    case Keyboard::S:
                        if (dir != UP) nextDir = DOWN;
                        break;
                    case Keyboard::Left:
                    case Keyboard::A:
                        if (dir != RIGHT) nextDir = LEFT;
                        break;
                    case Keyboard::Right:
                    case Keyboard::D:
                        if (dir != LEFT) nextDir = RIGHT;
                        break;
                    case Keyboard::Escape:
                        window.close();
                        gameOver = true;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    void moveSnake() {
        if (nextDir != NONE) {
            dir = nextDir;
            nextDir = NONE;
        }

        Position head = snake[0];
        
        switch (dir) {
            case UP:
                head.y--;
                break;
            case DOWN:
                head.y++;
                break;
            case LEFT:
                head.x--;
                break;
            case RIGHT:
                head.x++;
                break;
            default:
                return;
        }

        // Check wall collision
        if (head.x < 0 || head.x >= GRID_WIDTH || head.y < 0 || head.y >= GRID_HEIGHT) {
            gameOver = true;
            return;
        }

        // Check self collision
        if (isSnakePosition(head)) {
            gameOver = true;
            return;
        }

        snake.insert(snake.begin(), head);

        // Check food collision
        if (head == food) {
            score++;
            generateFood();
        } else {
            snake.pop_back();
        }
    }

    void draw() {
        window.clear(Color(30, 30, 30)); // Dark gray background

        // Draw game area border
        RectangleShape gameArea(Vector2f(WINDOW_WIDTH, CELL_SIZE * GRID_HEIGHT));
        gameArea.setFillColor(Color::Transparent);
        gameArea.setOutlineColor(Color(100, 100, 100));
        gameArea.setOutlineThickness(2);
        gameArea.setPosition(0, 0);
        window.draw(gameArea);

        // Draw food
        RectangleShape foodRect(Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
        foodRect.setPosition(food.x * CELL_SIZE + 1, food.y * CELL_SIZE + 1);
        foodRect.setFillColor(Color(255, 50, 50)); // Red
        window.draw(foodRect);

        // Draw snake
        for (size_t i = 0; i < snake.size(); i++) {
            RectangleShape segment(Vector2f(CELL_SIZE - 2, CELL_SIZE - 2));
            int x = snake[i].x * CELL_SIZE + 1;
            int y = snake[i].y * CELL_SIZE + 1;
            segment.setPosition(x, y);
            
            if (i == 0) {
                segment.setFillColor(Color(50, 255, 50)); // Bright green for head
            } else {
                segment.setFillColor(Color(0, 200, 0)); // Darker green for body
            }
            window.draw(segment);
        }

        // Draw score and instructions
        Text scoreText;
        scoreText.setString("Score: " + to_string(score));
        scoreText.setCharacterSize(20);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(10, CELL_SIZE * GRID_HEIGHT + 5);
        window.draw(scoreText);

        Text instructionsText;
        instructionsText.setString("Use Arrow Keys or WASD to move | ESC to quit");
        instructionsText.setCharacterSize(14);
        instructionsText.setFillColor(Color(200, 200, 200));
        instructionsText.setPosition(10, CELL_SIZE * GRID_HEIGHT + 30);
        window.draw(instructionsText);

        // Draw game over message
        if (gameOver) {
            RectangleShape overlay(Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
            overlay.setFillColor(Color(0, 0, 0, 220)); // Semi-transparent black
            window.draw(overlay);

            Text gameOverText;
            gameOverText.setString("GAME OVER!");
            gameOverText.setCharacterSize(36);
            gameOverText.setFillColor(Color::Red);
            gameOverText.setStyle(Text::Bold);
            FloatRect textRect = gameOverText.getLocalBounds();
            gameOverText.setOrigin(textRect.left + textRect.width / 2.0f,
                                 textRect.top + textRect.height / 2.0f);
            gameOverText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 30);
            window.draw(gameOverText);

            Text scoreFinalText;
            scoreFinalText.setString("Final Score: " + to_string(score));
            scoreFinalText.setCharacterSize(24);
            scoreFinalText.setFillColor(Color::White);
            textRect = scoreFinalText.getLocalBounds();
            scoreFinalText.setOrigin(textRect.left + textRect.width / 2.0f,
                                   textRect.top + textRect.height / 2.0f);
            scoreFinalText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 10);
            window.draw(scoreFinalText);

            Text exitText;
            exitText.setString("Press ESC or close window to exit");
            exitText.setCharacterSize(16);
            exitText.setFillColor(Color(180, 180, 180));
            textRect = exitText.getLocalBounds();
            exitText.setOrigin(textRect.left + textRect.width / 2.0f,
                             textRect.top + textRect.height / 2.0f);
            exitText.setPosition(WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f + 45);
            window.draw(exitText);
        }

        window.display();
    }

public:
    SnakeGame() : window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Snake Game", WINDOW_STYLE),
                  dir(RIGHT), nextDir(NONE), gameOver(false), score(0) {
        window.setFramerateLimit(60); // Smooth rendering
        window.setKeyRepeatEnabled(false); // Prevent key repeat
        
        // Initialize snake in the center
        snake.push_back(Position(GRID_WIDTH / 2, GRID_HEIGHT / 2));
        snake.push_back(Position(GRID_WIDTH / 2 - 1, GRID_HEIGHT / 2));
        snake.push_back(Position(GRID_WIDTH / 2 - 2, GRID_HEIGHT / 2));
        
        generateFood();
        srand(time(0));
        gameClock.restart();
    }

    void run() {
        // Main game loop - window runs independently
        while (window.isOpen()) {
            handleInput();
            
            if (!gameOver) {
                // Move snake at fixed intervals
                if (gameClock.getElapsedTime().asSeconds() >= GAME_SPEED) {
                    moveSnake();
                    gameClock.restart();
                }
            }
            
            draw();
        }
    }
};

int main() {
    SnakeGame game;
    game.run();
    return 0;
}
