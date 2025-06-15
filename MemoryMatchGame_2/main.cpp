using namespace std;

void displayBackground(int level);
void initializeGame(int level);
void displayBoard();
bool checkMatch();
void undoMove();
bool isValidMove(int row, int col, bool checkPrevious = false);
bool playTurn(int& turns);
int countMatches();
void showMenu();
void playLevel1();
void playLevel2();
void playLevel3();
void playLevel4();
void playLevel5();


#include <iostream>
#include <map>
#include <set>
#include <list>
#include <stack>
#include <queue>
#include <algorithm>
#include <random>
#include <chrono>
#include <thread>
#include <limits>

const int MAX_LEVELS = 5;
int currentBoardSize = 4;
map<pair<int, int>, int> board;
set<pair<int, int>> flipped;
set<pair<int, int>> matched; // Track permanently matched cards
list<int> cardPool;
stack<map<pair<int, int>, int>> gameState;
queue<pair<int, int>> moveHistory;

void displayBackground(int level)
{
    // Background logic handled in level functions
}

void displayWithBorder(const string& text)
{
    int width = text.length() + 4;
    string topBottomBorder(width, '-');

    cout << topBottomBorder << "\n";
    cout << "| " 
         << text 
         << " |\n";
    cout << topBottomBorder << "\n";
}

void initializeGame(int level)
{
    currentBoardSize = 2 * level; // 2, 4, 6, 8, 10 for levels 1 to 5
    int totalCards = currentBoardSize * currentBoardSize;
    int pairs = totalCards / 2;
    list<int> cardValues;

    for (int i = 1; i <= pairs; ++i)
    {
        cardValues.push_back(i);
        cardValues.push_back(i);
    }

    random_device rd;
    mt19937 g(rd());
    list<int> shuffledCards = cardValues;
    int tempArray[totalCards];
    int index = 0;

    for (auto it = shuffledCards.begin(); it != shuffledCards.end(); ++it)
    {
        tempArray[index] = *it;
        index++;
    }

    shuffle(tempArray, tempArray + totalCards, g);
    shuffledCards.clear();

    for (int i = 0; i < totalCards; ++i)
    {
        shuffledCards.push_back(tempArray[i]);
    }

    board.clear();
    index = 0;

    for (int i = 0; i < currentBoardSize; ++i)
    {
        for (int j = 0; j < currentBoardSize; ++j)
        {
            board[make_pair(i, j)] = shuffledCards.front();
            shuffledCards.pop_front();
        }
    }

    gameState.push(board);
    flipped.clear();
    matched.clear(); // Reset matched cards at the start of a new game
}

void displayBoard()
{
    // Print column numbers
    cout << "     "; // Space for row label
    for (int j = 0; j < currentBoardSize; ++j)
    {
        cout << j + 1 
             << " ";
    }
    cout << "\n";

    // Print separator below column numbers
    string colSeparator(currentBoardSize * 2 + 1, '_');
    cout << "    " 
         << colSeparator 
         << "\n";

    // Print rows with row numbers and right border
    for (int i = 0; i < currentBoardSize; ++i)
    {
        cout << i + 1 
             << "   |";

        for (int j = 0; j < currentBoardSize; ++j)
        {
            if (flipped.count(make_pair(i, j)) || matched.count(make_pair(i, j)))
            {
                cout << board[make_pair(i, j)] 
                     << " ";
            }
            else
            {
                cout << "- ";
            }
        }

        cout << "|\n";
    }

    // Print bottom border
    string bottomBorder(currentBoardSize * 2 + 1, '-');
    cout << "    " 
         << bottomBorder 
         << "\n";
}

bool checkMatch()
{
    if (flipped.size() != 2)
    {
        return false;
    }
    else
    {
        auto it1 = flipped.begin();
        auto it2 = next(it1);
        return board[*it1] == board[*it2];
    }
}

void undoMove()
{
    if (gameState.size() > 1)
    {
        gameState.pop();
        board = gameState.top();
        flipped.clear();
        
        // Since matched cards are permanent, we don't clear them on undo
        if (!moveHistory.empty())
        {
            moveHistory.pop();
        }
    }
}

bool isValidMove(int row, int col, bool checkPrevious)
{
    bool validBounds = row >= 0 && row < currentBoardSize && col >= 0 && col < currentBoardSize && !flipped.count(make_pair(row, col)) && !matched.count(make_pair(row, col));

    if (!validBounds)
    {
        return false;
    }

    if (checkPrevious && !moveHistory.empty())
    {
        auto lastMove = moveHistory.back();

        if (lastMove.first == row && lastMove.second == col)
        {
            return false;
        }
    }

    return true;
}

