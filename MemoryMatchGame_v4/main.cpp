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
#include <vector>
#include <unordered_map>
using namespace std;

//constants and global variables
const int MAX_LEVELS = 8;//maximum number of levels
int currentBoardSize = 4;//current board size, adjusted per level
map<pair<int, int>, int> board;//stores card values at (row, col)
set<pair<int, int>> flipped;//tracks currently flipped cards
set<pair<int, int>> matched;//tracks permanently matched cards
list<int> cardPool;//temporary storage for card values
queue<pair<int, int>> moveHistory;//move sequence for validation
map<int, int> levelScores;//best scores (fewest turns) per level
int hintsRemaining = 0;//hints remaining for current level
int totalMoves = 0;//total moves (card selections) in a level
unordered_map<int, vector<pair<int, int>>> cardPositions;//hash table for card positions

//binary search tree node for scores
struct ScoreNode
{
    int level;//level number
    int turns;//turns taken
    ScoreNode* left;//left child
    ScoreNode* right;//right child
    ScoreNode(int l, int t) : level(l), turns(t), left(nullptr), right(nullptr) {}
};

//graph node for card relationships
struct GraphNode
{
    pair<int, int> position;//card position
    vector<pair<int, int>> adjacent;//adjacent positions
};

//class to manage bst for scores
class ScoreBST
{
private:
    ScoreNode* root;//root of the bst

    //insert a node recursively
    void insert(ScoreNode*& node, int level, int turns)
    {
        if (!node)
        {
            node = new ScoreNode(level, turns);
            return;
        }
        if (level < node->level)
        {
            insert(node->left, level, turns);
        }
        else if (level > node->level)
        {
            insert(node->right, level, turns);
        }
        else if (turns < node->turns)
        {
            node->turns = turns;//update if fewer turns
        }
    }

    //in-order traversal to collect scores
    void inOrder(ScoreNode* node, vector<pair<int, int>>& scores)
    {
        if (!node) return;
        inOrder(node->left, scores);
        scores.push_back({node->level, node->turns});
        inOrder(node->right, scores);
    }

