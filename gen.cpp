#include <iostream>
#include <stdlib.h>
#include <termios.h>
#include <stdio.h>
#include <time.h>
#include <set>

static struct termios old, neu;

/* Initialize new terminal i/o settings */
void initTermios(int echo)
{
    tcgetattr(0, &old); /* grab old terminal i/o settings */
    neu = old; /* make new settings same as old settings */
    neu.c_lflag &= ~ICANON; /* disable buffered i/o */
    neu.c_lflag &= echo ? ECHO : ~ECHO; /* set echo mode */
    tcsetattr(0, TCSANOW, &neu); /* use these new terminal i/o settings now */
}

/* Restore old terminal i/o settings */
void resetTermios(void)
{
    tcsetattr(0, TCSANOW, &old);
}

/* Read 1 character - echo defines echo mode */
char getch_(int echo)
{
    char ch;
    initTermios(echo);
    ch = getchar();
    resetTermios();
    return ch;
}

/* Read 1 character without echo */
char getch(void)
{
    return getch_(0);
}

using namespace std;

const int X = 40;
const int Y = 20;
char a[X][Y];

bool route(int x, int y, int goal_x, int goal_y, set<pair<int, int> > &s)
{
    if (x == goal_x && y == goal_y)
        return true;
    if (a[x][y] == '#')
        return false;
    if (s.insert(make_pair(x, y)).second)
    {
        if (route(x + 1, y, goal_x, goal_y, s)) return true;
        if (route(x - 1, y, goal_x, goal_y, s)) return true;
        if (route(x, y + 1, goal_x, goal_y, s)) return true;
        if (route(x, y - 1, goal_x, goal_y, s)) return true;
    }
    return false;
}

int main()
{
    for (int i = 0; i < X; i++)
    {
        for (int j = 0; j < Y; j++)
        {
            if (i == 0 || j == 0 || i == X - 1 || j == Y - 1)
                a[i][j] = '#';
            else
                a[i][j] = ' ';
        }
    }
    char c;
    int x = 1;
    int y = 1;
    int goal_x;
    int goal_y;
    srand(time(NULL));
    do
    {
        goal_x = rand() % (X - 2) + 1;
        goal_y = rand() % (Y - 2) + 1;
    }
    while (goal_x == x && goal_y == y);

    int hardness = 500;
    while (hardness > 0)
    {
        int cand_x = rand() % (X - 2) + 1;
        int cand_y = rand() % (Y - 2) + 1;
        if (cand_x == x && cand_y == y)
            continue;
        if (cand_x == goal_x && cand_y == goal_y)
            continue;
        if (a[cand_x][cand_y] == '#')
            continue;
        hardness--;
        a[cand_x][cand_y] = '#';
        set<pair<int, int> > s;
        if (!route(x, y, goal_x, goal_y, s))
            a[cand_x][cand_y] = ' ';
    }

    while (true)
    {
        system("clear");
        for (int j = 0; j < Y; j++)
        {
            for (int i = 0; i < X; i++)
            {
                if (i == x && j == y)
                    cout << "\033[1;37m@\033[0m";
                else if (i == goal_x && j == goal_y)
                    cout << "\033[1;34mX\033[0m";
                else
                    cout << a[i][j];
            }
            cout << "\n";
        }
        c = getch();
        switch (c)
        {
        case 65:
            if (a[x][y - 1] == ' ') y--;
            break;
        case 66:
            if (a[x][y + 1] == ' ') y++;
            break;
        case 67:
            if (a[x + 1][y] == ' ') x++;
            break;
        case 68:
            if (a[x - 1][y] == ' ') x--;
            break;
        }
        if (x == goal_x && y == goal_y)
        {
            break;
        }
    }
}
