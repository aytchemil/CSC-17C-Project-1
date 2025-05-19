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
#include <iterator>  

const int MAX_LEVELS = 8; // max levels
int currentBoardSize = 4; // Current board size, adj per level
map<pair<int, int>, int> board; // Stores card values at (row, col) pos
set<pair<int, int>> flipped; // Tracks currently flipped cards
set<pair<int, int>> matched; // Tracks permanently matched cards
list<int> cardPool; // Temporary storage for card vals
queue<pair<int, int>> moveHistory; // Tracks move sequence for validation
map<int, int> levelScores; // Tracks best scores (fewest turns) per level
int hintsRemaining = 0; // Tracks remaining hints for the current level
int totalMoves = 0; // Tracks total moves (card selections) in a level

// Function declarations
void displayBackground(int level);
void initializeGame(int level);
void displayBoard();
bool checkMatch();
bool isValidMove(int row, int col, bool checkPrevious = false);
bool playTurn(int& turns);
int countMatches();
void showMenu();
void updateScore(int level, int turns);
void getHint();
void displayStats(int turns);
void playLevel1();
void playLevel2();
void playLevel3();
void playLevel4();
void playLevel5();
void playLevel6();
void playLevel7();
void playLevel8();

// Displays text with a bordered format for user prompts
// Parameters: text - the message to display
// Uses string manipulation to create dynamic borders
void displayWithBorder(const string& text)
{
    
    int width = text.length() + 4; //  border width
    string topBottomBorder(width, '-'); //  border string

    
    cout << topBottomBorder << "\n";
    cout << "| " << text << " |\n";
    cout << topBottomBorder << "\n";
}

// init the game board for a given level
// level - the level number (1 to 8)
// use STL list for card values, map for board, and shuffle algorithm
void initializeGame(int level)
{
    
    // Set board size based on level (2x2 for level 1, 4x4 for level 2 etc)
    currentBoardSize = 2 * level;
    int totalCards = currentBoardSize * currentBoardSize; // Total number of cards
    int pairs = totalCards / 2; // Number of pairs needed
    list<int> cardValues; // List to store card values

    
    // Reset hints for the new level (1 hint for Level 1, 2 for Level 2 etc)
    hintsRemaining = level;

    
    // Populate card values with pairs
    for (int i = 1; i <= pairs; ++i)
    {
        cardValues.push_back(i);
        cardValues.push_back(i);
    }
    

    //random num generator for shuffling
    random_device rd;
    mt19937 g(rd());
    list<int> shuffledCards = cardValues; // Copy card values
    int tempArray[totalCards]; // Temporary array for shuffling

    
    // use std::copy to transfer list elements to array
    std::copy(shuffledCards.begin(), shuffledCards.end(), tempArray);

    
    // shuff the array using STL algorithm
    shuffle(tempArray, tempArray + totalCards, g);
    shuffledCards.clear(); // Clear the list
    

    // repop with shuffled values
    for (int i = 0; i < totalCards; ++i)
    {
        shuffledCards.push_back(tempArray[i]);
    }

    // clear the board and assign shuffled cards
    board.clear();
    for (int i = 0; i < currentBoardSize; ++i)
    {
        for (int j = 0; j < currentBoardSize; ++j)
        {
            board[make_pair(i, j)] = shuffledCards.front();
            shuffledCards.pop_front(); // Remove assigned card
        }
    }
    

    // clear other containers
    flipped.clear();
    matched.clear();
    moveHistory = queue<pair<int, int>>(); // Reset move history
}

// displ the current state of the game board
// use map to access card values and set to check flipped/matched states
void displayBoard()
{
    // Print col nums
    cout << "     ";
    for (int j = 0; j < currentBoardSize; ++j)
    {
        cout << j + 1 << " ";
    }
    cout << "\n";

    // Print top border
    string colSeparator(currentBoardSize * 2 + 1, '_');
    cout << "    " << colSeparator << "\n";

    // Print each row with card states
    for (int i = 0; i < currentBoardSize; ++i)
    {
        cout << i + 1 << "   |";
        for (int j = 0; j < currentBoardSize; ++j)
        {
            if (flipped.count(make_pair(i, j)) || matched.count(make_pair(i, j)))
            {
                cout << board[make_pair(i, j)] << " "; // Show card value
            }
            else
            {
                cout << "- "; // Show face-down card
            }
        }
        cout << "|\n";
    }

    // print bottom border
    string bottomBorder(currentBoardSize * 2 + 1, '-');
    cout << "    " << bottomBorder << "\n";
}

// checks if the two flipped cards match
// use set iterators to access flipped card positions
bool checkMatch()
{
    // Ensure exactly two cards are flipped
    if (flipped.size() != 2)
    {
        return false;
    }
    else
    {
        // Use iterators to access flipped card positions
        auto it1 = flipped.begin();
        auto it2 = next(it1);
        return board[*it1] == board[*it2]; // Compare card values
    }
}

