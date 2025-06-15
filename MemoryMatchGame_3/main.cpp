using namespace std;

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

const int MAX_LEVELS = 8;
int currentBoardSize = 4;
map<pair<int, int>, int> board;
set<pair<int, int>> flipped;
set<pair<int, int>> matched;
list<int> cardPool;
stack<map<pair<int, int>, int>> gameState;
queue<pair<int, int>> moveHistory;

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
void playLevel6();
void playLevel7();
void playLevel8();

void displayWithBorder(const string& text)
{
    int width = text.length() + 4;
    string topBottomBorder(width, '-');

    cout << topBottomBorder << "\n";
    cout << "| " << text << " |\n";
    cout << topBottomBorder << "\n";
}

//Starts the game
void initializeGame(int level)
{
    //Set the board size
    currentBoardSize = 2 * level;
    
    //declaration of game values
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

    //Initialization of the cards
    shuffle(tempArray, tempArray + totalCards, g);
    shuffledCards.clear();

    for (int i = 0; i < totalCards; ++i)
    {
        shuffledCards.push_back(tempArray[i]);
    }

    //Clear board
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

    //push the game state to the board cleared
    gameState.push(board);
    flipped.clear();
    matched.clear();
}

//Displays the board
void displayBoard()
{
    cout << "     ";
    for (int j = 0; j < currentBoardSize; ++j)
    {
        cout << j + 1 << " ";
    }
    cout << "\n";

    string colSeparator(currentBoardSize * 2 + 1, '_');
    cout << "    " << colSeparator << "\n";

    //Border stuff
    for (int i = 0; i < currentBoardSize; ++i)
    {
        cout << i + 1 << "   |";
        for (int j = 0; j < currentBoardSize; ++j)
        {
            if (flipped.count(make_pair(i, j)) || matched.count(make_pair(i, j)))
            {
                cout << board[make_pair(i, j)] << " ";
            }
            else
            {
                cout << "- ";
            }
        }
        cout << "|\n";
    }

    //Setting the bottom border
    string bottomBorder(currentBoardSize * 2 + 1, '-');
    cout << "    " << bottomBorder << "\n";
}

//Checks if there is a match between two cards
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
        //returns the board
    }
}

//undos the move, auto move on, do not require additional input
void undoMove()
{
    if (gameState.size() > 1)
    {
        gameState.pop();
        board = gameState.top();
        flipped.clear();
        if (!moveHistory.empty())
        {
            moveHistory.pop();
        }
    }
}

