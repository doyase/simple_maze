
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

//制限時間の設定(秒数)
int TIME_LIMIT;

//バッファをクリアする関数
void clear_buffer() {
    while (getchar() != '\n');  //改行が出るまで読み飛ばす
}

//迷路の壁と通路とプレイヤーの描画
void displayMaze(int **maze,int playerY,int playerX, int rows, int cols) {
       
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (y == playerY && x == playerX) {
                printf("〇");
            }
            else if (maze[y][x] == 1) {
                printf("■");//壁
            }
            else if (maze[y][x] == 0) {
                printf("　");//通路
            }
            else if (maze[y][x] == 2) {
                printf("ほ");//ゴール地点
            }
            else if (maze[y][x] == 3) {
                printf("☆");//ヒント
            }
        }
        printf("\n");
    }
}

// 迷路を生成する再帰的な関数
// 迷路の壁を取り除いて通路を作り、再帰的に迷路を広げていく
// y, x は現在の位置、rows, cols は迷路のサイズ
void generateMaze(int **maze, int y, int x, int rows, int cols) {

    // 進行方向を示す配列（上下左右）
    // それぞれの方向は[縦方向の移動量, 横方向の移動量]を表している
    int directions[4][2] = { {-2,0},{2,0},{0,-2},{0,2} };
    for (int i = 0; i < 4; i++) {
        int randIndex = rand() % 4;
        int temp[2] = { directions[i][0], directions[i][1] };
        directions[i][0] = directions[randIndex][0];
        directions[i][1] = directions[randIndex][1];
        directions[randIndex][0] = temp[0];
        directions[randIndex][1] = temp[1];
    }

    //4方向に対して再帰的に通路を生成
    for(int i = 0; i < 4; i++){

        //新しい位置を計算(y,xの方向に進んだ位置)
        int ny = y + directions[i][0];
        int nx = x + directions[i][1];

        //新しい位置が迷路内にあり、まだ通路が無い場所があれば
        if (ny > 0 && ny < rows - 1 && nx > 0 && nx < cols - 1 && maze[ny][nx] == 1) {
            
            //その位置を通路(0)に変更
            maze[ny][nx] = 0;

            //現在の位置を新しい位置の間の壁も取り除く(壁の真ん中を通路にする)
            maze[y + directions[i][0] / 2][x + directions[i][1] / 2] = 0;

            //再帰的にその位置から迷路を生成
            generateMaze(maze, ny, nx, rows, cols);        
        }        
    }
}
//方向指定入力受付
void getInput(char* move) {
    printf("MOVE: w[up],s[down],a[left],d[right],h[hint]\n");

    while (1) {     //入力がただしくなるまでループ
        printf("入力: ");
        if (scanf(" %c", move) != 1) {    //文字が正しく入力されたかチェック
            printf("入力エラーよ、もう一度入力してくれるかしら\n");
            clear_buffer();
            continue;
        }
        //受け付ける入力だけを許可
        if (*move == 'w' || *move == 's' || *move == 'a' || *move == 'd' || *move == 'h') {
            return;//正しい入力なら関数を終了
        }
        printf("無効な入力よ。w,s,a,d,hのいずれかを入力してね。\n");
        clear_buffer();
    }
}

//当たり判定とプレイヤーの移動
char movePlayer(int** maze, char move, int* playerY, int* playerX) {
    if (move == 'w' && maze[*playerY - 1][*playerX] != 1) {
        (*playerY)--;
        return 'w';
    }

    if (move == 's' && maze[*playerY + 1][*playerX] != 1) {
        (*playerY)++;
        return 's';
    }

    if (move == 'a' && maze[*playerY][*playerX - 1] != 1) {
        (*playerX)--;
        return 'a';
    }

    if (move == 'd' && maze[*playerY][*playerX + 1] != 1) {
        (*playerX)++;
        return 'd';
    }

    if (move == 'h') {
        return 'h';
    }
    return 'i';
}

//ゴール地点の生成
void generategoalPoint(int **maze, int *goalY, int *goalX, int rows, int cols) {
    
    while (1) {
        *goalY = rand() % (rows - 2) + 1;
        *goalX = rand() % (cols - 2) + 1;

        if (maze[*goalY][*goalX] == 0) {
            maze[*goalY][*goalX] = 2;
            break;
        }
    }    
}
// メモリ解放処理を関数化
void freeMemory(int** array, int rows) {
    for (int i = 0; i < rows; i++) {
        free(array[i]);
    }
    free(array);
}

//難易度選択により迷路サイズのメモリを確保する
void allocateMemory(int*** maze, int rows, int cols) {
    *maze = (int**)malloc(sizeof(int*) * rows);
    if (*maze == NULL) {
        printf("メモリの確保に失敗したわ\n");
        exit(1);//プログラムの強制終了
    }
   
    for (int i = 0; i < rows; i++) {
        (*maze)[i] = (int*)malloc(sizeof(int) * cols);
        if ((*maze)[i] == NULL) {
            printf("メモリの確保に失敗したわ\n");
            freeMemory(*maze, i);  //メモリの解放            
            exit(1);
        }
    }
}

