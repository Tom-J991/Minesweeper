#include <string>

#include <raylib.h>

void Init();
void Update();
void Draw();

void DrawCell(int x, int y);
void DrawRestart(int x, int y);
void FloodFill(int x, int y);

struct Cell
{
    int value = 0;
    bool isBomb = false;
    bool flagged = false;
    bool hovered = false;
    bool held = false;
    bool clicked = false;
};
Cell **board = nullptr;

Cell *restartBtn = nullptr;

const int cellNeighbors[9][2] = // Neighbor lookup
{
    { -1,  1 }, { 0,  1 }, { 1,  1 },
    { -1,  0 }, { 0,  0 }, { 1,  0 },
    { -1, -1 }, { 0, -1 }, { 1, -1 }
};

const int boardWidth = 9;
const int boardHeight = 9;
const int cellSize = 64;

int flagCount = 0;
int bombCount = 0;
int correctFlags = 0;

bool gameStart = false;
bool gameOver = false;
bool gameWon = false;

bool firstClick = false;

int main()
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Minesweeper");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    // Init
    board = new Cell*[boardWidth];
    for (int i = 0; i < boardWidth; ++i)
    {
        board[i] = new Cell[boardHeight];
    }
    if (board == nullptr)
        return 0;

    restartBtn = new Cell();

    Init();

    // Game Loop
    while (!WindowShouldClose())
    {
        Update();

        BeginDrawing();
        ClearBackground(RAYWHITE);
        {
            Draw();
        }
        EndDrawing();
    }

    // Cleanup
    for (int i = 0; i < boardWidth; ++i)
    {
        delete[] board[i];
    }
    delete[] board;

    delete restartBtn;

    CloseWindow();

	return 0;
}

void Init()
{
    gameStart = false;
    gameWon = false;
    gameOver = false;

    firstClick = false;

    // Set bombs & defaults
    bombCount = 0;
    for (int i = 0; i < boardHeight; i++)
    {
        for (int j = 0; j < boardWidth; j++)
        {
            int max = boardWidth/2;
            int chance = GetRandomValue(0, max);
            if (chance == max/2)
            {
                board[j][i] = { 0, true };
                bombCount++;
            }
            else
            {
                board[j][i] = { 0, false };
            }
        }
    }
    flagCount = bombCount;
    correctFlags = 0;

    // Set values based on neighbors.
    for (int i = 0; i < boardHeight; i++)
    {
        for (int j = 0; j < boardWidth; j++)
        {
            if (board[j][i].isBomb)
                continue;

            int neighborBombs = 0;
            for (const int *offset : cellNeighbors)
            {
                if (j + offset[0] < 0 || j + offset[0] >= boardWidth || i + offset[1] < 0 || i + offset[1] >= boardHeight)
                    continue;
                if (board[j + offset[0]][i + offset[1]].isBomb == true)
                    neighborBombs++;
            }

            board[j][i].value = neighborBombs;
        }
    }
}

void Update()
{
    // Game Logic
    int mouseX = GetMouseX() / cellSize;
    int mouseY = GetMouseY() / cellSize;

    // Handle Restart Button
    if (IsMouseButtonUp(MOUSE_BUTTON_LEFT))
    {
        restartBtn->held = false;
        restartBtn->clicked = false;
    }
    if (mouseX == boardWidth + 1 && mouseY == 1)
    {
        restartBtn->hovered = true;
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
        {
            restartBtn->held = true;
        }
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
        {
            restartBtn->clicked = true;
            Init();
        }
    }
    else
    {
        restartBtn->hovered = false;
    }

    if (gameOver || gameWon) // Don't update board if won or lost.
        return;

    // Update board.
    for (int i = 0; i < boardHeight; i++)
    {
        for (int j = 0; j < boardWidth; j++)
        {
            if (mouseX == j && mouseY == i)
            {
                board[j][i].hovered = true;
                if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
                {
                    if (board[j][i].clicked == false)
                        restartBtn->held = true;
                }
                if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
                {
                    if (board[j][i].flagged == false)
                    {
                        board[j][i].clicked = true;
                        if (board[j][i].isBomb == true)
                            gameOver = true;
                        else if (board[j][i].value == 0)
                        {
                            FloodFill(j-1, i);
                            FloodFill(j,   i-1);
                            FloodFill(j+1, i);
                            FloodFill(j,   i+1);
                        }
                    }
                }
                else if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT))
                {
                    if (board[j][i].clicked == false)
                    {
                        if (board[j][i].flagged == true)
                        {
                            board[j][i].flagged = false;
                            if (board[j][i].isBomb == true)
                                correctFlags--;
                            flagCount++;
                        }
                        else
                        {
                            board[j][i].flagged = true;
                            if (board[j][i].isBomb == true)
                                correctFlags++;
                            flagCount--;
                        }
                    }
                }
            }
            else
            {
                board[j][i].hovered = false;
            }
        }
    }

    // Check win condition.
    if (correctFlags == bombCount)
    {
        gameWon = true;
    }
}

