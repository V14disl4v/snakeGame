#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>  
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <array>
#include <windows.h>
#include <cstdio>

#include <ctime>    
#include <conio.h>

const int WIN_SCORE = 250;

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;  // размер не важен, когда visible=false
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

using namespace std;
#define sLen 100

void moveXY(int x, int y) {
    COORD pos;
    pos.X = x;
    pos.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

#define mWidth 80
#define mHeight 25

//--------- КАРТА --------//
struct Map {
    const int WALL_HEIGHT = 6;
    const int HORIZONTAL_WALL_LENGTH = 6;
    POINT apples[8];
    int appleCount{};
    POINT cakes[4];
    int cakeCount{};
    POINT walles[4];
    int wallCount{};
    POINT wallesG[3];
    int wallCountG{};
    POINT euros[4];
    int euroCount{};
    POINT rub[5];
    int rubCount{};
    char map1[mHeight][mWidth + 1];
    void clear();
    void show();
    void InitApples();          // спавн 4-х на старте
    void SpawnApple();          // одно новое рандомное
    void DrawApples();          // нарисовать все яблока на map1
    void RemoveAppleAt(int px, int py);  // удалить съеденное

    void InitCakes();
    void SpawnCakes();
    void DrawCakes();
    void RemoveCakesAt(int px, int py);

    void InitWall();
    void SpawnWall();
    void DrawWall();

    void InitEuro();
    void SpawnEuro();
    void DrawEuro();
    void RemoveEuroAt(int px, int py);

    bool euroEnabled = false;
    void initWallG();
    void SpawnWallG();
    void DrawWallG();
    
    void initRub();
    void SpawnRub();
    void DrawRub();
    void RemoveRubAt(int px, int py);
};



struct Pos {
    uint8_t x;  // 0–255, хватит для 80
    uint8_t y;  // 0–255, хватит для 25
};

enum Direction { dirleft, dirright, dirup, dirdown };
enum Result { reOK, reKILL, reWIN, re100 };

class Snake {
    int x, y;
    int* s;
    Map* mp;         // указатель на карту
    vector<Pos> tail;
    //POINT tail[sLen];
    Direction dir;   // текущее направление
    float score = 0.0f;
    POINT OutTextPos;
public:
    void AddTail(int _x, int _y);
    void MoveTail(int _x, int _y);
    Snake(Map* _mp, int outX, int outY);
    void Init(int _x, int _y, Direction _dir);
    void PutOnMap();
    Result UserControl(char w, char s, char a, char d);
    void AddScore(float points);
    void ResetScore();
    void DevScore();

    float GetScore() const {
        return round(score * 100.0f) / 100.0f;
    }
};

void Snake::AddScore(float points) {
    score += points;
}

void Snake::ResetScore() {
    score = 0.0f;
}

void Snake::DevScore() {
    score = score / 2.0f;
}


void Snake::MoveTail(int _x, int _y)
{
    if (!tail.empty()) {
        tail.pop_back();
    }

    tail.insert(tail.begin(), Pos{ static_cast<uint8_t>(_x), static_cast<uint8_t>(_y) });
}

void Snake::AddTail(int _x, int _y)
{
    tail.insert(tail.begin(), Pos{ static_cast<uint8_t>(_x), static_cast<uint8_t>(_y) });
}


Result Snake::UserControl(char w, char s, char a, char d) {

    POINT old;
    old.x = x;
    old.y = y;
    Direction newDir = dir;
    if (GetAsyncKeyState(w) & 0x8001) newDir = dirup;
    if (GetAsyncKeyState(s) & 0x8001) newDir = dirdown;
    if (GetAsyncKeyState(a) & 0x8001) newDir = dirleft;
    if (GetAsyncKeyState(d) & 0x8001) newDir = dirright;
    if (!(
        (dir == dirup && newDir == dirdown) ||
        (dir == dirdown && newDir == dirup) ||
        (dir == dirleft && newDir == dirright) ||
        (dir == dirright && newDir == dirleft)
        )) {
        dir = newDir;
    }

    if (dir == dirdown) y++;
    else if (dir == dirup) y--;
    else if (dir == dirright) x++;
    else if (dir == dirleft) x--;

    if ((x < 1 || (x >= mWidth - 1)) || (y < 1 || y >= mHeight - 1) || (mp->map1[y][x] == 'i')
        || (mp->map1[y][x] == 'l') || (mp->map1[y][x] == 'o') || (mp->map1[y][x] == 'v') || (mp->map1[y][x] == 'e')
        || (mp->map1[y][x] == 'A' || (mp->map1[y][x] == 'N') || (mp->map1[y][x] == 'A') || (mp->map1[y][x] == 'L')
            || (mp->map1[y][x] == '<') || (mp->map1[y][x] == '|') || (mp->map1[y][x] == '-'))) {
        return reKILL;
    }

    /*char cell2 = mp->map1[y][x];
    if (score >= 100 && !mp->cakesEnabled) {
        mp->cakesEnabled = true;
        mp->InitEuro();
        mp->DrawEuro();
        if (cell2 == '€') {
            mp->RemoveEuroAt(x, y);
            mp->map1[y][x] = ' ';
            for (int i = 0; i < 7; i++) AddTail(old.x, old.y);
            score += 50.0f;
            mp->SpawnEuro();
        }
        if (score >= WIN_SCORE) return reWIN;
    }*/

    char cell = mp->map1[y][x];
    if (cell == '*' || cell == '$' || cell == '€' || cell == '₽') {
        if (cell == '*') mp->RemoveAppleAt(x, y);
        else if (cell == '$')  mp->RemoveCakesAt(x, y);
        else if (cell == '€') mp->RemoveEuroAt(x, y);
        else if (cell == '₽') mp->RemoveRubAt(x, y);

        mp->map1[y][x] = ' ';
        for (int i = 0; i < 7; i++) AddTail(old.x, old.y);
        if (cell == '*') score += 10.0f;
        else if (cell == '$')  score += 20.0f;
        else if (cell == '€') score += 30.0f;
        else if (cell == '₽') score -= 30.0f;
        if (score >= 100.0f && !mp->euroEnabled) {
            mp->euroEnabled = true;
            mp->InitEuro(); 
        }

        if (cell == '*') mp->SpawnApple();
        else if (cell == '$') mp->SpawnCakes();
        else if (cell == '€') mp->SpawnEuro();
        else if (cell == '₽') mp->SpawnRub();
        if (score >= WIN_SCORE) return reWIN;
    }
    MoveTail(old.x, old.y);


    return reOK;
}

void Snake::Init(int _x, int _y, Direction _dir) {
    x = _x;
    y = _y;
    dir = _dir;
    tail.clear();
}

void Snake::PutOnMap()
{
    char str[16];
    sprintf(str, "%.1f", score);

    for (int i = 0; i < strlen(str); i++) {
        mp->map1[OutTextPos.y + 1][OutTextPos.x + i + 1] = str[i];
    }

    mp->map1[y][x] = '<';

    static const string bodyText = "iloveANAL";
    size_t textLen = bodyText.size();

    for (size_t i = 0; i < tail.size(); ++i) {
        char symbol = bodyText[i % textLen];  // циклическое повторение
        mp->map1[tail[i].y][tail[i].x] = symbol;
    }
}

Snake::Snake(Map* _mp, int outX, int outY)
{
    mp = _mp;
    Init(0, 0, dirright);
    score = 0.0f;
    OutTextPos.x = outX;
    OutTextPos.y = outY;
    tail.clear();
}

// -------------------

void Map::InitWall() {
    wallCount = 0;
    for (int i = 0; i < 3; i++) {
        SpawnWall();
    }
}

void Map::SpawnWall() {
    int rx = rand() % (mWidth - 6) + 3;
    int max_top_y = mHeight - WALL_HEIGHT - 1; // x: 1 .. mWidth-2 (не на краях)
    int ry = rand() % (max_top_y - 1 + 1) + 1;    // y: 2 .. mHeight-1 (чтобы y-2 >=0!)
    walles[wallCount] = { rx, ry };
    wallCount++;
}

void Map::DrawWall() {
    for (int i = 0; i < wallCount; i++) {
        int x = walles[i].x;
        int y = walles[i].y;
        for (int h = 0; h < WALL_HEIGHT; h++) {
            int cy = y + h;
            if (cy >= 1 && cy < mHeight - 1) {
                map1[cy][x] = '|';
            }
        }
    }
}

//---------

void Map::initWallG() {
    wallCountG = 0;
    for (int i = 0; i < 3; i++) {
        SpawnWallG();
    }
}

void Map::SpawnWallG() {
    int min_y = 2;
    int max_y = mHeight - 3;
    int ry = rand() % (max_y - min_y + 1) + min_y;

    int min_x = 2;
    int max_start_x = mWidth - 12;  // фиксированная длина 10 блоков

    if (max_start_x < min_x) return;  // поле слишком узкое

    int rx = rand() % (max_start_x - min_x + 1) + min_x;

    wallesG[wallCountG] = { rx, ry };
    wallCountG++;
}

void Map::DrawWallG() {
    for (int i = 0; i < wallCountG; i++) {
        int start_x = wallesG[i].x;
        int y = wallesG[i].y;

        
        for (int k = 0; k < 10; k++) {
            int cx = start_x + k;
            if (cx >= 1 && cx < mWidth - 1) {
                map1[y][cx] = '-'; 
            }
        }
    }
}

// ------ИНИЦИАЛИЗАЦИЯ ЯБЛОЧЕК И ПИРОЖЕННЫХ

void Map::InitApples() {
    appleCount = 0;
    for (int i = 0; i < 8; i++) {
        SpawnApple();
    }
}

void Map::SpawnApple() {
    int rx = rand() % (mWidth - 2) + 1;
    int ry = rand() % (mHeight - 2) + 1;
    apples[appleCount] = { rx, ry };
    appleCount++;
}

void Map::DrawApples() {
    for (int i = 0; i < appleCount; i++) {
        map1[apples[i].y][apples[i].x] = '*';
    }
}

void Map::RemoveAppleAt(int px, int py) {
    for (auto it = 0; it < appleCount; ++it) {
        if (apples[it].x == px && apples[it].y == py) {
            for (int j = it; j < appleCount - 1; j++) {
                apples[j] = apples[j + 1];
            }
            appleCount--;
            return;
        }
    }
}

void Map::InitCakes() {
    cakeCount = 0;
    for (int i = 0; i < 4; i++) {
        SpawnCakes();
    }
}

void Map::SpawnCakes() {
    int rx = rand() % (mWidth - 2) + 1;
    int ry = rand() % (mHeight - 2) + 1;
    cakes[cakeCount] = { rx, ry };
    cakeCount++;
}

void Map::DrawCakes() {
    for (size_t i = 0; i < cakeCount; i++) {
        map1[cakes[i].y][cakes[i].x] = '$';
    }
}

void Map::RemoveCakesAt(int px, int py) {
    for (auto it = 0; it < cakeCount; ++it) {
        if (cakes[it].x == px && cakes[it].y == py) {
            for (int j = it; j < cakeCount - 1; j++) {
                cakes[j] = cakes[j + 1];
            }
            cakeCount--;
            return;
        }
    }
}

void Map::InitEuro() {
    euroCount = 0;
    for (int i = 0; i < 4; i++) {
        SpawnEuro();
    }
}

void Map::SpawnEuro() {
    int rx = rand() % (mWidth - 2) + 1;
    int ry = rand() % (mHeight - 2) + 1;
    euros[euroCount] = { rx, ry };
    euroCount++;
}

void Map::DrawEuro() {
    for (int i = 0; i < euroCount; i++) {
        map1[euros[i].y][euros[i].x] = '€';
    }
}

void Map::RemoveEuroAt(int px, int py) {
    for (auto it = 0; it < euroCount; ++it) {
        if (euros[it].x == px && euros[it].y == py) {
            for (int j = it; j < euroCount - 1; j++) {
                euros[j] = euros[j + 1];
            }
            euroCount--;
            return;
        }
    }
}

void Map::initRub() {
    rubCount = 0;
    for (int i = 0; i < 5; i++) {
        SpawnRub();
    }
}

void Map::SpawnRub() {
    int rx = rand() % (mWidth - 2) + 1;
    int ry = rand() % (mHeight - 2) + 1;
    rub[rubCount] = { rx, ry };
    rubCount++;
}

void Map::DrawRub() {
    for (int i = 0; i < rubCount; i++) {
        map1[rub[i].y][rub[i].x] = '₽';
    }
}

void Map::RemoveRubAt(int px, int py) {
    for (auto it = 0; it < rubCount; ++it) {
        if (rub[it].x == px && rub[it].y == py) {
            for (int j = it; j < rubCount - 1; j++) {
                rub[j] = rub[j + 1];
            }
            rubCount--;
            return;
        }
    }
}

// -------------------

void Map::show() {
    moveXY(0, 0);
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    for (size_t j = 0; j < mHeight; j++) {
        cout << map1[j] << '\n';
    }
    cout.flush();
}

void Map::clear() {
    //пробелы
    for (size_t y = 0; y < mHeight; y++) {
        memset(map1[y], ' ', mWidth);
        map1[y][mWidth] = '\0';
    }

    // верх и низ
    for (int x = 0; x < mWidth; x++) {
        map1[0][x] = '%';
        map1[mHeight - 1][x] = '%';
    }

    // лево и право 
    for (int y = 0; y < mHeight; y++) {
        map1[y][0] = '%';
        map1[y][mWidth - 1] = '%';
    }
}

int main() {
    setlocale(LC_ALL, "RU");
    SetConsoleOutputCP(CP_UTF8);
    hideCursor();
    srand(static_cast<unsigned>(time(nullptr)));

    int cntPlayers{};
    cout << "Выберите режиме игры: \n 1 - solo 2 - PvP \n";
    cin >> cntPlayers;

    Map map1;
    map1.InitApples();
    map1.InitCakes();
    map1.InitWall();
    map1.initWallG();
    //map1.InitEuro();
    map1.initRub();

    Snake snake1(&map1, 3, 1);
    snake1.Init(10, 5, dirright);
    snake1.AddTail(3, 3);

    if (cntPlayers == 2) {

        Snake snake2(&map1, 73, 1);
        snake2.Init(70, 15, dirleft);
        snake2.AddTail(70, 16);

        do
        {
            map1.clear();
            map1.DrawApples();
            map1.DrawCakes();
            map1.DrawWall();
            map1.DrawWallG();
            map1.DrawRub();
            if (map1.euroEnabled) map1.DrawEuro();  // ← только если разблокированы!
            map1.DrawWall();
            //map1.DrawEuro();
            snake1.PutOnMap();
            snake2.PutOnMap();

            Result res = snake1.UserControl('W', 'S', 'A', 'D');
            //Result res2 = snake2.UserControl(VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT);
            Result res2 = snake2.UserControl('I', 'K', 'J', 'L');

            map1.show();
            Sleep(40); // изменение скорости змейки

            /*if (res == re100) {
                map1.InitEuro();
                map1.DrawEuro();
            }*/
            //Result res2 = snake2.UserControl('I', 'K', 'J', 'L');
            if (res == reKILL) {
                //map1.euroEnabled = false;
                snake1.Init(10, 5, dirright);
                snake1.AddTail(3, 3);
                snake1.DevScore();
                map1.InitApples();
                map1.InitWall();
                map1.initWallG();
                //map1.InitApples();       // спавн яблок при смерти
                //map1.InitCakes();
            }
            if (res2 == reKILL) {
               // map1.euroEnabled = false;
                snake2.Init(70, 15, dirleft);
                snake2.AddTail(70, 16);
                snake2.DevScore();  //деление счета при смерти
                map1.InitWall();
                map1.initWallG();
                //map1.InitApples();       // спавн яблок при смерти
                //map1.InitCakes();
            }

            else if (res == reWIN || res2 == reWIN) {
                while (true) {
                    if (res == reWIN || res2 == reWIN) {
                        map1.clear();
                        moveXY(20, 10);
                        cout << "WINNER: " << (res == reWIN ? "PLAYER 1 (слева)!" : "PLAYER 2 (справа)!") << endl;
                        moveXY(20, 12);
                        cout << "Get Scores: " << (res == reWIN ? snake1.GetScore() : snake2.GetScore()) << endl;
                        moveXY(20, 14);
                        cout << "Press ENTER for restert" << endl;
                        _getch();
                    }
                    if (GetKeyState(VK_RETURN) < 0) {   // ENTER
                        map1.clear();
                        snake1.ResetScore(); //сброс очков
                        snake1.Init(10, 5, dirright);// перезапуск
                        snake1.AddTail(3, 3);
                        snake2.ResetScore(); //сброс очков
                        snake2.Init(70, 15, dirleft);
                        snake2.AddTail(70, 16);
                        break;
                    }
                    if (GetKeyState(VK_ESCAPE) < 0) {
                        return 0;   // выход из программы
                    }
                    Sleep(10);
                }
            }
        } while (GetKeyState(VK_ESCAPE) >= 0);
    }

    else {
        do
        {   
            map1.clear();
            map1.DrawApples();
            map1.DrawCakes();
            map1.DrawWall();
            snake1.PutOnMap();
           

            Result res = snake1.UserControl('W', 'S', 'A', 'D');

            map1.show();
            Sleep(7); // изменение скорости змейки

            if (res == reKILL) {
                snake1.Init(10, 5, dirright);
                snake1.AddTail(3, 3);
                snake1.DevScore();
                //map1.InitApples();
                map1.InitWall();
                map1.initWallG();
                //map1.InitApples();       // спавн яблок при смерти
                //map1.InitCakes();
            }

            else if (res == reWIN) {
                while (true) {
                    moveXY(20, 10);
                    cout << "You are Winner!" << endl;
                    moveXY(20, 12);
                    cout << "Get Scores: " << snake1.GetScore() << endl;
                    moveXY(20, 14);
                    cout << "Press ENTER for restert" << endl;
                    _getch();
                }

                if (GetKeyState(VK_RETURN) < 0) {   // ENTER
                    snake1.ResetScore(); //сброс очков
                    snake1.Init(10, 5, dirright);// перезапуск
                    snake1.AddTail(3, 3);
                    break;
                }
                if (GetKeyState(VK_ESCAPE) < 0) {
                    return 0;   // выход из программы
                }
                Sleep(10);
            }
        } while (GetKeyState(VK_ESCAPE) >= 0);
    }
    return 0;
}