    //clear the bst
    void clear(ScoreNode* node)
    {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

public:
    ScoreBST() : root(nullptr) {}
    ~ScoreBST()
    {
        clear(root);
    }

    //public method to insert score
    void insertScore(int level, int turns)
    {
        insert(root, level, turns);
    }

    //public method to get scores
    vector<pair<int, int>> getScores()
    {
        vector<pair<int, int>> scores;
        inOrder(root, scores);
        return scores;
    }
};

ScoreBST scoreTree;//bst instance for scores

//function declarations
void displayWithBorder(const string& text);
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
pair<int, int> findMatchRecursive(int value, const set<pair<int, int>>& visited, int row, int col);
void mergeSort(vector<int>& arr, int left, int right);
void merge(vector<int>& arr, int left, int mid, int right);
vector<pair<int, int>> getAdjacentCards(pair<int, int> pos);

//display text with a bordered format
void displayWithBorder(const string& text)
{
    int width = text.length() + 4;//calculate border width
    string topBottomBorder(width, '-');
    cout << topBottomBorder << "\n";
    cout << "| " << text << " |\n";
    cout << topBottomBorder << "\n";
}

//merge function for merge sort
void merge(vector<int>& arr, int left, int mid, int right)
{
    vector<int> leftArr(arr.begin() + left, arr.begin() + mid + 1);
    vector<int> rightArr(arr.begin() + mid + 1, arr.begin() + right + 1);
    int i = 0;
    int j = 0;
    int k = left;
    while (i < leftArr.size() && j < rightArr.size())
    {
        if (leftArr[i] <= rightArr[j])
        {
            arr[k] = leftArr[i];
            i++;
            k++;
        }
        else
        {
            arr[k] = rightArr[j];
            j++;
            k++;
        }
    }
    while (i < leftArr.size())
    {
        arr[k] = leftArr[i];
        i++;
        k++;
    }
    while (j < rightArr.size())
    {
        arr[k] = rightArr[j];
        j++;
        k++;
    }
}

//recursive merge sort
void mergeSort(vector<int>& arr, int left, int right)
{
    if (left < right)
    {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

//initialize game board
void initializeGame(int level)
{
    currentBoardSize = 2 * level;//set board size
    int totalCards = currentBoardSize * currentBoardSize;
    int pairs = totalCards / 2;
    vector<int> cardValues;
    hintsRemaining = level;//set hints per level
    cardPositions.clear();//clear hash table
    for (int i = 1; i <= pairs; i++)
    {
        cardValues.push_back(i);
        cardValues.push_back(i);
    }
    mergeSort(cardValues, 0, cardValues.size() - 1);//sort using merge sort
    random_device rd;
    mt19937 g(rd());
    shuffle(cardValues.begin(), cardValues.end(), g);//shuffle cards
    board.clear();
    int index = 0;
    for (int i = 0; i < currentBoardSize; i++)
    {
        for (int j = 0; j < currentBoardSize; j++)
        {
            board[make_pair(i, j)] = cardValues[index];
            cardPositions[cardValues[index]].push_back({i, j});
            index++;
        }
    }
    flipped.clear();
    matched.clear();
    moveHistory = queue<pair<int, int>>();
}

//display the game board
void displayBoard()
{
    cout << "     ";
    for (int j = 0; j < currentBoardSize; j++)
    {
        cout << j + 1 << " ";
    }
    cout << "\n";
    string colSeparator(currentBoardSize * 2 + 1, '_');
    cout << "    " << colSeparator << "\n";
    for (int i = 0; i < currentBoardSize; i++)
    {
        cout << i + 1 << "   |";
        for (int j = 0; j < currentBoardSize; j++)
        {
            if (flipped.count({i, j}) || matched.count({i, j}))
            {
                cout << board[{i, j}] << " ";
            }
            else
            {
                cout << "- ";
            }
        }
        cout << "|\n";
    }
    string bottomBorder(currentBoardSize * 2 + 1, '-');
    cout << "    " << bottomBorder << "\n";
}

//check if flipped cards match
bool checkMatch()
{
    if (flipped.size() != 2) return false;
    auto it1 = flipped.begin();
    auto it2 = next(it1);
    return board[*it1] == board[*it2];
}

//validate a move
bool isValidMove(int row, int col, bool checkPrevious)
{
    bool validBounds = row >= 0 && row < currentBoardSize &&
                       col >= 0 && col < currentBoardSize &&
                       !flipped.count({row, col}) &&
                       !matched.count({row, col});
    if (!validBounds) return false;
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

//get adjacent cards for hints
vector<pair<int, int>> getAdjacentCards(pair<int, int> pos)
{
    vector<pair<int, int>> adj;
    int row = pos.first;
    int col = pos.second;
    vector<pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (const auto& dir : directions)
    {
        int newRow = row + dir.first;
        int newCol = col + dir.second;
        if (isValidMove(newRow, newCol, false))
        {
            adj.push_back({newRow, newCol});
        }
    }
    return adj;
}

//recursive function to find a matching card
pair<int, int> findMatchRecursive(int value, const set<pair<int, int>>& visited, int row, int col)
{
    if (visited.count({row, col}) || !isValidMove(row, col, false))
    {
        return {-1, -1};
    }
    if (board[{row, col}] == value && !matched.count({row, col}))
    {
        return {row, col};
    }
    set<pair<int, int>> newVisited = visited;
    newVisited.insert({row, col});
    auto adj = getAdjacentCards({row, col});
    for (const auto& nextPos : adj)
    {
        auto result = findMatchRecursive(value, newVisited, nextPos.first, nextPos.second);
        if (result.first != -1) return result;
    }
    return {-1, -1};
}

//process a single turn
bool playTurn(int& turns)
{
    int row1, col1, row2, col2;
    bool validTurn = false;
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
        if (row1 == -9 && col1 == -9)
        {
            displayWithBorder("Quitting to menu...");
            return false;
        }
        if (row1 == -1 && col1 == -1)
        {
            getHint();
            displayBoard();
            continue;
        }
        if (!isValidMove(row1 - 1, col1 - 1, true))
        {
            displayWithBorder("Invalid move! Try again.");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard();
            continue;
        }
        flipped.insert({row1 - 1, col1 - 1});
        moveHistory.push({row1 - 1, col1 - 1});
        totalMoves++;
        displayBoard();
        validTurn = true;
    }
    validTurn = false;
    while (!validTurn)
    {
        displayWithBorder("Enter second card (row col 0-" + to_string(currentBoardSize - 1) +
                         ") or -1 -1 for hint (" + to_string(hintsRemaining) +
                         " left), -9 -9 to quit: ");
        if (!(cin >> row2 >> col2))
        {
            displayWithBorder("Invalid input! Please enter two numbers.");
            flipped.erase({row1 - 1, col1 - 1});
            moveHistory.pop();
            totalMoves--;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard();
            continue;
        }
        if (row2 == -9 && col2 == -9)
        {
            displayWithBorder("Quitting to menu...");
            flipped.erase({row1 - 1, col1 - 1});
            moveHistory.pop();
            totalMoves--;
            return false;
        }
        if (row2 == -1 && col2 == -1)
        {
            getHint();
            displayBoard();
            continue;
        }
        if (!isValidMove(row2 - 1, col2 - 1, true))
        {
            displayWithBorder("Invalid move! Try again.");
            flipped.erase({row1 - 1, col1 - 1});
            moveHistory.pop();
            totalMoves--;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            displayBoard();
            return true;
        }
        flipped.insert({row2 - 1, col2 - 1});
        moveHistory.push({row2 - 1, col2 - 1});
        totalMoves++;
        displayBoard();
        validTurn = true;
    }
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
    turns++;
    return true;
}

//count matched cards
int countMatches()
{
    return matched.size();
}

//update score in bst
void updateScore(int level, int turns)
{
    scoreTree.insertScore(level, turns);
    auto scores = scoreTree.getScores();
    for (const auto& score : scores)
    {
        if (score.first == level)
        {
            cout << "Best score for Level " << level << ": " << score.second << " turns\n";
            break;
        }
    }
}

//provide a hint
void getHint()
{
    if (hintsRemaining <= 0)
    {
        displayWithBorder("No hints remaining!");
        return;
    }
    pair<int, int> suggestion = {-1, -1};
    for (int i = 0; i < currentBoardSize; i++)
    {
        for (int j = 0; j < currentBoardSize; j++)
        {
            pair<int, int> pos = {i, j};
            if (!flipped.count(pos) && !matched.count(pos))
            {
                int value = board[pos];
                set<pair<int, int>> visited;
                suggestion = findMatchRecursive(value, visited, i, j);
                if (suggestion.first != -1 && suggestion != pos) break;
            }
        }
        if (suggestion.first != -1) break;
    }
    if (suggestion.first != -1)
    {
        hintsRemaining--;
        displayWithBorder("Hint: Try card at row " + to_string(suggestion.first + 1) +
                         ", col " + to_string(suggestion.second + 1) +
                         " (" + to_string(hintsRemaining) + " hints left)");
    }
    else
    {
        displayWithBorder("No valid moves available");
    }
}

//display game statistics
void displayStats(int turns)
{
    cout << "Game Statistics:\n";
    cout << "Total Turns: " << turns << "\n";
    cout << "Total Moves: " << totalMoves << "\n";
    cout << "Total Matches: " << matched.size() / 2 << "\n";
    cout << "Hints Used: " << (hintsRemaining - hintsRemaining) << "\n";
    cout << "Matched Positions:\n";
    for_each(matched.begin(), matched.end(), [](const pair<int, int>& pos)
    {
        cout << "(Row " << pos.first + 1 << ", Col " << pos.second + 1 << ")\n";
    });
    cout << "----------------\n";
}

//show the main menu
void showMenu()
{
    cout << "==== Memory Match Game ====\n";
    auto scores = scoreTree.getScores();
    for (int i = 1; i <= MAX_LEVELS; i++)
    {
        cout << i << ". Level " << i << " (" << 2*i << "x" << 2*i << ")";
        for (const auto& score : scores)
        {
            if (score.first == i)
            {
                cout << " (Best: " << score.second << " turns)";
                break;
            }
        }
        cout << "\n";
    }
    cout << "9. Quit\n";
    cout << "========================\n";
    displayWithBorder("Select a level: ");
}

//run a game level
void runGameLevel(int level, const string& background)
{
    initializeGame(level);
    totalMoves = 0;
    cout << background << "\n";
    displayBoard();
    int turns = 0;
    while (countMatches() < currentBoardSize * currentBoardSize)
    {
        bool continueGame = playTurn(turns);
        if (!continueGame)
        {
            flipped.clear();
            matched.clear();
            moveHistory = queue<pair<int, int>>();
            hintsRemaining = 0;
            totalMoves = 0;
            return;
        }
    }
    cout << "Congratulations! You won Level " << level << " in " << turns << " turns\n";
    updateScore(level, turns);
    displayStats(turns);
    flipped.clear();
    matched.clear();
    moveHistory = queue<pair<int, int>>();
    hintsRemaining = 0;
    totalMoves = 0;
}

//level 1: smiley face
void playLevel1()
{
    const string background =
        "Level 1: Smiley Face\n"
        "----------\n"
        "         \n"
        "   +  +   \n"
        " |      | \n"
        "  ^----^  \n"
        "        \n"
        "        \n"
        "------------\n";
    runGameLevel(1, background);
}

//level 2: tree house
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

//level 3: beach
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

//level 4: ocean
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

//level 5: bright star
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

//level 6: christmas tree
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

//level 7: bridge
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

//level 8: fish
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

//main function
int main()
{
    int choice;
    while (true)
    {
        showMenu();
        cin >> choice;
        if (choice == 9)
        {
            break;
        }
        if (choice < 1 || choice > 8)
        {
            displayWithBorder("Invalid choice!");
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }
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
    cout << "Thanks for playing!\n";
    return 0;
}