// Validates a move based on bounds and game rules
// Parameters row, col - position to check; checkPrevious - prevent re-selecting last move
bool isValidMove(int row, int col, bool checkPrevious)
{
    // Check if the move is within bounds and not already flipped/matched
    bool validBounds = row >= 0 && row < currentBoardSize && col >= 0 &&
                      col < currentBoardSize && !flipped.count(make_pair(row, col)) &&
                      !matched.count(make_pair(row, col));

    if (!validBounds)
    {
        return false;
    }

    // Prevent selecting the same card as the last move
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

// Processes a single turn, including input and matching
// Parameters: turns - reference to turn counter
// Returns: true to continue game, false if player quits
// Supports hints (-1 -1) and quit (-9 -9), but undo is disabled
bool playTurn(int& turns)
{
    
    int row1, col1, row2, col2;
    bool validTurn = false;

    // Handle first card selection
    while (!validTurn)
    {
        displayWithBorder("Enter first card (row col 0-" + to_string(currentBoardSize - 1) +
                         ") or -1 -1 for hint (" + to_string(hintsRemaining) +
                         " left), -9 -9 to quit: ");

        if (!(cin >> row1 >> col1))
        {
            displayWithBorder("Invalid input! Please enter two numbers.");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard();
            continue;
        }

        // Check for special commands
        if (row1 == -9 && col1 == -9)
        {
            displayWithBorder("Quitting to menu...");
            return false; // Signal to exit the level
        }
        if (row1 == -1 && col1 == -1)
        {
            getHint(); // Provide a hint
            displayBoard();
            continue;
        }

        // Validate the move
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
            totalMoves++; // Increment move counter
            displayBoard();
            validTurn = true;
        }
    }

    // Handle second card selection
    validTurn = false;
    while (!validTurn)
    {
        
        displayWithBorder("Enter second card (row col 0-" + to_string(currentBoardSize - 1) +
                         ") or -1 -1 for hint (" + to_string(hintsRemaining) +
                         " left), -9 -9 to quit: ");

        if (!(cin >> row2 >> col2))
        {
            displayWithBorder("Invalid input! Please enter two numbers.");
            flipped.erase(make_pair(row1 - 1, col1 - 1));
            moveHistory.pop();
            totalMoves--; // Revert move count
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard();
            continue;
        }

        // Check for special commands
        if (row2 == -9 && col2 == -9)
        {
            displayWithBorder("Quitting to menu...");
            flipped.erase(make_pair(row1 - 1, col1 - 1));
            moveHistory.pop();
            totalMoves--; // Revert move count
            return false; // Signal to exit the level
        }
        if (row2 == -1 && col2 == -1)
        {
            getHint();
            displayBoard();
            continue;
        }

        // Validate the move
        if (!isValidMove(row2 - 1, col2 - 1, true))
        {
            displayWithBorder("Invalid move! Try again.");
            flipped.erase(make_pair(row1 - 1, col1 - 1));
            moveHistory.pop();
            totalMoves--; // Revert move count
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard();
            return true; // Continue the game
        }
        else
        {
            flipped.insert(make_pair(row2 - 1, col2 - 1));
            moveHistory.push(make_pair(row2 - 1, col2 - 1));
            totalMoves++; // Increment move counter
            displayBoard();
            validTurn = true;
        }
    }

    // Process the turn result
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
        displayWithBorder("No match. Flipping back...");
        this_thread::sleep_for(chrono::seconds(1));
        flipped.clear();
        displayBoard();
    }

    turns++; // Increment turn count
    return true; // Continue the game
}

// Returns the number of matched cards
// Uses set size to count matches
int countMatches()
{
    return matched.size();
}

// Updates the best score for a level
// Parameters: level - level number; turns - turns taken
void updateScore(int level, int turns)
{
    // Update score if first attempt or fewer turns
    if (levelScores.find(level) == levelScores.end() || turns < levelScores[level])
    {
        levelScores[level] = turns;
    }
    // Display the current best score
    cout << "Best score for Level " << level << ": " << levelScores[level] << " turns\n";
}

// Provides a hint by suggesting a valid move
// Checks hint limit and decrements remaining hints
void getHint()
{
    
    // Check if hints are available
    if (hintsRemaining <= 0)
    {
        displayWithBorder("No hints remaining!");
        return;
    }

    // Find a valid, unbfliped, unmatched card
    pair<int, int> suggestion = {-1, -1};
    for (int i = 0; i < currentBoardSize; ++i)
    {
        
        for (int j = 0; j < currentBoardSize; ++j)
        {
            pair<int, int> pos = make_pair(i, j);
            if (!flipped.count(pos) && !matched.count(pos))
            {
                suggestion = pos;
                break;
            }
        }
        if (suggestion.first != -1) break;
    }

    // Display the suggestion
    if (suggestion.first != -1)
    {
        
        hintsRemaining--; // Decrement hint count
        displayWithBorder("Hint: Try card at row " + to_string(suggestion.first + 1) +
                         ", col " + to_string(suggestion.second + 1) +
                         " (" + to_string(hintsRemaining) + " hints left)");
    }
    else
    {
        displayWithBorder("No valid moves available!");
    }
}

