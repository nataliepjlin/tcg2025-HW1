#include "solver.h"
#include "lib/helper.h"

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
void resolve(Position &pos)
{
    /* You can use these aliases */
    info << "Output (only) your answers here!\n"; // ====> stdout <====
    debug << "Print debug info here!\n";          // stderr
    error << "Print errors here?\n";              // also stderr

    info << pos;

    /* A random number generator is available globally */
    int random_num_below_42 = rng(42);

    /* Generate moves like this */
    MoveList<All, Black> moves(pos); // `<All, Black>` is optional

    /* Iterate all moves */
    for (Move mv : moves) {
        info << mv;                       // print the move
        Position pos_copy(pos);           // copy the position
        bool done = pos_copy.do_move(mv); // do the move
    }

    /* Get some pieces */
    Board all_the_pieces = pos.pieces();
    Board black_pieces   = pos.pieces(Black);
    Board black_soldiers = pos.pieces(Black, Soldier);

    /* Iterate those pieces */
    for (Square sq : BoardView(black_pieces)) {
        info << "There is a black piece at square " << sq << ".";

        // To get _complete_ information about the piece at a square,
        // use peek_piece_at()
        Piece p = pos.peek_piece_at(sq);
        if (p.type == Advisor) {
            info << " It is an advisor.";
        }

        info << "\n";
    }

    /* Distance between two squares */
    int dist      = distance(SQ_B2, SQ_C4);       // 3
    int dist_rank = distance<Rank>(SQ_B2, SQ_C4); // 1

    /* Example: output a random legal move */
    info << "Random walking:" << std::endl;
    for (int counter = 21; counter -- /* counter slides to 0 */
                                     \
                                      \
                                       \
                                        > 0;) {
        MoveList mvs(pos);
        if (mvs.size() == 0) {
            return;
        }
        Move chosen = mvs[rng(mvs.size())];
        info << (21 - counter) << ". " << chosen;
        pos.do_move(chosen);
    }
}