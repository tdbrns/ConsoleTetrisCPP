#include <iostream>
#include <Windows.h>
#include <thread>
#include <vector>

using namespace std;

wstring tetrominoes[7];
unsigned char* pField = nullptr;        // Elements of field are stored in unsigned char array

const int SCREEN_WIDTH = 120;
const int SCREEN_HEIGHT = 40;
const int FIELD_WIDTH = 12;
const int FIELD_HEIGHT = 18;
const int TETROMINO_BLOCK_WIDTH = 4;
const int TETROMINO_BLOCK_HEIGHT = 4;

// Prototypes
int RotateTetromino(int, int, int);
void DrawField(wchar_t*);
void DrawCurrentTetromino(wchar_t*, int, int, int, int);
bool DoesTetrominoFit(int, int, int, int);

int main()
{
    // All 4x4 tetromino blocks
    tetrominoes[0].append(L"..X.");
    tetrominoes[0].append(L"..X.");
    tetrominoes[0].append(L"..X.");
    tetrominoes[0].append(L"..X.");

    tetrominoes[1].append(L"..X.");
    tetrominoes[1].append(L".XX.");
    tetrominoes[1].append(L".X..");
    tetrominoes[1].append(L"....");

    tetrominoes[2].append(L".X..");
    tetrominoes[2].append(L".XX.");
    tetrominoes[2].append(L"..X.");
    tetrominoes[2].append(L"....");

    tetrominoes[3].append(L"....");
    tetrominoes[3].append(L".XX.");
    tetrominoes[3].append(L".XX.");
    tetrominoes[3].append(L"....");

    tetrominoes[4].append(L"..X.");
    tetrominoes[4].append(L".XX.");
    tetrominoes[4].append(L"..X.");
    tetrominoes[4].append(L"....");

    tetrominoes[5].append(L"....");
    tetrominoes[5].append(L".XX.");
    tetrominoes[5].append(L"..X.");
    tetrominoes[5].append(L"..X.");

    tetrominoes[6].append(L"....");
    tetrominoes[6].append(L".XX.");
    tetrominoes[6].append(L".X..");
    tetrominoes[6].append(L".X..");

    pField = new unsigned char[FIELD_WIDTH * FIELD_HEIGHT];
    for (int x = 0; x < FIELD_WIDTH; x++)
    {
        for (int y = 0; y < FIELD_HEIGHT; y++)
            // Conditional expression; 9 represents the border of the field ('#') and 0 represents the empty space within the field (' ')
            // 9 and 0 each represent an index of the string L" ABCDEFG=#" in the DrawField function
            pField[y * FIELD_WIDTH + x] = (x == 0 || x == FIELD_WIDTH - 1 || y == FIELD_HEIGHT - 1) ? 9 : 0;
    }

    // Turn the console into a screen buffer
    wchar_t* screen = new wchar_t[SCREEN_WIDTH * SCREEN_HEIGHT];
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
        screen[i] = L' ';
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
    DWORD dwBytesWritten = 0;

    bool boolKeys[4]{};
    bool rotateHeld = false;
    bool gameOver = false;
    int currentTetromino = 0;
    int currentRotation = 0;
    int tickLimit = 20;
    int tickCounter = 0;
    bool forceDown = false;
    int pieceCount = 0;
    int score = 0;
    vector<int> vectLines;

    // New tetromino pieces spawn at the center-top of the field
    int currentX = FIELD_WIDTH / 2;
    int currentY = 0;

    // Game loop
    while (!gameOver)
    {
        // 1 game tick is 50 ms. Tetromino will be forced to move one space down whenever the tick limit is reached; game starts with a tick limit
        // of 20 ticks
        this_thread::sleep_for(50ms);
        tickCounter++;
        forceDown = (tickCounter == tickLimit);

        // Controls
        // The four boolean values of controlKeys[] represent the state of each of the four keys used for game
        // Virtual-key codes: \x27 = right arrow   \x25 = left arrow   \x28 = down arrow   \x20 = spacebar
        for (int i = 0; i < 4; i++)
            boolKeys[i] = (0x8000 & GetAsyncKeyState((unsigned char)("\x27\x25\x28\x20"[i]))) != 0;

        // Conditional expressions to check is a move left, right, or down or a rotation is valid
        currentX += (boolKeys[0] && DoesTetrominoFit(currentTetromino, currentRotation, currentX + 1, currentY)) ? 1 : 0;
        currentX -= (boolKeys[1] && DoesTetrominoFit(currentTetromino, currentRotation, currentX - 1, currentY)) ? 1 : 0;
        currentY += (boolKeys[2] && DoesTetrominoFit(currentTetromino, currentRotation, currentX, currentY + 1)) ? 1 : 0;

        // Ensure that tetromino will only clockwise rotate once per spacebar hit
        if (boolKeys[3])
        {
            currentRotation += (rotateHeld == true && DoesTetrominoFit(currentTetromino, currentRotation + 1, currentX, currentY)) ? 1 : 0;
            rotateHeld = false;
        }
        else
            rotateHeld = true;

        // Forces the tetromino downward by 1 row if possible
        if (forceDown)
        {
            if (DoesTetrominoFit(currentTetromino, currentRotation, currentX, currentY + 1))
                currentY++;
            else
            {
                // Lock current piece in the field
                for (int x = 0; x < TETROMINO_BLOCK_WIDTH; x++)
                {
                    for (int y = 0; y < TETROMINO_BLOCK_HEIGHT; y++)
                    {
                        if (tetrominoes[currentTetromino][RotateTetromino(x, y, currentRotation)] == L'X')
                            pField[(currentY + y) * FIELD_WIDTH + (currentX + x)] = currentTetromino + 1;
                    }
                }

                pieceCount++;

                // If the piece count is a multiple of 10, decrease the time before the tetromino is forced down one space
                if (pieceCount % 10 == 0)
                    if (tickLimit >= 10)
                        tickLimit--;

                // Check for lines
                for (int y = 0; y < 4; y++)
                {
                    if (currentY + y < FIELD_HEIGHT - 1)
                    {
                        bool lineExists = true;

                        // A line exists within the field if none of the indexed spaces of the row is an empty space ' '; otherwise, lineExists is 
                        // set to false
                        for (int x = 1; x < FIELD_WIDTH - 1; x++)
                            lineExists &= (pField[(currentY + y) * FIELD_WIDTH + x]) != 0;

                        // If a line exists, the charaters in that line are set to '=' before being removed
                        if (lineExists == true)
                        {
                            for (int x = 1; x < FIELD_WIDTH - 1; x++)
                                pField[(currentY + y) * FIELD_WIDTH + x] = 8;

                            vectLines.push_back(currentY + y);
                        }
                    }
                }

                // Each successfully placed tetromino is worth 25 points
                score += 25;
                // The number of points earned by making a line increases exponentially with each line made
                // 1 is bit shifted to the left by the number of lines made simultaneously by Player; look at bit shift calculator
                if (!vectLines.empty())
                    score += (1 << vectLines.size()) * 100;     // Translation: score += 2^vectLines.size() * 100

                // Spawn new random tetromino
                currentX = FIELD_WIDTH / 2;
                currentY = 0;
                currentRotation = 0;
                currentTetromino = rand() % 7;

                if (DoesTetrominoFit(currentTetromino, currentRotation, currentX, currentY) == false)
                    gameOver = true;
            }

            tickCounter = 0;
        }

        DrawField(screen);
        DrawCurrentTetromino(screen, currentTetromino, currentRotation, currentX, currentY);

        // Print score; Note that swprintf_s() must be used to write to this version of the console
        swprintf_s(&screen[2 * SCREEN_WIDTH + FIELD_WIDTH + 6], 16, L"SCORE: %8d", score);

        if (!vectLines.empty())
        {
            // Display the line(s) of equal signs ('=') for 400ms
            WriteConsoleOutputCharacter(hConsole, screen, SCREEN_WIDTH * SCREEN_HEIGHT, { 0,0 }, &dwBytesWritten);
            this_thread::sleep_for(400ms);

            // Remove the line(s) and move the pieces above downward
            for (auto& line : vectLines)
            {
                for (int x = 1; x < FIELD_WIDTH - 1; x++)
                {
                    for (int y = line; y > 0; y--)
                        pField[y * FIELD_WIDTH + x] = pField[(y - 1) * FIELD_WIDTH + x];

                    pField[x] = 0;
                }
            }

            vectLines.clear();
        }

        // Displays each frame of the game to the console
        WriteConsoleOutputCharacter(hConsole, screen, SCREEN_WIDTH * SCREEN_HEIGHT, { 0,0 }, &dwBytesWritten);
    }

    // Revert back to default console buffer
    CloseHandle(hConsole);
    cout << "Game Over! Score: " << score << endl;
    system("pause");

    return 0;
}

