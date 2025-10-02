#include "solver.h"
#include "lib/helper.h"
#include <queue>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <chrono>

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
    return pos.count(Red);
}
std::string position_key(Position& pos){
    return pos.toFEN();
}

void resolve(Position &pos)
{
    
    auto start_time = std::chrono::high_resolution_clock::now();

    if(pos.winner() == Black){ // already win
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
        info << duration.count() << "\n";
        info << "0\n";
        return;
    }
    
    std::unordered_map<std::string, int>visited;// key: pos_key, value: node index
    std::vector<Node>nodes;
    Move m;
    Node start_node(pos, 0, heuristic(pos), -1, m);
    nodes.push_back(start_node);
    // pq stores indices of nodes, sorted by nodes[index].f_cost
    auto cmp = [&nodes](int a, int b) {
        const Node &na = nodes[a];
        const Node &nb = nodes[b];
        // Primary: lower f-cost first (min-heap behavior)
        if (na.f_cost != nb.f_cost)
            return na.f_cost > nb.f_cost;
        // Secondary: fewer pieces left on the board first
        if (na.h_cost != nb.h_cost)
            return na.h_cost > nb.h_cost;
        // Tertiary: original tie-breaker (prefer larger g on top with this comparator)
        return na.g_cost < nb.g_cost;
    };
    std::priority_queue<int, std::vector<int>, decltype(cmp)> pq(cmp);
    pq.push(0);// push the index of the first node

    while(!pq.empty()){
        int cur_index = pq.top();
        pq.pop();

        Node cur = nodes[cur_index];
        // debug << "f_cost = " << cur.f_cost << ", g_cost = " << cur.g_cost << ", h_cost = " << cur.h_cost << "\n";
        // debug << cur.position;

        if(cur.position.winner() == Black){
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time);
            info << duration.count() << "\n";
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