#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <climits>

struct Point {
    int x, y;
    Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};

class OthelloBoard {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int flip_times = 0;
    int cur_player;
    bool done;
    int winner;
private:
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            flip_times++;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
                flip_times++;
            }
        }
    }
public:
    OthelloBoard() {
        reset();
    }
    OthelloBoard(const OthelloBoard& game) {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = game.board[i][j];
            }
        }
        cur_player = game.cur_player;
        disc_count[EMPTY] = game.disc_count[EMPTY];
        disc_count[BLACK] = game.disc_count[BLACK];
        disc_count[WHITE] = game.disc_count[WHITE];
        next_valid_spots = game.next_valid_spots;
        done = game.done;
        winner = game.winner;

    }
    OthelloBoard(std::array<std::array<int, SIZE>, SIZE> bd) {
        disc_count[BLACK] = 0;
        disc_count[WHITE] = 0;
        disc_count[EMPTY] = 0;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = bd[i][j];
                if(bd[i][j] == EMPTY) disc_count[EMPTY]++;
                else if(bd[i][j] == BLACK) disc_count[BLACK]++;
                else if(bd[i][j] == WHITE) disc_count[WHITE]++;
            }
        }
        //cur_player = BLACK;
        //next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return flip_times;
    }
};


int player;
const int SIZE = 8;
std::array<std::array<int, SIZE>, SIZE> board;
std::vector<Point> next_valid_spots;

void read_board(std::ifstream& fin) {
    fin >> player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> board[i][j];
        }
    }
}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        next_valid_spots.push_back(Point(x, y));
    }
}
int disc_place(OthelloBoard game, int mode) {
    int bonus = 0;

    // center
    for(int i = 3; i <= 4; i++) {
        for(int j = 3; j <= 4; j++) {
            if(game.board[i][j] == 1) bonus += 2;
        }
    }
    for(int i = 2; i <= 5; i++) {
        for(int j = 2; j <= 5; j++) {
            if(game.board[i][j] == 1) bonus ++;
        }
    }

    // corner
    for(int i: {0, 7}) {
        for(int j: {0, 7}) {
            if(game.board[i][j] == 1) bonus += 5;
        }
    }

    // side
    for(int i: {0, 7}) {
        for(int j = 0; j <= 7; j++) {
            if(game.board[i][j] == 1) bonus += 2;
        }
    }
    for(int j: {0, 7}) {
        for(int i = 0; i <= 7; i++) {
            if(game.board[i][j] == 1) bonus += 2;
        }
    }

    // danger zone
    if(mode == 1) {
        for(int i = 0; i <= 7; i++) {
            if(game.board[i][1] == 1) bonus --;
            if(game.board[i][6] == 1) bonus --;
        }
        for(int j = 0;  j <= 7; j++) {
            if(game.board[1][j] == 1) bonus --;
            if(game.board[6][j] == 1) bonus --;
        }
    }

    return bonus;
}
int evaluate(OthelloBoard game) {
    int bonus = 0;
    bonus = disc_place(game, 1);
    //flip disc as fewer as possible  & have more place to put disc
    if(game.cur_player == game.BLACK)
        bonus += game.next_valid_spots.size()*2 - game.flip_times*5;
    else
        bonus += game.flip_times*5 - game.next_valid_spots.size()*2;

    return bonus;
}
int eval(OthelloBoard game) {
    int bonus = 0;
    bonus = disc_place(game, 2);
    //flip disc as fewer as possible  & have more place to put disc
    if(game.cur_player == game.BLACK)
        bonus += game.flip_times*5;
    else
        bonus -= game.flip_times*5;
    bonus += (game.disc_count[1] - game.disc_count[2]);

    return bonus;
}

int state_value(OthelloBoard game, int depth, int alpha, int beta, int player) {
    if(depth == 0)
        return evaluate(game);
    if(game.done)
        return eval(game);

    if(player == OthelloBoard::BLACK) {
        int value = INT_MIN;
        for(Point p: game.next_valid_spots){
            OthelloBoard game2 = game;
            game2.put_disc(p);
            value = std::max(value, state_value(game2, depth-1, alpha, beta, game2.cur_player));
            alpha = std::max(value, alpha);
            if(alpha >= beta)
                break;
        }
        return value;
    }
    else {
        int value = INT_MAX;
        for(Point p: game.next_valid_spots){
            OthelloBoard game2 = game;
            game2.put_disc(p);
            value = std::min(value, state_value(game2, depth-1, alpha, beta, game2.cur_player));
            beta = std::min(value, beta);
            if(beta <= alpha)
                break;
        }
        return value;
    }
}

void write_valid_spot(std::ofstream& fout) {
    int value = INT_MIN;
    for(Point p: next_valid_spots) {
        OthelloBoard game2(board);    //用讀進來的board初始化一個Othello物件
        game2.cur_player = player;
        game2.put_disc(p);
        int val = state_value(game2, 6, INT_MIN, INT_MAX, game2.cur_player);
        if(val > value) {
            value = val;
            fout << p.x << " " << p.y << std::endl;
        }
    }
    fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}