void DrawField(wchar_t* screen)
{
    // Draw the field
    for (int x = 0; x < FIELD_WIDTH; x++)
    {
        for (int y = 0; y < FIELD_HEIGHT; y++)
            // Contents of L string (including the whitespace) represent all the components displayed in the game
            // '+ 2' is used as an offset the field
            screen[(y + 2) * SCREEN_WIDTH + (x + 2)] = L" ABCDEFG=#"[pField[y * FIELD_WIDTH + x]];
    }
}

void DrawCurrentTetromino(wchar_t* screen, int tetromino, int rotation, int positionX, int positionY)
{
        for (int x = 0; x < TETROMINO_BLOCK_WIDTH; x++)
        {
            for (int y = 0; y < TETROMINO_BLOCK_HEIGHT; y++)
            {
                if (tetrominoes[tetromino][RotateTetromino(x, y, rotation)] == L'X')
                    // Draw tetromino according to the position of its Xs; 
                    // pieceIndex + 65 uses ASCII values to determine the letter (A, B, C, D, E, F, or G) that will replace the Xs of the tetromino
                    screen[(positionY + y + 2) * SCREEN_WIDTH + (positionX + x + 2)] = tetromino + 65;
            }
        }
}

int RotateTetromino(int xIndex, int yIndex, int rotation)
{
    switch (rotation % 4)
    {
        case 0: return yIndex * 4 + xIndex;         // 0 degrees
        case 1: return 12 + yIndex - (xIndex * 4);  // 90 degrees
        case 2: return 15 - (yIndex * 4) - xIndex;  // 180 degrees
        case 3: return 3 - yIndex + (xIndex * 4);   // 270 degrees
        default: return 0;
    }
}

// positionX and positionY represent the location of the upper right corner of every tetromino's 4x4 block
bool DoesTetrominoFit(int tetromino, int rotation, int positionX, int positionY)
{
    for (int x = 0; x < TETROMINO_BLOCK_WIDTH; x++)
    {
        for (int y = 0; y < TETROMINO_BLOCK_HEIGHT; y++)
        {
            int pieceIndex = RotateTetromino(x, y, rotation);
            int fieldIndex = (positionY + y) * FIELD_WIDTH + (positionX + x);

            // Ensure that a space outside of the field is not checked
            if (positionX + x >= 0 && positionX + x < FIELD_WIDTH)
            {
                if (positionY + y >= 0 && positionY + y < FIELD_HEIGHT)
                {
                    // Collision detection; if the tetromino block contacts a tetromino that has already been placed, return false
                    if (tetrominoes[tetromino][pieceIndex] == L'X' && pField[fieldIndex] != 0)
                        return false;
                }
            }
        }
    }

    return true;
}