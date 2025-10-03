#include "solver.h"
#include "lib/helper.h"
#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>
#include <iomanip>

/*
 * Wakasagi will call this and only this function.
 * Below you will find examples of functions you might use
 * and an example for outputting random moves.
 *
 * The header files contain detailed comments.
 *
 * Guide & documentation can be found here:
 * https://docs.google.com/document/d/1AcbayzQvl0wyp6yl5XzhtDRPyOxINTpjR-1ghtxHQso/edit?usp=sharing
 *
 * Good luck!
 */

struct Node{
    Position position;
    int g_cost;// actual cost from start to current state
    int h_cost;// heuristic cost to reach the goal
    int f_cost;// g + h, can be omitted actually (?)

    int parent;// index of parent
    Move mv;// move from parent to current

    // initiate
    Node(const Position &pos, int g, int h, int p, Move move)
        : position(pos), g_cost(g), h_cost(h), f_cost(g+h), parent(p), mv(move){}
};

int heuristic(const Position& pos) {
    int sum = 0;
    Board red_board = pos.pieces(Red);// squares_sorted
    Board black_board = pos.pieces(Black);
    std::vector<int>used(SQUARE_NB, -1);
    for(Square red_sq: BoardView(red_board)){
        Piece target = pos.peek_piece_at(red_sq);
        
        int min_step = 1000;
        int best_attack = -1;
        for(Square black_sq: BoardView(black_board)){
            Piece attacker = pos.peek_piece_at(black_sq);
            if(attacker.type == Duck || !(attacker.type > target.type))
                continue;

            int cur_step = 1000;
            if(attacker.type != Chariot)
                cur_step = distance<Square>(black_sq, red_sq);
            else{
                // handle chariot
                if(distance<Square>(black_sq, red_sq) == distance<Rank>(black_sq, red_sq)
                || distance<Square>(black_sq, red_sq) == distance<File>(black_sq, red_sq))
                    cur_step = 1;
                else
                    cur_step = 2;
            }

            if(min_step > cur_step){
                min_step = cur_step;
                best_attack = black_sq;
            }
        }
        if(best_attack != -1){
            if(used[best_attack] != -1){
                if(used[best_attack] > min_step){// may be sequentially reached
                    min_step = 0;
                }
                else if(used[best_attack] == min_step){
                    min_step = 1;
                }
                else{// used[best_attack] < min_step
                    int prev = used[best_attack];
                    used[best_attack] = min_step;
                    min_step -= prev;
                }
            }
            else{
                used[best_attack] = min_step;
            }
        }
        sum += (min_step == 1000 ? 20 : min_step);
    }
    return sum;
}
std::string position_key(Position& pos){
    return pos.toFEN();
}

void resolve(Position &pos)
{
    
    auto start_time = std::chrono::high_resolution_clock::now();

    if(pos.winner() == Black){ // already win
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        info << std::fixed << std::setprecision(3) << duration.count() / 1000.0 << "\n";
        info << "0\n";
        return;
    }
    
    std::unordered_map<std::string, int>visited;// key: pos_key, value: node index
    std::vector<Node>nodes;
    Move m;
    Node start_node(pos, 0, heuristic(pos), -1, m);
    nodes.push_back(start_node);
    // for pq, Compare(a, b) returns true if a has lower priority than b
    auto cmp = [&nodes](int a, int b) {
        const Node &na = nodes[a];
        const Node &nb = nodes[b];
        // primary: lower f-cost first
        if (na.f_cost != nb.f_cost)
            return na.f_cost > nb.f_cost;
        // secondary: fewer pieces left on the board first
        return na.h_cost > nb.h_cost;
        // return na.g_cost < nb.g_cost;
    };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> pq(cmp);
    pq.push(0);// push the index of the first node

    while(!pq.empty()){
        int cur_index = pq.top();
        pq.pop();

        Node cur = nodes[cur_index];
        debug << "f_cost = " << cur.f_cost << ", g_cost = " << cur.g_cost << ", h_cost = " << cur.h_cost << "\n";
        debug << cur.position;

        if(cur.position.winner() == Black){
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
            info << std::fixed << std::setprecision(3) << duration.count() / 1000.0 << "\n";
            info << cur.f_cost << "\n";
            std::vector<Move>moves;
            while(cur_index != 0){
                moves.push_back(nodes[cur_index].mv);
                cur_index = nodes[cur_index].parent;
            }
            for(int i = moves.size() - 1; i >= 0; i--){
                info << moves[i];
            }
            return;
        }

        MoveList<> moves(cur.position);
        for(Move move: moves){
            Position new_pos(cur.position);
            if(new_pos.do_move(move)){
                std::string new_pos_key = position_key(new_pos);
                int new_g = cur.g_cost + 1;
                int new_h = heuristic(new_pos);
                if(visited.find(new_pos_key) == visited.end() || nodes[visited[new_pos_key]].g_cost > new_g){
                    Node new_node(new_pos, new_g, new_h, cur_index, move);
                    nodes.push_back(new_node);
                    int new_index = nodes.size() - 1;
                    visited[new_pos_key] = new_index;
                    pq.push(new_index);
                }
            }
        }
    }
    info << -1;
    // if reach here, no solution was found. shouldn't happen though
}