#include <string>

#include <raylib.h>

void Init();
void Update();
void Draw();

void DrawCell(int x, int y, int size);
void FloodFill(int x, int y);

struct Cell
{
    int value = 0;
    bool isBomb = false;
    bool flagged = false;
    bool hovered = false;
    bool clicked = false;
};
Cell **board = nullptr;

const int boardWidth = 10;
const int boardHeight = 10;
const int cellSize = 40;

int flagCount = 0;
int bombCount = 0;
int correctFlags = 0;
bool gameStart = false;
bool gameOver = false;
bool gameWon = false;

int main()
{
    const int screenWidth = 600;
    const int screenHeight = 600;

    InitWindow(screenWidth, screenHeight, "Minesweeper");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);

    // Init
    board = new Cell*[boardHeight];
    for (int i = 0; i < boardHeight; ++i)
    {
        board[i] = new Cell[boardWidth];
    }
    if (board == nullptr)
        return 0;

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
    for (int i = 0; i < boardHeight; ++i)
    {
        delete[] board[i];
    }
    delete[] board;

    CloseWindow();

	return 0;
}

void Init()
{
    // Set bombs & defaults
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

    // Set values based on neighbors.
    const int neighbors[9][2] = // Neighbor lookup
    {
        { -1,  1 }, { 0,  1 }, { 1,  1 },
        { -1,  0 }, { 0,  0 }, { 1,  0 },
        { -1, -1 }, { 0, -1 }, { 1, -1 }
    };

    for (int i = 0; i < boardHeight; i++)
    {
        for (int j = 0; j < boardWidth; j++)
        {
            if (board[j][i].isBomb)
                continue;

            int neighborBombs = 0;
            for (const int *offset : neighbors)
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
    if (gameOver || gameWon)
        return;

    int mouseX = GetMouseX() / cellSize;
    int mouseY = GetMouseY() / cellSize;

    for (int i = 0; i < boardHeight; i++)
    {
        for (int j = 0; j < boardWidth; j++)
        {
            if (mouseX == j && mouseY == i)
            {
                board[j][i].hovered = true;
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
                            FloodFill(j, i-1);
                            FloodFill(j+1, i);
                            FloodFill(j, i+1);

                            FloodFill(j-1, i-1);
                            FloodFill(j+1, i-1);
                            FloodFill(j+1, i+1);
                            FloodFill(j-1, i+1);
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
                DrawCell(j, i, cellSize);
            }
        }
    }

    // Draw UI.
    std::string flagText = "Flags: " + std::to_string(flagCount);
    DrawText(flagText.c_str(), boardWidth * cellSize + cellSize, boardHeight * cellSize - cellSize/2, cellSize/2, BLACK);

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

void DrawCell(int x, int y, int size)
{
    Cell &cell = board[x][y];

    int halfSize = size/2;

    std::string text = std::to_string(cell.value);

    int fontSize = size/2;
    int textHalfWidth = MeasureText(text.c_str(), fontSize) / 2;
    int textHalfHeight = fontSize / 2;

    Color fillColor = LIGHTGRAY;
    if (cell.clicked)
        fillColor = WHITE;
    else if (cell.hovered)
        fillColor = BLUE;

    DrawRectangle(x * size, y * size, size, size, fillColor);
    DrawRectangleLines(x * size, y * size, size, size, BLACK);

    bool drawText = true;
    if (cell.isBomb)
    {
        text = "B";
        drawText = true;
    }
    else if (cell.value <= 0)
        drawText = false;

    if (cell.clicked || cell.flagged)
    {
        if (cell.flagged == true)
            text = "F";

        if (drawText == true)
            DrawText(text.c_str(), x * size + halfSize - textHalfWidth, y * size + halfSize - textHalfHeight, fontSize, BLACK);
    }
}

void FloodFill(int x, int y)
{
    if (x < 0 || x >= boardWidth || y < 0 || y >= boardHeight)
        return;
    if (board[x][y].value > 1 || board[x][y].isBomb == true || board[x][y].clicked == true)
        return;

    board[x][y].clicked = true;

    FloodFill(x-1, y);
    FloodFill(x, y-1);
    FloodFill(x+1, y);
    FloodFill(x, y+1);

    FloodFill(x-1, y-1);
    FloodFill(x+1, y-1);
    FloodFill(x+1, y+1);
    FloodFill(x-1, y+1);
}