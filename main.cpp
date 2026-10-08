#include "tbprobe.h"

#include <cstdint>
#include <cstdio>

// Square index: a1 = 0, b1 = 1, ..., h8 = 63.
constexpr uint64_t sq(char file, char rank) { return 1ULL << ((rank - '1') * 8 + (file - 'a')); }

// Print a bitboard as hex, as its 8 bytes (one byte per rank) and as a board.
void printBitboard(const char *name, uint64_t bb) {
    std::printf("%-8s = 0x%016llx\n", name, (unsigned long long)bb);
    std::printf("  bytes (rank 8 .. rank 1, bit order h..a):\n");
    for (int rank = 7; rank >= 0; --rank) {
        unsigned byte = (bb >> (rank * 8)) & 0xFF;
        std::printf("    rank %d: 0x%02x  ", rank + 1, byte);
        for (int bit = 7; bit >= 0; --bit) std::printf("%d", (byte >> bit) & 1);
        std::printf("    ");
        for (int file = 0; file < 8; ++file) std::printf("%c ", (byte >> file) & 1 ? 'x' : '.');
        std::printf("\n");
    }
    std::printf("                              a b c d e f g h\n\n");
}

int main() {
    if (!tb_init("Data") || TB_LARGEST == 0) {
        std::fprintf(stderr, "No tablebases found in Data/\n");
        return 1;
    }

    // White: Ke1, Qd1. Black: Ke8. White to move.
    uint64_t white  = sq('e', '1') | sq('d', '1');
    uint64_t black  = sq('e', '8');
    uint64_t kings  = sq('e', '1') | sq('e', '8');
    uint64_t queens = sq('d', '1');
    bool whiteToMove = true;

    // Everything passed to the probe functions.
    printBitboard("white", white);
    printBitboard("black", black);
    printBitboard("kings", kings);
    printBitboard("queens", queens);
    printBitboard("rooks", 0);
    printBitboard("bishops", 0);
    printBitboard("knights", 0);
    printBitboard("pawns", 0);
    std::printf("rule50   = %u\ncastling = %u\nep       = %u\nturn     = %d (%s)\n\n", 0u, 0u, 0u,
                whiteToMove, whiteToMove ? "white" : "black");

    // WDL: win / draw / loss for the side to move.
    unsigned wdl = tb_probe_wdl(white, black, kings, queens, 0, 0, 0, 0, 0, 0, 0, whiteToMove);
    
    if (wdl == TB_RESULT_FAILED) {
        std::fprintf(stderr, "WDL probe failed\n");
        return 1;
    }
    const char *names[] = {"loss", "blessed loss", "draw", "cursed win", "win"};
    std::printf("WDL: %s\n", names[wdl]);

    // Root probe: DTZ and the best move.
    unsigned res = tb_probe_root(white, black, kings, queens, 0, 0, 0, 0, 0, 0, 0, whiteToMove, nullptr);
    if (res == TB_RESULT_FAILED) {
        std::fprintf(stderr, "DTZ probe failed\n");
        return 1;
    }
    if (res == TB_RESULT_CHECKMATE) { std::printf("Checkmate\n"); return 0; }
    if (res == TB_RESULT_STALEMATE) { std::printf("Stalemate\n"); return 0; }

    unsigned from = TB_GET_FROM(res), to = TB_GET_TO(res);
    std::printf("DTZ: %u plies\n", TB_GET_DTZ(res));
    std::printf("Best move: %c%c%c%c\n", 'a' + from % 8, '1' + from / 8, 'a' + to % 8, '1' + to / 8);

    tb_free();
    return 0;
}
