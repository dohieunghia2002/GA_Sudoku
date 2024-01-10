#include <iostream>
#include <fstream>
#include <vector>
#include <set>
#include <cstdlib>
#include <algorithm>
#include <random>
#define ROWS 9
#define COLS 9
#define AREA_SQUARE_SIZE 3
#define NUM_INDIVIDUALS 1000
#define NUM_BLOCKS 9
#define NUM_CELLS_INSIDE_BLOCK 9
#define showBoard showBoard3x3
using namespace std;

struct Coord {
    int x, y;
};

struct Cell {
    Coord position;
    int valueAt;
    bool fixed;
};

void initCell(Cell &cell) {
    cell.position = {0, 0};
    cell.valueAt = 0;
    cell.fixed = false;
}

struct Block {
    Cell cells[NUM_CELLS_INSIDE_BLOCK];
};

void initBlock(Block &B) {
    for (int i = 0; i < NUM_CELLS_INSIDE_BLOCK; i++) {
        initCell(B.cells[i]);
    }
}

struct Sudoku {
    int board[ROWS][COLS];
    int scoreFit;
    Block blocks[NUM_BLOCKS];
};

void initSudoku(Sudoku &sudoku) {
    for (int i = 0; i < NUM_BLOCKS; i++) {
        initBlock(sudoku.blocks[i]);
    }
    ifstream f;
    f.open("sudoku_hard_3x3.txt");
    int num;
    while (!f.eof()) {
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                f >> num;
                sudoku.board[i][j] = num;
            }
        }
    }
    f.close();
    int idxCell = 0, orderBlock = 0;
    for (int x = 0; x < COLS; x += AREA_SQUARE_SIZE)
    {
        for (int y = 0; y < COLS; y += AREA_SQUARE_SIZE)
        {
            for (int r = x; r < (x + AREA_SQUARE_SIZE); r++) {
                for (int c = y; c < (y + AREA_SQUARE_SIZE); c++) {
                    Coord pos = {r, c};
                    sudoku.blocks[orderBlock].cells[idxCell].valueAt = sudoku.board[r][c];
                    sudoku.blocks[orderBlock].cells[idxCell].position = pos;
                    idxCell++;
                }
            }
            idxCell = 0;
            orderBlock++;
        }
    }
    for (int b = 0; b < NUM_BLOCKS; b++) {
        for (int c = 0; c < NUM_CELLS_INSIDE_BLOCK; c++) {
            if (sudoku.blocks[b].cells[c].valueAt != 0) {
                sudoku.blocks[b].cells[c].fixed = true;
            }
        }
    }
    sudoku.scoreFit = 0;
}

void showBoard3x3(Sudoku sudoku) {
    for (int i = 0; i < ROWS; i++) {
        if (i % AREA_SQUARE_SIZE == 0)
            cout << "-------------------------\n";
        for (int j = 0; j < COLS; j++) {
            if (j % AREA_SQUARE_SIZE == 0) {
                cout << "| ";
            }
            cout << sudoku.board[i][j] << " ";
        }
        cout << "|\n";
    }
    cout << "-------------------------\n";
}

void showBoard4x4(Sudoku sudoku) {
    for (int i = 0; i < ROWS; i++) {
        if (i % AREA_SQUARE_SIZE == 0)
            cout << " -----------------------------------------------------------------\n";
        for (int j = 0; j < COLS; j++) {
            if (j % AREA_SQUARE_SIZE == 0) {
                cout << " |  ";
            }
            if (sudoku.board[i][j] < 10) {
                cout << "0";
            }
            cout << sudoku.board[i][j] << " ";
        }
        cout << " |\n";
    }
    cout << " -----------------------------------------------------------------\n";
}

bool isSolved(Sudoku &sudoku) {
    return (sudoku.scoreFit == 0);
}

Sudoku fillRandom(Sudoku sudoku) {
    Sudoku S;
    initSudoku(S);
    for (int b = 0; b < NUM_BLOCKS; b++) {
        vector<int> clean;
        for (int e = 1; e <= NUM_CELLS_INSIDE_BLOCK; e++) {
            clean.push_back(e);
        }
        for (int c = 0; c < NUM_CELLS_INSIDE_BLOCK; c++) {
            if (sudoku.blocks[b].cells[c].fixed == true) {
                clean[(sudoku.blocks[b].cells[c].valueAt) - 1] = 0;
            }
        }
        vector<int> domainValues;
        for (int e = 0; e < clean.size(); e++) {
            if (clean[e] != 0)
                domainValues.push_back(clean[e]);
        }
        for (int c = 0; c < NUM_CELLS_INSIDE_BLOCK; c++) {
            if (sudoku.blocks[b].cells[c].fixed == false) {
                int indexVal;
                if (domainValues.size() > 1)
                    indexVal = rand() % domainValues.size();
                else
                    indexVal = 0;

                S.blocks[b].cells[c].valueAt = domainValues[indexVal];
                Coord pos = sudoku.blocks[b].cells[c].position;
                S.board[pos.x][pos.y] = domainValues[indexVal];

                swap(domainValues[indexVal], domainValues[domainValues.size() - 1]);
                domainValues.pop_back();
            }
        }
    }
    return S;
}