//Checks if there is a valid move to be made
bool isValidMove(int row, int col, bool checkPrevious)
{
    bool validBounds = row >= 0 && row < currentBoardSize && col >= 0 && col < currentBoardSize && !flipped.count(make_pair(row, col)) && !matched.count(make_pair(row, col));

    if (!validBounds)
    {
        return false;
    }

    //checking prev and move history
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

//Playst the turn
bool playTurn(int& turns)
{
    int row1, col1, row2, col2;
    bool validTurn = false;

    while (!validTurn)
    {
        displayWithBorder("Enter first card (row col 0-" + to_string(currentBoardSize - 1) + "): ");

        //Cin the move
        if (!(cin >> row1 >> col1))
        {
            displayWithBorder("Invalid input! Please enter two numbers.");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard();
            continue;
        }
        else
        {
            //move check here
            if (!isValidMove(row1 - 1, col1 - 1, true))
            {
                displayWithBorder("Invalid move! Try again.");
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                displayBoard();
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
            displayBoard();
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
                displayBoard();
                return false;
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

    //Check if there is a match
    if (checkMatch())
    {
        auto it1 = flipped.begin();
        auto it2 = next(it1);
        matched.insert(*it1);
        matched.insert(*it2);
        flipped.clear();
        displayWithBorder("Match found!");
    }
    else
    {
        //If not then flip back only the prev move 
        //Keep the rest of the moves that were made before hand
        
        displayWithBorder("No match. Flipping back...");
        this_thread::sleep_for(chrono::seconds(1));
        flipped.clear();
        displayBoard();
    }

    gameState.push(board);
    turns++;
    
    return true;
}

//Returning the matched size
int countMatches()
{
    return matched.size();
}


//Shows the menu
void showMenu()
{
    cout << "=== Memory Match Game ===\n";
    cout << "1. Level 1 (2x2) - Smiley Face\n";
    cout << "2. Level 2 (4x4) - Tree House\n";
    cout << "3. Level 3 (6x6) - Beach\n";
    cout << "4. Level 4 (8x8) - Ocean\n";
    cout << "5. Level 5 (10x10) - Bright Star\n";
    cout << "6. Level 6 (12x12) - Christmas Tree\n";
    cout << "7. Level 7 (14x14) - Bridge\n";
    cout << "8. Level 8 (16x16) - Fish\n";
    cout << "9. Quit\n";
    cout << "========================\n";
    
    //Each of these are backgrounds, (cause every good card games need a background)

    displayWithBorder("Select a level: ");
}

//Runs the level given (level and background)
void runGameLevel(int level, const string& background)
{
    //init
    initializeGame(level);
    cout << background << "\n";
    
    //Display and reset vals
    displayBoard();
    int turns = 0;

    //Loop if not complete
    while (countMatches() < currentBoardSize * currentBoardSize)
    {
        bool turnCompleted = playTurn(turns);
    }

    cout << "Congratulations! You won Level " << level << " in " << turns << " turns!\n";

    flipped.clear();
    matched.clear();
    gameState = stack<map<pair<int, int>, int>>();
    moveHistory = queue<pair<int, int>>();
}

//Plays level 1
void playLevel1()
{
    const string background = 
        "Level 1: Smiley Face\n"
        "----------\n"
        "\n"
        "         \n"
        "   +  +   \n"
        " \\      / \n"
        "  ^----^  \n"
        "\n"
        "        \n"
        "        \n"
        "-----------\n";
    
    runGameLevel(1, background);
}

//Plays level 2
void playLevel2()
{
    const string background = 
        "Level 2: Tree House\n"
        "      /\\    \n"
        "     /  \\   \n"
        "    /    \\  \n"
        "   /______\\ \n"
        "   |  *** | \n"
        "   |  *** | \n"
        "   |  *** | \n"
        "   |  *** | \n"
        "   |______| \n"
        "      ||    \n"
        "-------------\n";
    
    runGameLevel(2, background);
}

//plays level 3
void playLevel3()
{
    const string background = 
        "Level 3: Beach\n"
        "  ~~~~~~~~~~ \n"
        " ~     ~~~  ~\n"
        "  ~ ~~~    ~ \n"
        "   ~      ~  \n"
        "    ~  ~~~   \n"
        "     ~  ~    \n"
        " ----------- \n"
        "   *         \n"
        "          *  \n"
        "-------------\n";
    
    runGameLevel(3, background);
}

//plays level 4
void playLevel4()
{
    const string background = 
        "Level 4: Ocean\n"
        " ~~~~~~~~~~~~ \n"
        "~    ~~~     ~\n"
        " ~          ~ \n"
        "  ~~  ~~~   ~~ \n"
        "   ~~~   ~~~  \n"
        " ~~   ~~~~~   \n"
        "   ~~~~~~~    \n"
        "  ~~~~~~~     \n"
        " ~~ ~~~  ~~   \n"
        "~~~~~~~~~~~~  \n";
    
    runGameLevel(4, background);
}

///plays level 5
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

//plays levl 6
void playLevel6()
{
    const string background = 
        "Level 6: Christmas Tree\n"
        "     /\\     \n"
        "    /  \\    \n"
        "   /____\\   \n"
        "   /    \\   \n"
        "  /______\\  \n"
        "  /  /\\  \\  \n"
        " /  /  \\  \\ \n"
        "/__/____\\__\\\n"
        " |  ***  |  \n"
        " |_______|  \n"
        "-------------\n";
    
    runGameLevel(6, background);
}


//plays levl 7
void playLevel7()
{
    const string background = 
        "Level 7: Bridge\n"
        "\n"
        "\n"
        "\n"
        "\n"
        " /|\\     /|\\ \n"
        "/ | \\   / | \\ \n"
        "  |  \\ /  |   \n"
        "============ \n"
        "  |       |  \n"
        "  |       |  \n"
        "-------------\n";
    
    runGameLevel(7, background);
}

//plays levle 8
void playLevel8()
{
    const string background = 
        "Level 8: Fish\n"
        "     |      \n"
        " ~~~~|~~~~~~\n"
        "     |      \n"
        "     |      \n"
        "     |      \n"
        "  <(()=<    \n"
        "            \n"
        "            \n"
        "            \n"
        "           \n"
        "-------------\n";
    
    runGameLevel(8, background);
}


//main function for project running
int main()
{
    //cin choice
    int choice;

    while (true)
    {
        //Utilizing menu for user interface
        showMenu();
        cin >> choice;

        if (choice == 9)
        {
            break;
        }

        //input validation for the menu
        if (choice < 1 || choice > 8)
        {
            displayWithBorder("Invalid choice!");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        else
        {
            //picking which level to play
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
                    
                case 6:
                    playLevel6();
                    break;
                    
                case 7:
                    playLevel7();
                    break;
                    
                case 8:
                    playLevel8();
                    break;
            }
        }
    }

    //Goodbye statement
    cout << "Thanks for playing!\n";
    
    //terminate program
    return 0;
}