// 難易度に応じて迷路のサイズを設定し、メモリを確保する関数
// 入力された難易度に基づいて、迷路の行数と列数を決定
// メモリの確保に成功したか確認し、失敗した場合は処理を中止する
void difficultyLevel(int*** maze, int* rows, int* cols) {

    int input;
    printf("迷路の難易度を設定するわね\n");
    printf("難しい：[1]　普通：[2]　易しい：[3]\n");

    //入力エラーを防ぐため、scanfでの読み取り結果を確認
    while (1) {
        printf("どれにする？: ");
        if (scanf("%d", &input) != 1) {
            printf("入力エラーよ、数字を一つ入力してくれるかしら\n");
            clear_buffer();
            continue;
        }

        switch (input) {
            //難易度1：大きな迷路
        case 1:
            *rows = 27;
            *cols = 51;
            allocateMemory(maze, *rows, *cols);
            return;


            //難易度2：中くらいの迷路
        case 2:
            *rows = 21;
            *cols = 41;
            allocateMemory(maze, *rows, *cols);
            return;

            //難易度3：小さい迷路
        case 3:
            *rows = 21;
            *cols = 21;
            allocateMemory(maze, *rows, *cols);
            return;
        default:
            printf("入力が正しくないわ･･･。\n");
            clear_buffer();
            continue;
        }
    }
}

void timelimitChoice(int* TIME_LIMIT) {

    int input2;
    printf("制限時間を設定するわ\n");
    printf("80秒：[1]120秒：[2]160秒：[3]\n");
    while (1) {   //入力が正しくなるまでループ
        printf("どうする？: ");
        if (scanf(" %d", &input2) != 1) {
            printf("入力エラーよ、数字を一つ選んでくれるかしら\n");
            clear_buffer();
            continue;
        }

        switch (input2) {
        case 1:
            *TIME_LIMIT = 80;
            return;

        case 2:
            *TIME_LIMIT = 120;
            return;

        case 3:
            *TIME_LIMIT = 160;
            return;

        default:
            printf("入力が正しくないわ･･･\n");
            continue;
        }
    }
}

// --- BFSで使う構造体とキュー
typedef struct {
    int y;
    int x;
}Position;

#define MAX_QUEUE 10000

Position queue[MAX_QUEUE];
int head = 0, tail = 0;

void enqueue(Position p) {
    if (tail < MAX_QUEUE) {
        queue[tail++] = p;
    }
    else {
        printf("キューが満杯よ\n");
        exit(1); // 強制終了
    }
}

Position dequeue() {
    return queue[head++];
}