void initPopulation(Sudoku sudoku, vector<Sudoku> &population) {
    for (int i = 0; i < NUM_INDIVIDUALS; i++) {
        Sudoku P = fillRandom(sudoku);
        population.push_back(P);
    }
}

void scoreFitness(Sudoku &sudoku) {
    int fit = 0;
    for (int row = 0; row < ROWS; row++) {
        set<int> cleanRow;
        set<int> cleanCol;
        for (int col = 0; col < COLS; col++) {
            cleanRow.insert(sudoku.board[row][col]);
            cleanCol.insert(sudoku.board[col][row]);
        }
        fit = fit + (ROWS - cleanRow.size());
        fit = fit + (COLS - cleanCol.size());
    }
    sudoku.scoreFit = fit;
}

bool compareIndividuals(Sudoku &S1, Sudoku &S2) {
    return (S1.scoreFit < S2.scoreFit);
}

void rankPopulation(vector<Sudoku> &population) {
    sort(population.begin(), population.end(), compareIndividuals);
}

vector<Sudoku> pickFromPopulation(vector<Sudoku> population, float selectRate, float selectRateRand) {
    rankPopulation(population);
    vector<Sudoku> nextBreeders;
    if (isSolved(population[0]) == true) {
        nextBreeders.push_back(population[0]);
        return nextBreeders;
    }

    int nbBestToSelect = population.size() * selectRate;
    int nbRandToSelect = population.size() * selectRateRand;
    for (int i = 0; i < nbBestToSelect; i++) { // select individuals best 
        nextBreeders.push_back(population[i]);
    }
    vector<Sudoku> nextBreedersTemp;
    for (int p = nbBestToSelect; p < population.size(); p++) // copy the rest of individuals after select into  nextBreedersTemp
    {
        nextBreedersTemp.push_back(population[p]);
    }
    for (int j = 0; j < nbRandToSelect; j++) // random select the rest of individuals push to nextBreeders
    {

        int random = rand() % nextBreedersTemp.size();
        nextBreeders.push_back(nextBreedersTemp[random]);
        swap(nextBreedersTemp[random], nextBreedersTemp[nextBreedersTemp.size() - 1]);
        nextBreedersTemp.pop_back();
    }
    random_device rd;
    shuffle(nextBreeders.begin(), nextBreeders.end(), rd);
    return nextBreeders;
}

void fillWithBlock(Sudoku &sudoku, Block block, int idxBlock) {
    for (int c = 0; c < NUM_CELLS_INSIDE_BLOCK; c++)
    {
        sudoku.blocks[idxBlock].cells[c].valueAt = block.cells[c].valueAt;
        Coord pos = sudoku.blocks[idxBlock].cells[c].position;
        sudoku.board[pos.x][pos.y] = block.cells[c].valueAt;
    }
}

void createOneChild(vector<int> &mark, Sudoku father, Sudoku mother, Sudoku &child) {
    int idxCrossover = rand() % mark.size();
    int crossoverPoint = mark[idxCrossover];
    for (int i = 0; i < NUM_BLOCKS; i++) {
        if (i < crossoverPoint) {
            fillWithBlock(child, father.blocks[i], i);
        }
        else {
            fillWithBlock(child, mother.blocks[i], i);
        }
    }
    swap(mark[idxCrossover], mark[mark.size() - 1]);
    mark.pop_back();
}

vector<Sudoku> createChildren(vector<Sudoku> nextBreeders, int nbChild, Sudoku &child) {
    vector<Sudoku> nextPopulation;
    vector<int> mark;
    for (int i = 0; i < (nextBreeders.size() / 2); i++) {
        mark.resize(0);
        for (int cr = 1; cr <= NUM_BLOCKS - 2; cr++) {
            mark.push_back(cr);
        }
        for (int j = 0; j < nbChild; j++) {
            createOneChild(mark, nextBreeders[i], nextBreeders[nextBreeders.size() - 1 - i], child);
            nextPopulation.push_back(child);
        }
    }
    return nextPopulation;
}

