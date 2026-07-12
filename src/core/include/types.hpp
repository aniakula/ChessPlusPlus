#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <ostream>
#include <utility>

namespace chesspp::core {

using Bitboard = std::uint64_t;
using HashKey = std::uint64_t;
using Square = std::uint8_t;
using Score = std::int32_t;
using Phase = std::uint8_t;

// score of something at start of game vs end of game:
using Weight = std::pair<Score, Score>;

// for less wide Position score arrays:
using W = Weight;

inline constexpr Square NO_SQUARE = 64;
inline constexpr int SQUARE_COUNT = 64;
inline constexpr int COLOR_COUNT = 2;
inline constexpr int PIECE_TYPE_COUNT = 6;
inline constexpr const char *STARTING_POSITION_FEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

// phase weights for interpolation:
inline constexpr Phase OPENING_PHASE_WEIGHT = 24;
inline constexpr Phase KNIGHTS_PHASE_WEIGHT = 1;
inline constexpr Phase BISHOPS_PHASE_WEIGHT = 1;
inline constexpr Phase ROOKS_PHASE_WEIGHT = 2;
inline constexpr Phase QUEENS_PHASE_WEIGHT = 4;

// all scores are in centi-pawns (1/100th of a pawn's value)
inline constexpr Score INF = std::numeric_limits<Score>::max() / 2;
inline constexpr Score MATE_SCORE = INF;

inline constexpr Score PAWN_SCORE = 100;
inline constexpr Score KNIGHT_SCORE = 320;
inline constexpr Score BISHOP_SCORE = 330;
inline constexpr Score ROOK_SCORE = 500;
inline constexpr Score QUEEN_SCORE = 900;

/*
int mg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

int mg_bishop_table[64] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

int mg_queen_table[64] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};
*/

// Piece-square tables as W(middlegame, endgame).
// Square 0 = a1.
// BLACK tables use source[sq] as written; WHITE tables use source[sq ^ 56].

inline constexpr std::array<Weight, 64> WHITE_PAWN_POSITION_SCORE{
    W(0, 0),    W(0, 0),     W(0, 0),    W(0, 0),    W(0, 0),    W(0, 0),
    W(0, 0),    W(0, 0),     W(-35, 13), W(-1, 8),   W(-20, 8),  W(-23, 10),
    W(-15, 13), W(24, 0),    W(38, 2),   W(-22, -7), W(-26, 4),  W(-4, 7),
    W(-4, -6),  W(-10, 1),   W(3, 0),    W(3, -5),   W(33, -1),  W(-12, -8),
    W(-27, 13), W(-2, 9),    W(-5, -3),  W(12, -7),  W(17, -7),  W(6, -8),
    W(10, 3),   W(-25, -1),  W(-14, 32), W(13, 24),  W(6, 13),   W(21, 5),
    W(23, -2),  W(12, 4),    W(17, 17),  W(-23, 17), W(-6, 94),  W(7, 100),
    W(26, 85),  W(31, 67),   W(65, 56),  W(56, 53),  W(25, 82),  W(-20, 84),
    W(98, 178), W(134, 173), W(61, 158), W(95, 134), W(68, 147), W(126, 132),
    W(34, 165), W(-11, 187), W(0, 0),    W(0, 0),    W(0, 0),    W(0, 0),
    W(0, 0),    W(0, 0),     W(0, 0),    W(0, 0)};

inline constexpr std::array<Weight, 64> WHITE_KNIGHT_POSITION_SCORE{
    W(-105, -29), W(-21, -51),  W(-58, -23), W(-33, -15), W(-17, -22),
    W(-28, -18),  W(-19, -50),  W(-23, -64), W(-29, -42), W(-53, -20),
    W(-12, -10),  W(-3, -5),    W(-1, -2),   W(18, -20),  W(-14, -23),
    W(-19, -44),  W(-23, -23),  W(-9, -3),   W(12, -1),   W(10, 15),
    W(19, 10),    W(17, -3),    W(25, -20),  W(-16, -22), W(-13, -18),
    W(4, -6),     W(16, 16),    W(13, 25),   W(28, 16),   W(19, 17),
    W(21, 4),     W(-8, -18),   W(-9, -17),  W(17, 3),    W(19, 22),
    W(53, 22),    W(37, 22),    W(69, 11),   W(18, 8),    W(22, -18),
    W(-47, -24),  W(60, -20),   W(37, 10),   W(65, 9),    W(84, -1),
    W(129, -9),   W(73, -19),   W(44, -41),  W(-73, -25), W(-41, -8),
    W(72, -25),   W(36, -2),    W(23, -9),   W(62, -25),  W(7, -24),
    W(-17, -52),  W(-167, -58), W(-89, -38), W(-34, -13), W(-49, -28),
    W(61, -31),   W(-97, -27),  W(-15, -63), W(-107, -99)};

inline constexpr std::array<Weight, 64> WHITE_BISHOP_POSITION_SCORE{
    W(-33, -23), W(-3, -9),   W(-14, -23), W(-21, -5),  W(-13, -9),
    W(-12, -16), W(-39, -5),  W(-21, -17), W(4, -14),   W(15, -18),
    W(16, -7),   W(0, -1),    W(7, 4),     W(21, -9),   W(33, -15),
    W(1, -27),   W(0, -12),   W(15, -3),   W(15, 8),    W(15, 10),
    W(14, 13),   W(27, 3),    W(18, -7),   W(10, -15),  W(-6, -6),
    W(13, 3),    W(13, 13),   W(26, 19),   W(34, 7),    W(12, 10),
    W(10, -3),   W(4, -9),    W(-4, -3),   W(5, 9),     W(19, 12),
    W(50, 9),    W(37, 14),   W(37, 10),   W(7, 3),     W(-2, 2),
    W(-16, 2),   W(37, -8),   W(43, 0),    W(40, -1),   W(35, -2),
    W(50, 6),    W(37, 0),    W(-2, 4),    W(-26, -8),  W(16, -4),
    W(-18, 7),   W(-13, -12), W(30, -3),   W(59, -13),  W(18, -4),
    W(-47, -14), W(-29, -14), W(4, -21),   W(-82, -11), W(-37, -8),
    W(-25, -7),  W(-42, -9),  W(7, -17),   W(-8, -24)};

inline constexpr std::array<Weight, 64> WHITE_ROOK_POSITION_SCORE{
    W(-19, -9), W(-13, 2),   W(1, 3),    W(17, -1),  W(16, -5),  W(7, -13),
    W(-37, 4),  W(-26, -20), W(-44, -6), W(-16, -6), W(-20, 0),  W(-9, 2),
    W(-1, -9),  W(11, -9),   W(-6, -11), W(-71, -3), W(-45, -4), W(-25, 0),
    W(-16, -5), W(-17, -1),  W(3, -7),   W(0, -12),  W(-5, -8),  W(-33, -16),
    W(-36, 3),  W(-26, 5),   W(-12, 8),  W(-1, 4),   W(9, -5),   W(-7, -6),
    W(6, -8),   W(-23, -11), W(-24, 4),  W(-11, 3),  W(7, 13),   W(26, 1),
    W(24, 2),   W(35, 1),    W(-8, -1),  W(-20, 2),  W(-5, 7),   W(19, 7),
    W(26, 7),   W(36, 5),    W(17, 4),   W(45, -3),  W(61, -5),  W(16, -3),
    W(27, 11),  W(32, 13),   W(58, 13),  W(62, 11),  W(80, -3),  W(67, 3),
    W(26, 8),   W(44, 3),    W(32, 13),  W(42, 10),  W(32, 18),  W(51, 15),
    W(63, 12),  W(9, 12),    W(31, 8),   W(43, 5)};

inline constexpr std::array<Weight, 64> WHITE_QUEEN_POSITION_SCORE{
    W(-1, -33),  W(-18, -28), W(-9, -22),  W(10, -43), W(-15, -5),  W(-25, -32),
    W(-31, -20), W(-50, -41), W(-35, -22), W(-8, -23), W(11, -30),  W(2, -16),
    W(8, -16),   W(15, -23),  W(-3, -36),  W(1, -32),  W(-14, -16), W(2, -27),
    W(-11, 15),  W(-2, 6),    W(-5, 9),    W(2, 17),   W(14, 10),   W(5, 5),
    W(-9, -18),  W(-26, 28),  W(-9, 19),   W(-10, 47), W(-2, 31),   W(-4, 34),
    W(3, 39),    W(-3, 23),   W(-27, 3),   W(-27, 22), W(-16, 24),  W(-16, 45),
    W(-1, 57),   W(17, 40),   W(-2, 57),   W(1, 36),   W(-13, -20), W(-17, 6),
    W(7, 9),     W(8, 49),    W(29, 47),   W(56, 35),  W(47, 19),   W(57, 9),
    W(-24, -17), W(-39, 20),  W(-5, 32),   W(1, 41),   W(-16, 58),  W(57, 25),
    W(28, 30),   W(54, 0),    W(-28, -9),  W(0, 22),   W(29, 22),   W(12, 27),
    W(59, 27),   W(44, 19),   W(43, 10),   W(45, 20)};

inline constexpr std::array<Weight, 64> WHITE_KING_POSITION_SCORE{
    W(-15, -53), W(36, -34),  W(12, -21), W(-54, -11), W(8, -28),
    W(-28, -14), W(24, -24),  W(14, -43), W(1, -27),   W(7, -11),
    W(-8, 4),    W(-64, 13),  W(-43, 14), W(-16, 4),   W(9, -5),
    W(8, -17),   W(-14, -19), W(-14, -3), W(-22, 11),  W(-46, 21),
    W(-44, 23),  W(-30, 16),  W(-15, 7),  W(-27, -9),  W(-49, -18),
    W(-1, -4),   W(-27, 21),  W(-39, 24), W(-46, 27),  W(-44, 23),
    W(-33, 9),   W(-51, -11), W(-17, -8), W(-20, 22),  W(-12, 24),
    W(-27, 27),  W(-30, 26),  W(-25, 33), W(-14, 26),  W(-36, 3),
    W(-9, 10),   W(24, 17),   W(2, 23),   W(-16, 15),  W(-20, 20),
    W(6, 45),    W(22, 44),   W(-22, 13), W(29, -12),  W(-1, 17),
    W(-20, 14),  W(-7, 17),   W(-8, 17),  W(-4, 38),   W(-38, 23),
    W(-29, 11),  W(-65, -74), W(23, -35), W(16, -18),  W(-15, -18),
    W(-56, -11), W(-34, 15),  W(2, 4),    W(13, -17)};

inline constexpr std::array<Weight, 64> BLACK_PAWN_POSITION_SCORE{
    W(0, 0),    W(0, 0),     W(0, 0),    W(0, 0),     W(0, 0),    W(0, 0),
    W(0, 0),    W(0, 0),     W(98, 178), W(134, 173), W(61, 158), W(95, 134),
    W(68, 147), W(126, 132), W(34, 165), W(-11, 187), W(-6, 94),  W(7, 100),
    W(26, 85),  W(31, 67),   W(65, 56),  W(56, 53),   W(25, 82),  W(-20, 84),
    W(-14, 32), W(13, 24),   W(6, 13),   W(21, 5),    W(23, -2),  W(12, 4),
    W(17, 17),  W(-23, 17),  W(-27, 13), W(-2, 9),    W(-5, -3),  W(12, -7),
    W(17, -7),  W(6, -8),    W(10, 3),   W(-25, -1),  W(-26, 4),  W(-4, 7),
    W(-4, -6),  W(-10, 1),   W(3, 0),    W(3, -5),    W(33, -1),  W(-12, -8),
    W(-35, 13), W(-1, 8),    W(-20, 8),  W(-23, 10),  W(-15, 13), W(24, 0),
    W(38, 2),   W(-22, -7),  W(0, 0),    W(0, 0),     W(0, 0),    W(0, 0),
    W(0, 0),    W(0, 0),     W(0, 0),    W(0, 0)};

inline constexpr std::array<Weight, 64> BLACK_KNIGHT_POSITION_SCORE{
    W(-167, -58), W(-89, -38),  W(-34, -13),  W(-49, -28), W(61, -31),
    W(-97, -27),  W(-15, -63),  W(-107, -99), W(-73, -25), W(-41, -8),
    W(72, -25),   W(36, -2),    W(23, -9),    W(62, -25),  W(7, -24),
    W(-17, -52),  W(-47, -24),  W(60, -20),   W(37, 10),   W(65, 9),
    W(84, -1),    W(129, -9),   W(73, -19),   W(44, -41),  W(-9, -17),
    W(17, 3),     W(19, 22),    W(53, 22),    W(37, 22),   W(69, 11),
    W(18, 8),     W(22, -18),   W(-13, -18),  W(4, -6),    W(16, 16),
    W(13, 25),    W(28, 16),    W(19, 17),    W(21, 4),    W(-8, -18),
    W(-23, -23),  W(-9, -3),    W(12, -1),    W(10, 15),   W(19, 10),
    W(17, -3),    W(25, -20),   W(-16, -22),  W(-29, -42), W(-53, -20),
    W(-12, -10),  W(-3, -5),    W(-1, -2),    W(18, -20),  W(-14, -23),
    W(-19, -44),  W(-105, -29), W(-21, -51),  W(-58, -23), W(-33, -15),
    W(-17, -22),  W(-28, -18),  W(-19, -50),  W(-23, -64)};

inline constexpr std::array<Weight, 64> BLACK_BISHOP_POSITION_SCORE{
    W(-29, -14), W(4, -21),   W(-82, -11), W(-37, -8),  W(-25, -7),
    W(-42, -9),  W(7, -17),   W(-8, -24),  W(-26, -8),  W(16, -4),
    W(-18, 7),   W(-13, -12), W(30, -3),   W(59, -13),  W(18, -4),
    W(-47, -14), W(-16, 2),   W(37, -8),   W(43, 0),    W(40, -1),
    W(35, -2),   W(50, 6),    W(37, 0),    W(-2, 4),    W(-4, -3),
    W(5, 9),     W(19, 12),   W(50, 9),    W(37, 14),   W(37, 10),
    W(7, 3),     W(-2, 2),    W(-6, -6),   W(13, 3),    W(13, 13),
    W(26, 19),   W(34, 7),    W(12, 10),   W(10, -3),   W(4, -9),
    W(0, -12),   W(15, -3),   W(15, 8),    W(15, 10),   W(14, 13),
    W(27, 3),    W(18, -7),   W(10, -15),  W(4, -14),   W(15, -18),
    W(16, -7),   W(0, -1),    W(7, 4),     W(21, -9),   W(33, -15),
    W(1, -27),   W(-33, -23), W(-3, -9),   W(-14, -23), W(-21, -5),
    W(-13, -9),  W(-12, -16), W(-39, -5),  W(-21, -17)};

inline constexpr std::array<Weight, 64> BLACK_ROOK_POSITION_SCORE{
    W(32, 13),  W(42, 10),  W(32, 18),  W(51, 15),   W(63, 12),  W(9, 12),
    W(31, 8),   W(43, 5),   W(27, 11),  W(32, 13),   W(58, 13),  W(62, 11),
    W(80, -3),  W(67, 3),   W(26, 8),   W(44, 3),    W(-5, 7),   W(19, 7),
    W(26, 7),   W(36, 5),   W(17, 4),   W(45, -3),   W(61, -5),  W(16, -3),
    W(-24, 4),  W(-11, 3),  W(7, 13),   W(26, 1),    W(24, 2),   W(35, 1),
    W(-8, -1),  W(-20, 2),  W(-36, 3),  W(-26, 5),   W(-12, 8),  W(-1, 4),
    W(9, -5),   W(-7, -6),  W(6, -8),   W(-23, -11), W(-45, -4), W(-25, 0),
    W(-16, -5), W(-17, -1), W(3, -7),   W(0, -12),   W(-5, -8),  W(-33, -16),
    W(-44, -6), W(-16, -6), W(-20, 0),  W(-9, 2),    W(-1, -9),  W(11, -9),
    W(-6, -11), W(-71, -3), W(-19, -9), W(-13, 2),   W(1, 3),    W(17, -1),
    W(16, -5),  W(7, -13),  W(-37, 4),  W(-26, -20)};

inline constexpr std::array<Weight, 64> BLACK_QUEEN_POSITION_SCORE{
    W(-28, -9),  W(0, 22),    W(29, 22),   W(12, 27),   W(59, 27),   W(44, 19),
    W(43, 10),   W(45, 20),   W(-24, -17), W(-39, 20),  W(-5, 32),   W(1, 41),
    W(-16, 58),  W(57, 25),   W(28, 30),   W(54, 0),    W(-13, -20), W(-17, 6),
    W(7, 9),     W(8, 49),    W(29, 47),   W(56, 35),   W(47, 19),   W(57, 9),
    W(-27, 3),   W(-27, 22),  W(-16, 24),  W(-16, 45),  W(-1, 57),   W(17, 40),
    W(-2, 57),   W(1, 36),    W(-9, -18),  W(-26, 28),  W(-9, 19),   W(-10, 47),
    W(-2, 31),   W(-4, 34),   W(3, 39),    W(-3, 23),   W(-14, -16), W(2, -27),
    W(-11, 15),  W(-2, 6),    W(-5, 9),    W(2, 17),    W(14, 10),   W(5, 5),
    W(-35, -22), W(-8, -23),  W(11, -30),  W(2, -16),   W(8, -16),   W(15, -23),
    W(-3, -36),  W(1, -32),   W(-1, -33),  W(-18, -28), W(-9, -22),  W(10, -43),
    W(-15, -5),  W(-25, -32), W(-31, -20), W(-50, -41)};

inline constexpr std::array<Weight, 64> BLACK_KING_POSITION_SCORE{
    W(-65, -74), W(23, -35),  W(16, -18),  W(-15, -18), W(-56, -11),
    W(-34, 15),  W(2, 4),     W(13, -17),  W(29, -12),  W(-1, 17),
    W(-20, 14),  W(-7, 17),   W(-8, 17),   W(-4, 38),   W(-38, 23),
    W(-29, 11),  W(-9, 10),   W(24, 17),   W(2, 23),    W(-16, 15),
    W(-20, 20),  W(6, 45),    W(22, 44),   W(-22, 13),  W(-17, -8),
    W(-20, 22),  W(-12, 24),  W(-27, 27),  W(-30, 26),  W(-25, 33),
    W(-14, 26),  W(-36, 3),   W(-49, -18), W(-1, -4),   W(-27, 21),
    W(-39, 24),  W(-46, 27),  W(-44, 23),  W(-33, 9),   W(-51, -11),
    W(-14, -19), W(-14, -3),  W(-22, 11),  W(-46, 21),  W(-44, 23),
    W(-30, 16),  W(-15, 7),   W(-27, -9),  W(1, -27),   W(7, -11),
    W(-8, 4),    W(-64, 13),  W(-43, 14),  W(-16, 4),   W(9, -5),
    W(8, -17),   W(-15, -53), W(36, -34),  W(12, -21),  W(-54, -11),
    W(8, -28),   W(-28, -14), W(24, -24),  W(14, -43)};

inline constexpr Weight BISHOP_PAIR_WEIGHT{10, 40};

enum class Color : std::uint8_t {
  White = 0,
  Black = 1,
};

enum class PieceType : std::uint8_t {
  Pawn = 0,
  Knight = 1,
  Bishop = 2,
  Rook = 3,
  Queen = 4,
  King = 5,
  None = 6,
};

[[nodiscard]] constexpr Phase phase_weight(PieceType piece) noexcept {
  switch (piece) {
  case PieceType::Knight:
    return KNIGHTS_PHASE_WEIGHT;
  case PieceType::Bishop:
    return BISHOPS_PHASE_WEIGHT;
  case PieceType::Rook:
    return ROOKS_PHASE_WEIGHT;
  case PieceType::Queen:
    return QUEENS_PHASE_WEIGHT;
  default:
    return 0;
  }
}

// Interpolate opening/endgame scores by remaining game phase.
[[nodiscard]] constexpr Score taper(Weight weight, Phase phase) noexcept {
  return (weight.first * static_cast<Score>(phase) +
          weight.second * static_cast<Score>(OPENING_PHASE_WEIGHT - phase)) /
         static_cast<Score>(OPENING_PHASE_WEIGHT);
}

[[nodiscard]] constexpr Phase clamp_phase(int phase) noexcept {
  if (phase < 0) {
    return 0;
  }
  if (phase > static_cast<int>(OPENING_PHASE_WEIGHT)) {
    return OPENING_PHASE_WEIGHT;
  }
  return static_cast<Phase>(phase);
}

enum class GameResult : std::uint8_t {
  Ongoing,
  WhiteWins,
  BlackWins,
  Draw,
};

enum class DrawReason : std::uint8_t {
  None,
  Stalemate,
  FiftyMoveRule,
  ThreefoldRepetition,
  InsufficientMaterial,
};

enum class MoveFlag : std::uint16_t {
  Quiet = 0,
  Capture = 1 << 0,
  DoublePawnPush = 1 << 1,
  EnPassant = 1 << 2,
  KingCastle = 1 << 3,
  QueenCastle = 1 << 4,
  Promotion = 1 << 5,
};

[[nodiscard]] constexpr char piece_to_char(Color color, PieceType piece) {
  char diff = color == Color::Black ? 0 : 56;
  switch (piece) {
  case PieceType::Pawn:
    return ('p' - diff);
  case PieceType::Knight:
    return ('n' - diff);
  case PieceType::Bishop:
    return ('b' - diff);
  case PieceType::Rook:
    return ('r' - diff);
  case PieceType::Queen:
    return ('q' - diff);
  case PieceType::King:
    return ('k' - diff);
  default:
    return '?';
  }
}

[[nodiscard]] constexpr std::pair<Color, PieceType> char_to_piece(char token) {
  switch (token) {
  case 'p':
    return std::make_pair(Color::Black, PieceType::Pawn);
  case 'n':
    return std::make_pair(Color::Black, PieceType::Knight);
  case 'b':
    return std::make_pair(Color::Black, PieceType::Bishop);
  case 'r':
    return std::make_pair(Color::Black, PieceType::Rook);
  case 'q':
    return std::make_pair(Color::Black, PieceType::Queen);
  case 'k':
    return std::make_pair(Color::Black, PieceType::King);
  case 'P':
    return std::make_pair(Color::White, PieceType::Pawn);
  case 'N':
    return std::make_pair(Color::White, PieceType::Knight);
  case 'B':
    return std::make_pair(Color::White, PieceType::Bishop);
  case 'R':
    return std::make_pair(Color::White, PieceType::Rook);
  case 'Q':
    return std::make_pair(Color::White, PieceType::Queen);
  case 'K':
    return std::make_pair(Color::White, PieceType::King);
  default:
    return std::make_pair(Color::White, PieceType::None);
  }
}

[[nodiscard]] constexpr std::size_t color_index(Color color) {
  return static_cast<std::size_t>(color);
}

[[nodiscard]] constexpr std::size_t piece_index(PieceType piece) {
  return static_cast<std::size_t>(piece);
}

[[nodiscard]] constexpr Color opposite(Color color) {
  return color == Color::White ? Color::Black : Color::White;
}

[[nodiscard]] constexpr Square square_from(int file, int rank) {
  return static_cast<Square>((rank * 8) + file);
}

[[nodiscard]] constexpr Square square_from_algebraic(char file, char rank) {
  return square_from(static_cast<int>(file - 'a'),
                     static_cast<int>(rank - '1'));
}

[[nodiscard]] constexpr MoveFlag operator|(MoveFlag lhs, MoveFlag rhs) {
  return static_cast<MoveFlag>(static_cast<std::uint16_t>(lhs) |
                               static_cast<std::uint16_t>(rhs));
}

[[nodiscard]] constexpr bool has_flag(MoveFlag flags, MoveFlag flag) {
  return (static_cast<std::uint16_t>(flags) &
          static_cast<std::uint16_t>(flag)) != 0;
}

[[nodiscard]] constexpr Square pop_lsb(Bitboard &bitboard) noexcept {
  const Square square = static_cast<Square>(std::countr_zero(bitboard));
  bitboard &= bitboard - 1;
  return square;
}

// move encoding Layout:
// bits 0-5   from square
// bits 6-11  to square
// bits 12-15 promotion piece
// bits 16-31 flags
class Move {
public:
  constexpr Move() = default;
  constexpr Move(Square from, Square to, PieceType promotion = PieceType::None,
                 MoveFlag flags = MoveFlag::Quiet)
      : data_{static_cast<std::uint32_t>(from) |
              (static_cast<std::uint32_t>(to) << 6) |
              (static_cast<std::uint32_t>(promotion) << 12) |
              (static_cast<std::uint32_t>(flags) << 16)} {}

