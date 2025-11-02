# Snake Game with Graphical Interface

A Snake game built with C++ and SFML graphics library.

## Installation

### macOS

#### Option 1: Using Homebrew (Recommended)
```bash
brew install sfml
```

#### Option 2: Using MacPorts
```bash
sudo port install sfml
```

#### Option 3: Manual Installation
Download SFML from https://www.sfml-dev.org/download.php and follow the installation instructions.

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install libsfml-dev
```

### Linux (Fedora)
```bash
sudo dnf install SFML-devel
```

## Compilation

Using the Makefile:
```bash
make
```

Or manually:
```bash
g++ -std=c++11 snake.cpp -o snake -lsfml-graphics -lsfml-window -lsfml-system
```

## Running

```bash
./snake
```

A window will open automatically - the game runs in a graphical window, not in the terminal!

## How to Play

### Objective
Control the snake to eat the red food. Each time you eat food, your snake grows longer and your score increases by 1.

### Controls
- **Arrow Keys** (↑ ↓ ← →) or **WASD** keys: Change the snake's direction
  - ↑ or **W**: Move up
  - ↓ or **S**: Move down
  - ← or **A**: Move left
  - → or **D**: Move right
- **ESC**: Quit the game

### Gameplay Rules
1. The snake moves automatically in the direction you choose
2. You cannot reverse direction directly into yourself (e.g., if moving right, you can't immediately go left)
3. The game ends if you:
   - Hit a wall (the edge of the game area)
   - Collide with your own tail
4. When you eat food (red square):
   - Your snake grows by one segment
   - A new food appears randomly on the board
   - Your score increases

### Game Features

- **20×20 grid** with graphical interface
- **Smooth movement** - 60 FPS rendering
- **Real-time score** displayed in the window
- **Visual feedback** - green snake, red food
- **Game over screen** shows your final score
- **On-screen instructions** for controls

### Tips
- Plan your moves ahead to avoid trapping yourself
- Use the walls strategically to make turns
- The longer your snake gets, the more challenging it becomes!