void Draw()
{
    // Draw board.
    if (!(gameOver || gameWon))
    {
        for (int i = 0; i < boardHeight; i++)
        {
            for (int j = 0; j < boardWidth; j++)
            {
                DrawCell(j, i);
            }
        }
    }

    // Draw UI.
    std::string flagText = "Flags: " + std::to_string(flagCount);
    DrawText(flagText.c_str(), boardWidth * cellSize + cellSize, boardHeight * cellSize - cellSize/2, cellSize/2, BLACK);

    DrawRestart(boardWidth + 1, 1);

    if (gameOver)
    {
        int halfTextWidth = MeasureText("Game Over!", cellSize) / 2;
        int halfTextHeight = cellSize/2;
        DrawText("Game Over!", (boardWidth * cellSize)/2 - halfTextWidth, (boardHeight * cellSize)/2 - halfTextHeight, cellSize, RED);
    }
    else if (gameWon)
    {
        int halfTextWidth = MeasureText("You Won!", cellSize) / 2;
        int halfTextHeight = cellSize/2;
        DrawText("You Won!", (boardWidth * cellSize)/2 - halfTextWidth, (boardHeight * cellSize)/2 - halfTextHeight, cellSize, ORANGE);
    }
}

void DrawCell(int x, int y)
{
    Cell &cell = board[x][y];

    int halfSize = cellSize/2;

    std::string text = std::to_string(cell.value);

    int fontSize = cellSize/2;
    int textHalfWidth = MeasureText(text.c_str(), fontSize) / 2;
    int textHalfHeight = fontSize / 2;

    Color fillColor = LIGHTGRAY;
    if (cell.clicked)
        fillColor = WHITE;
    else if (cell.hovered)
        fillColor = BLUE;

    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, fillColor);
    DrawRectangleLines(x * cellSize, y * cellSize, cellSize, cellSize, BLACK);

    bool drawText = true;
    if (cell.isBomb)
    {
        text = "B";
        drawText = true;
    }
    else if (cell.value <= 0 && !cell.flagged)
        drawText = false;

    if (cell.clicked || cell.flagged)
    {
        if (cell.flagged == true)
            text = "F";

        if (drawText == true)
            DrawText(text.c_str(), x * cellSize + halfSize - textHalfWidth, y * cellSize + halfSize - textHalfHeight, fontSize, BLACK);
    }
}

void DrawRestart(int x, int y)
{
    int halfSize = cellSize / 2;

    std::string text = ":)";

    int fontSize = cellSize / 2;
    int textHalfWidth = MeasureText(text.c_str(), fontSize) / 2;
    int textHalfHeight = fontSize / 2;

    Color fillColor = LIGHTGRAY;
    if (restartBtn->clicked)
        fillColor = WHITE;
    else if (restartBtn->hovered)
        fillColor = BLUE;

    if (restartBtn->held)
        text = ":O";

    if (gameOver)
    {
        text = "x(";
    }

    DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, fillColor);
    DrawRectangleLines(x * cellSize, y * cellSize, cellSize, cellSize, BLACK);
    DrawText(text.c_str(), x * cellSize + halfSize - textHalfWidth, y * cellSize + halfSize - textHalfHeight, fontSize, BLACK);
}

void FloodFill(int x, int y)
{
    if (x < 0 || x >= boardWidth || y < 0 || y >= boardHeight)
        return;
    if (board[x][y].isBomb == true || board[x][y].clicked == true)
        return;

    int emptyNeighbors = 0;
    for (const int *offset : cellNeighbors)
    {
        if (x + offset[0] < 0 || x + offset[0] >= boardWidth || y + offset[1] < 0 || y + offset[1] >= boardHeight)
            continue;
        if (board[x + offset[0]][y + offset[1]].value <= 0 && board[x + offset[0]][y + offset[1]].isBomb == false)
            emptyNeighbors++;
    }

    if (emptyNeighbors <= 0)
        return;

    board[x][y].clicked = true;

    FloodFill(x-1, y);
    FloodFill(x,   y-1);
    FloodFill(x+1, y);
    FloodFill(x,   y+1);
}