void mutateWithBlock(Sudoku &sudoku, int idxBlock) {
    vector<int> clean;
    for (int e = 1; e <= NUM_CELLS_INSIDE_BLOCK; e++) {
        clean.push_back(e);
    }
    for (int c = 0; c < NUM_CELLS_INSIDE_BLOCK; c++) {
        if (sudoku.blocks[idxBlock].cells[c].fixed == true) {
            clean[(sudoku.blocks[idxBlock].cells[c].valueAt) - 1] = 0;
        }
    }
    vector<int> domainValues;
    for (int e = 0; e < clean.size(); e++) {
        if (clean[e] != 0) {
            domainValues.push_back(clean[e]);
        }
    }
    for (int c = 0; c < NUM_CELLS_INSIDE_BLOCK; c++) {
        if (sudoku.blocks[idxBlock].cells[c].fixed == false)
        {
            int indexVal;
            if (domainValues.size() > 1) {
                indexVal = rand() % domainValues.size();
            }
            else {
                indexVal = 0;
            }
            sudoku.blocks[idxBlock].cells[c].valueAt = domainValues[indexVal];
            Coord pos = sudoku.blocks[idxBlock].cells[c].position;
            sudoku.board[pos.x][pos.y] = domainValues[indexVal];

            swap(domainValues[indexVal], domainValues[domainValues.size() - 1]);
            domainValues.pop_back();
        }
    }
}

void mutateOneIndividual(Sudoku &sudoku) {
    int idxBlock, idxRow, count = 0;
    idxRow = idxBlock = rand() % ROWS;
    vector<int> clean;
    for (int e = 1; e <= ROWS; e++) {
        clean.push_back(e);
    }
    for (int r = 0; r < ROWS; r++) {
        int valuesInsideRow = sudoku.board[idxRow][r];
        clean[valuesInsideRow - 1] = 0;
    }
    for (int e = 0; e < clean.size(); e++) {
        if (clean[e] != 0)
            count++;
    }
    if (count > 0) {
        mutateWithBlock(sudoku, idxBlock);
    }
}

void mutatePopulation(vector<Sudoku> &population, float mutationRate) {
    random_device rd;
    shuffle(population.begin(), population.end(), rd);

    int nbMutation = population.size() * mutationRate;
    for (int cnt = 0; cnt < nbMutation; cnt++) {
        mutateOneIndividual(population[cnt]);
    }
}

bool sameBoard(Sudoku A, Sudoku B) {
    for (int x = 0; x < ROWS; x++) {
        for (int y = 0; y < COLS; y++)
            if (A.board[x][y] != B.board[x][y])
                return false;
    }
    return true;
}

int main() {
    Sudoku start, child;
    initSudoku(start);
    showBoard(start);
    initSudoku(child);
    vector<Sudoku> population;
    initPopulation(start, population); // initialization

    for (int i = 0; i < population.size(); i++) { // calculating fitness
        scoreFitness(population[i]);
    }
    bool flag = false;
    int count = 0;
    while (flag == false) {
        count++;
        // cout << count << endl;
        vector<Sudoku> nextBreeders = pickFromPopulation(population, 0.25f, 0.25f); // selection
        if (isSolved(nextBreeders[0]) == true) {
            cout << "Problem solved after" << count << "generations!!! Solution found is:\n"
                 << count;
            flag = true;
            showBoard(nextBreeders[0]);
            break;
        }
        else {
            flag = false;
        }

        vector<Sudoku> newGeneration = createChildren(nextBreeders, 4, child); // crossover
        for (int i = 0; i < newGeneration.size(); i++) {
            for (int j = 0; j < newGeneration.size(); j++) {
                if (i != j && sameBoard(newGeneration[i], newGeneration[j]) == true) {
                    newGeneration[j] = fillRandom(start);
                }
            }
        }

        mutatePopulation(newGeneration, 0.2f); // mutation
        for (int child = 0; child < newGeneration.size(); child++) {
            scoreFitness(newGeneration[child]);
            if (isSolved(newGeneration[child]) == true) {
                cout << "Problem solved after " << count << " generations!!! Solution found is:\n";
                flag = true;
                showBoard(newGeneration[child]);
                break;
            }
            else {
                flag = false;
            }
        }

        population.clear();
        for (int p = 0; p < newGeneration.size(); p++) { // new generation 
            population.push_back(newGeneration[p]);
        }
    }
    return 0;
}