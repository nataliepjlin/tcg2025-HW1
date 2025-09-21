// Wakasagihime
// Plays Chinese Dark Chess (Banqi)!

// For HW1, please refer to solver.cpp

#include "lib/chess.h"
#include "lib/marisa.h"
#include "lib/types.h"
#include "solver.h"

// Girls are preparing...
__attribute__((constructor)) void prepare()
{
    // Prepare the distance table
    for (Square i = SQ_A1; i < SQUARE_NB; i += 1) {
        for (Square j = SQ_A1; j < SQUARE_NB; j += 1) {
            SquareDistance[i][j] = distance<Rank>(i, j) + distance<File>(i, j);
        }
    }

    // Prepare the attack table (regular)
    Direction dirs[4] = { NORTH, SOUTH, EAST, WEST };
    for (Square sq = SQ_A1; is_okay(sq); sq += 1) {
        Board a = 0;
        for (Direction d : dirs) {
            a |= safe_destination(sq, d);
        }
        PseudoAttacks[sq] = a;
    }

    // Prepare magic
    init_magic<Chariot>(chariotTable, chariotMagics);
    init_magic<Cannon>(cannonTable, cannonMagics);
}

// le fishe
int main()
{
    // Read test case
    std::string fen;
    std::getline(std::cin, fen);

    // Initialize position
    Position pos(fen);

#if !(WAKASAGI_VALIDATE)
    // It's up to you! See solver.cpp
    resolve(pos);
#else
    // HW1 Validate mode!
    Move mv;
    while (pos.winner() == NO_COLOR) {
        std::cin >> mv;
        bool success = pos.do_move(mv);
        if (!success) {
            error << "ILLEGAL\n";
            return 1;
        }
    }

    if (pos.winner() != Black) {
        error << "DIDN'T WIN\n";
        return 2;
    }

    info << "Good job!\n";
#endif
    return 0;
}