  [[nodiscard]] static constexpr Move null() { return Move{}; }
  [[nodiscard]] constexpr bool is_null() const { return data_ == 0; }
  [[nodiscard]] constexpr Square from() const {
    return static_cast<Square>(data_ & 0x3F);
  }
  [[nodiscard]] constexpr Square to() const {
    return static_cast<Square>((data_ >> 6) & 0x3F);
  }
  [[nodiscard]] constexpr PieceType promotion() const {
    return static_cast<PieceType>((data_ >> 12) & 0x0F);
  }
  [[nodiscard]] constexpr MoveFlag flags() const {
    return static_cast<MoveFlag>((data_ >> 16) & 0xFFFF);
  }
  [[nodiscard]] constexpr std::uint32_t raw() const { return data_; }

  [[nodiscard]] constexpr bool operator==(const Move &) const = default;

private:
  std::uint32_t data_{0};
};

inline std::ostream &operator<<(std::ostream &out, Move move) {
  const auto write_square = [&out](Square square) {
    if (square == NO_SQUARE) {
      out << "-";
      return;
    }

    out << static_cast<char>('a' + (square % 8))
        << static_cast<char>('1' + (square / 8));
  };

  if (move.is_null()) {
    out << "<null move>";
    return out;
  }

  write_square(move.from());
  write_square(move.to());

  if (move.promotion() != PieceType::None) {
    switch (move.promotion()) {
    case PieceType::Knight:
      out << "=N";
      break;
    case PieceType::Bishop:
      out << "=B";
      break;
    case PieceType::Rook:
      out << "=R";
      break;
    case PieceType::Queen:
      out << "=Q";
      break;
    default:
      break;
    }
  }

  const MoveFlag flags = move.flags();
  if (flags == MoveFlag::Quiet) {
    return out;
  }

  const char *separator = " (";
  const auto write_flag = [&](MoveFlag flag, const char *name) {
    if (has_flag(flags, flag)) {
      out << separator << name;
      separator = ", ";
    }
  };

  write_flag(MoveFlag::Capture, "capture");
  write_flag(MoveFlag::DoublePawnPush, "double-pawn-push");
  write_flag(MoveFlag::EnPassant, "en-passant");
  write_flag(MoveFlag::KingCastle, "king-castle");
  write_flag(MoveFlag::QueenCastle, "queen-castle");
  write_flag(MoveFlag::Promotion, "promotion");

  out << ')';
  return out;
}

struct ScoredMove {
  Move move{};
  Score score{0};
};

// bitboard per piece per color:
using PieceBitboards =
    std::array<std::array<Bitboard, PIECE_TYPE_COUNT>, COLOR_COUNT>;

} // namespace chesspp::core