// Displays game statistcs
// Uses std::for_each to iterate over matched positions
// Includes total moves and hints used
void displayStats(int turns)
{
    
    // Print basic stats
    cout << "Game Statistics:\n";
    cout << "Total Turns: " << turns << "\n";
    cout << "Total Moves: " << totalMoves << "\n";
    cout << "Total Matches: " << matched.size() / 2 << "\n";
    cout << "Hints Used: " << (hintsRemaining - hintsRemaining) << "\n";

    // Use std::for_each to print matched positions
    cout << "Matched Positions:\n";
    for_each(matched.begin(), matched.end(), [](const pair<int, int>& pos) {
        cout << "(Row " << pos.first + 1 << ", Col " << pos.second + 1 << ")\n";
    });
    cout << "----------------\n";
}

// Displas the game menu with level options and scores
void showMenu()
{
    
    cout << "=== Memory Match Game ===\n";
    cout << "1. Level 1 (2x2) - Smiley Face";
    if (levelScores[1]) cout << " (Best: " << levelScores[1] << " turns)";
    cout << "\n";
    cout << "2. Level 2 (4x4) - Tree House";
    if (levelScores[2]) cout << " (Best: " << levelScores[2] << " turns)";
    cout << "\n";
    cout << "3. Level 3 (6x6) - Beach";
    if (levelScores[3]) cout << " (Best: " << levelScores[3] << " turns)";
    cout << "\n";
    cout << "4. Level 4 (8x8) - Ocean";
    if (levelScores[4]) cout << " (Best: " << levelScores[4] << " turns)";
    cout << "\n";
    cout << "5. Level 5 (10x10) - Bright Star";
    if (levelScores[5]) cout << " (Best: " << levelScores[5] << " turns)";
    cout << "\n";
    cout << "6. Level 6 (12x12) - Christmas Tree";
    if (levelScores[6]) cout << " (Best: " << levelScores[6] << " turns)";
    cout << "\n";
    cout << "7. Level 7 (14x14) - Bridge";
    if (levelScores[7]) cout << " (Best: " << levelScores[7] << " turns)";
    cout << "\n";
    cout << "8. Level 8 (16x16) - Fish";
    if (levelScores[8]) cout << " (Best: " << levelScores[8] << " turns)";
    cout << "\n";
    cout << "9. Quit\n";
    cout << "========================\n";
    displayWithBorder("Select a level: ");
}

// Runs a game level with the specified background
// Parameters: lvl - lvl number; background - ASCII art
// Exits early if player quits via -9 -9
void runGameLevel(int level, const string& background)
{
    
    initializeGame(level); // Set up the board
    totalMoves = 0; // Reset move counter
    cout << background << "\n"; // Display level background
    displayBoard(); // Show initial board
    int turns = 0; // Track number of turns

    // Continue until all cards are matched or player quits
    while (countMatches() < currentBoardSize * currentBoardSize)
    {
        bool continueGame = playTurn(turns);
        if (!continueGame) // Player quit to menu
        {
            // Clear game state before returning
            flipped.clear();
            matched.clear();
            moveHistory = queue<pair<int, int>>();
            hintsRemaining = 0;
            totalMoves = 0;
            return;
        }
    }

    // Display win msg and update scores
    cout << "Congratulations! You won Level " << level << " in " << turns << " turns!\n";
    updateScore(level, turns); // Update best score
    displayStats(turns); // Show game statistics

    // Reset game 
    flipped.clear();
    matched.clear();
    moveHistory = queue<pair<int, int>>();
    hintsRemaining = 0;
    totalMoves = 0;
    
}

// Level 1: Smiley Face theme
void playLevel1()
{
    
    const string background =
        "Level 1: Smiley Face\n"
        "----------\n"
        "\n"
        "         \n"
        "   +  +   \n"
        " \      / \n"
        "  ^----^  \n"
        "\n"
        "        \n"
        "        \n"
        "-----------\n";
    
    runGameLevel(1, background);
    
}

// Level 2: Tree House theme
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

// Level 3: Beach theme
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

// Level 4: Ocean theme
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

// Level 5: Bright Star theme
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

// Level 6: Christmas Tree theme
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

// Level 7: Bridge theme
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

// Level 8: Fish theme
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

// Main function to run the game
int main()
{
    
    int choice; // Store user’s menu choice

    // Main game loop
    while (true)
    {
        
        showMenu(); // Display the menu
        cin >> choice; // Get user input

        // Exit condition
        if (choice == 9)
        {
            break;
        }

        // Validate menu input
        if (choice < 1 || choice > 8)
        {
            displayWithBorder("Invalid choice!");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
        else
        {
            
            // Select and play the chosen level
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

    
    // Farewell message
    cout << "Thanks for playing!\n";
    return 0; // Terminate 
}