bool playTurn(int& turns)
{
    int row1, col1, row2, col2;
    bool validTurn = false;

    // First card
    while (!validTurn)
    {
        displayWithBorder("Enter first card (row col 0-" + to_string(currentBoardSize - 1) + "): ");

        if (!(cin >> row1 >> col1))
        {
            displayWithBorder("Invalid input! Please enter two numbers.");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard(); // Display board after invalid input
            continue;
        }
        else
        {
            if (!isValidMove(row1 - 1, col1 - 1, true)) // Adjust for 1-based input
            {
                displayWithBorder("Invalid move! Try again.");
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                displayBoard(); // Display board after invalid input
                continue;
            }
            else
            {
                flipped.insert(make_pair(row1 - 1, col1 - 1));
                moveHistory.push(make_pair(row1 - 1, col1 - 1));
                displayBoard();
                validTurn = true;
            }
        }
    }

    // Second card
    validTurn = false;

    while (!validTurn)
    {
        displayWithBorder("Enter second card (row col 0-" + to_string(currentBoardSize - 1) + "): ");

        if (!(cin >> row2 >> col2))
        {
            displayWithBorder("Invalid input! Please enter two numbers.");
            flipped.erase(make_pair(row1 - 1, col1 - 1));
            moveHistory.pop();
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard(); // Display board after invalid input
            continue;
        }
        else
        {
            if (!isValidMove(row2 - 1, col2 - 1, true))
            {
                displayWithBorder("Invalid move! Try again.");
                flipped.erase(make_pair(row1 - 1, col1 - 1));
                moveHistory.pop();
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                displayBoard(); // Display board after invalid input
                return false; // Retry the entire turn
            }
            else
            {
                flipped.insert(make_pair(row2 - 1, col2 - 1));
                moveHistory.push(make_pair(row2 - 1, col2 - 1));
                displayBoard();
                validTurn = true;
            }
        }
    }

    // Process the turn
    if (checkMatch())
    {
        // Move matched cards from flipped to matched
        auto it1 = flipped.begin();
        auto it2 = next(it1);
        matched.insert(*it1);
        matched.insert(*it2);
        flipped.clear();
        displayWithBorder("Match found!");
    }
    else
    {
        displayWithBorder("No match. Flipping back...");
        this_thread::sleep_for(chrono::seconds(1));
        flipped.clear();
        displayBoard();
    }

    gameState.push(board);
    turns++;
    return true;
}

int countMatches()
{
    return matched.size();
}

void showMenu()
{
    cout << "=== Memory Match Game ===\n";
    cout << "1. Level 1 (2x2)\n";
    cout << "2. Level 2 (4x4)\n";
    cout << "3. Level 3 (6x6)\n";
    cout << "4. Level 4 (8x8)\n";
    cout << "5. Level 5 (10x10)\n";
    cout << "6. Quit\n";
    displayWithBorder("Select a level: ");
}

void runGameLevel(int level, const string& background)
{
    initializeGame(level);
    cout << background << "\n"; // Display custom 10x10 background
    displayBoard();
    int turns = 0;

    while (countMatches() < currentBoardSize * currentBoardSize)
    {
        bool turnCompleted = playTurn(turns);
    }

    cout << "Congratulations! You won Level " 
         << level 
         << " in " 
         << turns 
         << " turns!\n";

    flipped.clear();
    matched.clear();
    gameState = stack<map<pair<int, int>, int>>();
    moveHistory = queue<pair<int, int>>();
}

void playLevel1()
{
    const string background = 
        "Level 1: Big Square\n"
        "  ++++++++++ \n"
        "  +       + \n"
        "  +       + \n"
        "  +       + \n"
        "  +       + \n"
        "  +       + \n"
        "  +       + \n"
        "  +       + \n"
        "  +       + \n"
        "  ++++++++++ \n";
    runGameLevel(1, background);
}

void playLevel2()
{
    const string background = 
        "Level 2: Tall Tree\n"
        "      /\\    \n"
        "     /  \\   \n"
        "    /    \\  \n"
        "   /______\\ \n"
        "   |  *** | \n"
        "   |  *** | \n"
        "   |  *** | \n"
        "   |  *** | \n"
        "   |______| \n"
        "      ||    \n";
    runGameLevel(2, background);
}

void playLevel3()
{
    const string background = 
        "Level 3: Wave Crash\n"
        "  ~~~~~~~~~~ \n"
        " ~          ~\n"
        "  ~        ~ \n"
        "   ~      ~  \n"
        "    ~    ~   \n"
        "     ~  ~    \n"
        "      ~~     \n"
        "     ~~~~    \n"
        "    ~~~~~~   \n"
        " ~~~~~~~~~~  \n";
    runGameLevel(3, background);
}

void playLevel4()
{
    const string background = 
        "Level 4: Sharp Triangle\n"
        "        /\\    \n"
        "       /  \\   \n"
        "      /    \\  \n"
        "     /      \\ \n"
        "    /________\\\n"
        "    |  *** |  \n"
        "    |  *** |  \n"
        "    |______|  \n"
        "      |||     \n"
        "      |||     \n";
    runGameLevel(4, background);
}

void playLevel5()
{
    const string background = 
        "Level 5: Bright Star\n"
        "    *     *    \n"
        "   * *   * *   \n"
        "  *   * *   *  \n"
        " *     *     * \n"
        "  *   ***   *  \n"
        "   * *   * *   \n"
        "    *     *    \n"
        "   ***   ***   \n"
        "  *     *     \n"
        " ***********   \n";
    runGameLevel(5, background);
}

int main()
{
    int choice;

    while (true)
    {
        showMenu();
        cin >> choice;

        if (choice == 6)
        {
            break;
        }

        if (choice < 1 || choice > 5)
        {
            displayWithBorder("Invalid choice!");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        else
        {
            switch (choice)
            {
                case 1:
                    playLevel1();
                    break;
                case 2:
                    playLevel2();
                    break;
                case 3:
                    playLevel3();
                    break;
                case 4:
                    playLevel4();
                    break;
                case 5:
                    playLevel5();
                    break;
            }
        }
    }

    cout << "Thanks for playing!\n";
    return 0;
}