// ----------------------------------------------------
// BFSで (sy, sx) -> (gy, gx) の経路を探索し、
// 見つかったら経路を "☆" (maze[][] = 3) で表示する
// そして5秒待って消す
// ----------------------------------------------------
void bfsFindPathAndShow(int** maze, int rows, int cols, int sy, int sx,
    int gy, int gx) {
    // 壁:1, 通路 : 0, ゴール : 2, ヒントor経路表示用 : 3
    // ただし BFS ではゴール(2)も「通行可能」とみなす

    // 訪問管理と前の座標を持つ配列を確保
    int** visited = (int**)malloc(rows * sizeof(int*));
    if (*visited == NULL) {
        printf("メモリの確保に失敗したわ¥n");
        return;
    }
    Position** prevPos = (Position**)malloc(rows * sizeof(Position*));
    if (prevPos == NULL) {
        printf("メモリの確保に失敗したわ\n");
        free(visited);
        return;
    }
    for (int i = 0; i < rows; i++) {
        visited[i] = (int*)calloc(cols, sizeof(int));
        if (visited[i] == NULL) {
            printf("メモリの確保に失敗したわ\n");
            free(visited);
            free(prevPos);
            return;
        }
        prevPos[i] = (Position*)malloc(cols * sizeof(Position));
        if (prevPos[i] == NULL) {
            printf("メモリの確保に失敗したわ\n");
            for (int j = 0; j < i; j++) {
                free(visited[j]);
                free(prevPos[j]);
            }
            free(visited);
            free(prevPos);
            return;
        }
        for (int j = 0; j < cols; j++) {
            prevPos[i][j].y = -1;
            prevPos[i][j].x = -1;
        }
    }


    //BFS初期化
    Position temp = { sy, sx };
    head = tail = 0;
    enqueue(temp);
    visited[sy][sx] = 1;

    int found = 0;
    while (head < tail) {
        Position cur = dequeue();
        if (cur.y == gy && cur.x == gx) {
            found = 1;
            break;
        }
        //上下左右
        int dy[4] = { -1, 1, 0,0 };
        int dx[4] = { 0,0,-1,1 };
        for (int i = 0; i < 4; i++) {
            int ny = cur.y + dy[i];
            int nx = cur.x + dx[i];
            if (ny >= 0 && ny < rows && nx >= 0 && nx < cols) {
                //壁(1)以外なら通過可能にする
                if (maze[ny][nx] != 1 && visited[ny][nx] == 0) {
                    visited[ny][nx] = 1;
                    prevPos[ny][nx].y = cur.y;
                    prevPos[ny][nx].x = cur.x;
                    Position temp2 = { ny,nx };
                    enqueue(temp2);
                }
            }
        }
    }
    //経路が見つかったら「☆」を表示して5秒後に消去
    if (found) {
        //ゴールからスタートしてprevPosを逆にたどる
        Position cur = { gy,gx };
        while (!(cur.y == sy && cur.x == sx)) {
            //ゴール(2)とスタート(0)をどう扱うかはお好み
            //ここでは通路のみを☆に変える
            if (maze[cur.y][cur.x] == 0) {
                maze[cur.y][cur.x] = 3;//☆
            }
            Position p = prevPos[cur.y][cur.x];
            cur = p;
        }

        //一度再描画して経路を表示
        system("cls");

        displayMaze(maze, sy, sx, rows, cols);

        printf("\n経路を☆で表示してみたわ。5秒後に消すわね\n");
        Sleep(5000);

        //星を通路に戻す
        cur.y = gy;
        cur.x = gx;
        while (!(cur.y == sy && cur.x == sx)) {
            if (maze[cur.y][cur.x] == 3) {
                maze[cur.y][cur.x] = 0;
            }
            Position p = prevPos[cur.y][cur.x];
            if (p.y == -1 && p.x == -1) {
                printf("経路復元中にエラーが発生したわ\n");
                break;
            }
            cur = p;
        }

        // ゴールが消えちゃった場合は再設定
        maze[gy][gx] = 2;


        //もう一度再描画
        system("cls");
        //printf("\033[2J\033[H"); fflush(stdout);// 画面をクリアしてカーソルを左上に移動
        displayMaze(maze, sy, sx, rows, cols);
        printf("\n経路を消したわ\n");
        Sleep(2000);
    }
    else {
        printf("経路が見つからなかったわ・・・\n");
        Sleep(2000);
    }

    //メモリ解放
    for (int i = 0; i < rows; i++) {
        free(visited[i]);
        free(prevPos[i]);
    }
    free(visited);
    free(prevPos);
}


int main()
{
    //開始時刻を記録
    clock_t starttime = clock();

    timelimitChoice(&TIME_LIMIT);

    int **maze = NULL;   //初期化
    
    int rows, cols;

    difficultyLevel(&maze,  &rows, &cols);
    if (maze == NULL) {
        printf("迷路のメモリ確保に失敗したから、プログラムを終了するわね\n");
        return 1;
    }

    //迷路全体を壁で埋める
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            maze[y][x] = 1;            
        }
    }
    
    srand(time(NULL));
    maze[1][1] = 0;  //スタート地点    

    //迷路の生成
    generateMaze(maze, 1, 1, rows, cols);

    //プレイヤーの座標
    int playerX = 1, playerY = 1;
        
    //ユーザーの移動方向指定を受け取る
    char move;

    //ゴール地点の生成
    int goalY, goalX;
    generategoalPoint(maze, &goalY, &goalX, rows, cols);
    
    while (1) {

        clock_t currenttime = clock();   //開始時刻を取得
        double elapsedtime = (double)(currenttime - starttime) / CLOCKS_PER_SEC;   //経過時間を計算        
        
        if (elapsedtime > TIME_LIMIT) {
            printf("時間切れよ･･･またちょうせんしてみてね。(ぎゅ･･･)\n");
            break;
        }
        system("cls");
        //printf("\033[2J\033[H"); fflush(stdout); // 画面をクリアしてカーソルを左上に移動

        //画面の描画
        displayMaze(maze, playerY, playerX, rows, cols);            
        
        //残り時間を表示
        printf("残り時間：%.1f秒\n", TIME_LIMIT - elapsedtime);

        //移動方向の受付
        getInput(&move);

        //プレイヤーの移動
        char result = movePlayer(maze, move, &playerY, &playerX);
        if (result == 'h') {
            bfsFindPathAndShow(maze, rows, cols,
                playerY, playerX, goalY, goalX);
            //戻ってきたらまたループ先頭へ
            continue;
        }
        //ゴール判定
        if (playerX == goalX && playerY == goalY) {
            printf("ゲームクリアよ！おめでとう！(ぎゅっ♪)\n");
            break;
        }
    }
    freeMemory(maze, rows);
    
    return 0;
}
