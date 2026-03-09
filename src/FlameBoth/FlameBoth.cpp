#include "FlameBoth.h"
#include "../BoardHash/BoardHash.h"
#include "../Chess/Chess.h"
#include "../Time/Time.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
const int pawnPST[64] = {0,   0,  0,  0,   0,  0,  0,   0,  50, 50, 50, 50, 50,
                         50,  50, 50, 10,  10, 20, 30,  30, 20, 10, 10, 5,  5,
                         10,  25, 25, 10,  5,  5,  0,   0,  0,  20, 20, 0,  0,
                         0,   5,  -5, -10, 0,  0,  -10, -5, 5,  5,  10, 10, -20,
                         -20, 10, 10, 5,   0,  0,  0,   0,  0,  0,  0,  0};
const int knightPST[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50};
const int bishopPST[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20};
const int rookPST[64] = {0,  0,  0, 0,  0, 0,  0,  0, 5,  10, 10, 10, 10,
                         10, 10, 5, -5, 0, 0,  0,  0, 0,  0,  -5, -5, 0,
                         0,  0,  0, 0,  0, -5, -5, 0, 0,  0,  0,  0,  0,
                         -5, -5, 0, 0,  0, 0,  0,  0, -5, -5, 0,  0,  0,
                         0,  0,  0, -5, 0, 0,  0,  5, 5,  0,  0,  0};
const int queenPST[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20};
const int kingMiddlePST[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20};
int mirrorIndex(int index) { return index ^ 56; }
} // namespace

void FlameBoth::Bot::checkTime() {
  if (timeLimitSeconds > 0 && searchTimer.elapsedTime() >= timeLimitSeconds) {
    timeOut = true;
  }
}

Chess::Move FlameBoth::Bot::getBestMove(Chess::Board board, int depth) {
  Chess::Move globalBestMove;
  Chess::Move miredownBackup;

  std::vector<Chess::Move> moves = getAllValidMoves(board, board.getTurn());
  if (moves.empty())
    return globalBestMove;

  globalBestMove = moves[0];

  timeOut = false;
  nodes = 0;
  searchTimer.start();

  for (int depthIndex = 1; depthIndex <= depth; depthIndex++) {
    Chess::Move currentDepthBestMove;
    bool isWhiteTurn = (board.getTurn() == Chess::Side::White);
    int bestVal = isWhiteTurn
                      ? -999999
                      : 999999; /**< INF macrosu nerede hatırlamıyorum */

    /**
     * bir önceki best move'u direk en üste taşıyor (fast moveOrdering)
     */
    if (depthIndex > 1) {
      for (size_t i = 0; i < moves.size(); ++i) {
        if (moves[i].getFrom().getCoordinate().file ==
                globalBestMove.getFrom().getCoordinate().file &&
            moves[i].getFrom().getCoordinate().rank ==
                globalBestMove.getFrom().getCoordinate().rank &&
            moves[i].getTo().getCoordinate().file ==
                globalBestMove.getTo().getCoordinate().file &&
            moves[i].getTo().getCoordinate().rank ==
                globalBestMove.getTo().getCoordinate().rank) {
          std::swap(moves[0], moves[i]);
          break;
        }
      }
    }

    /**
     * ilk derinlik search de yapılmıyor :
     * @todo yarım kalınca current best move direk yapılmasın! önceki depth den
     * kalan yapılsın
     */
    miredownBackup = globalBestMove;
    for (const auto &move : moves) {

      // Chess::GameState gs = Chess::getGameState(board);
      // std::cout << (gs == Chess::GameState::Ongoing) << " " << (gs ==
      // Chess::GameState::Checkmate) << " " << (gs ==
      // Chess::GameState::Stalemate) << "\n";

      Chess::Board tempBoard = board;
      Chess::makeMove(move, board.getTurn(), tempBoard); // artık flipliyor

      // gs = Chess::getGameState(board);
      // std::cout << (gs == Chess::GameState::Ongoing) << " " << (gs ==
      // Chess::GameState::Checkmate) << " " << (gs ==
      // Chess::GameState::Stalemate) << "\n";

      int moveVal = searchTree(tempBoard, depthIndex - 1, -INF, INF);

      if (isWhiteTurn && moveVal > bestVal) {
        bestVal = moveVal;
        currentDepthBestMove = move;
      } else if (moveVal < bestVal) {
        bestVal = moveVal;
        currentDepthBestMove = move;
      }

      /**
       * daha iyi zaman uyumu için :
       */
      if (timeOut) {
        currentDepthBestMove = miredownBackup;
        break;
      }
    }

    globalBestMove = currentDepthBestMove;

    // std::cout << "[Depth] " << depthIndex << " [finished] | [State]: " <<
    // bestVal << " | [Timer]: " << timer.elapsedTime() << "s" << "\n";

    if (timeOut) {
      break;
    }
  }

  return globalBestMove;
}

/**
 * @brief ana seachTree fonksiyonu
 * - alpha, beta cuting
 * - move Ordering (simple)
 * - string hashing
 * @param board hamle yapılacak tahta
 * @param depth mevucut yapılacak kalan derinlik
 * @param alpha bizim yapabildiğimiz en iyi move (top move for bot)
 * @param beta karşı tarafın yapabildiği en iyi move (top move for opposite)
 */
int FlameBoth::Bot::searchTree(Chess::Board board, int depth, int alpha,
                               int beta) {
  if ((nodes++ & 2047) == 0)
    checkTime();
  if (timeOut)
    return 0;

  Chess::Side currentSide = board.getTurn();

  uint64_t hashKey = BoardHash::generateHash(board);
  int hashIndex = hashKey % BoardHash::TABLE_SIZE;

  BoardHash::Entry &e = BoardHash::Table[hashIndex];
  if (e.key == hashKey && e.depth >= depth) {
    if (e.bound == BoardHash::BOUND_EXACT)
      return e.score;
    if (e.bound == BoardHash::BOUND_LOWER)
      alpha = std::max(alpha, e.score);
    else if (e.bound == BoardHash::BOUND_UPPER)
      beta = std::min(beta, e.score);
    if (alpha >= beta)
      return e.score;
  }

  std::vector<Chess::Move> moves = getAllValidMoves(board, currentSide);

  if (moves.empty()) {
    if (Chess::isKingInCheck(board, currentSide)) {
      if (currentSide == Chess::Side::White)
        return -MATE_SCORE + depth;
      else
        return MATE_SCORE - depth;
    } else
      return 0; // PAT
  }

  if (depth == 0)
    return evaluate(board);

  std::sort(moves.begin(), moves.end(),
            [&](const Chess::Move &a, const Chess::Move &b) {
              return fastMoveOrdering(a, board) > fastMoveOrdering(b, board);
            });

  int value;
  int alphaOrig = alpha;
  bool isMaximizing = (currentSide == Chess::Side::White);

  if (isMaximizing) {
    value = -INF;
    for (const auto &move : moves) {
      Chess::Board tmp = board;
      Chess::makeMove(move, currentSide, tmp);

      int eval = searchTree(tmp, depth - 1, alpha, beta);
      value = std::max(value, eval);
      alpha = std::max(alpha, eval);
      if (alpha >= beta)
        break;
    }
  } else {
    value = INF;
    for (const auto &move : moves) {
      Chess::Board tmp = board;
      Chess::makeMove(move, currentSide, tmp);

      int eval = searchTree(tmp, depth - 1, alpha, beta);
      value = std::min(value, eval);
      beta = std::min(beta, eval);
      if (alpha >= beta)
        break;
    }
  }

  BoardHash::Entry newEntry;
  newEntry.key = hashKey;
  newEntry.score = value;
  newEntry.depth = depth;
  if (value <= alphaOrig)
    newEntry.bound = BoardHash::BOUND_UPPER;
  else if (value >= beta)
    newEntry.bound = BoardHash::BOUND_LOWER;
  else
    newEntry.bound = BoardHash::BOUND_EXACT;
  BoardHash::Table[hashIndex] = newEntry;

  return value;
}

double specificSigmoid(int totalNonPawnMaterial) {
  double midPoint = 3500.0;
  double k = 0.0025;

  double phase = 1.0 / (1.0 + std::exp(-k * (totalNonPawnMaterial - midPoint)));

  return phase;
}

int FlameBoth::Bot::getRawPieceValue(Chess::Piece type) {
  switch (type) {
  case Chess::Piece::Pawn:
    return (int)EvulateValue::PawnValue;
  case Chess::Piece::Knight:
    return (int)EvulateValue::KnightValue;
  case Chess::Piece::Bishop:
    return (int)EvulateValue::BishopValue;
  case Chess::Piece::Rook:
    return (int)EvulateValue::RookValue;
  case Chess::Piece::Queen:
    return (int)EvulateValue::QueenValue;
  case Chess::Piece::King:
    return (int)EvulateValue::MateValue;
  default:
    return 0;
  }
}

int FlameBoth::Bot::getMaterialScore(const Chess::Board &board) {
  int whiteScore = 0;
  int blackScore = 0;

  for (int r = 1; r <= 8; ++r) {
    for (int f = 1; f <= 8; ++f) {
      Chess::Square sq = board.getSquare({(Chess::File)f, (Chess::Rank)r});
      if (sq.getPieceType() == Chess::Piece::Empty)
        continue;

      int val = getRawPieceValue(sq.getPieceType());
      int pstVal = 0;

      // 0=A8, 63=H1
      int sqIndex = (8 - r) * 8 + (f - 1);
      if (sq.getPieceSide() == Chess::Side::Black) {
        sqIndex = mirrorIndex(sqIndex);
      }

      switch (sq.getPieceType()) {
      case Chess::Piece::Pawn:
        pstVal = pawnPST[sqIndex];
        break;
      case Chess::Piece::Knight:
        pstVal = knightPST[sqIndex];
        break;
      case Chess::Piece::Bishop:
        pstVal = bishopPST[sqIndex];
        break;
      case Chess::Piece::Rook:
        pstVal = rookPST[sqIndex];
        break;
      case Chess::Piece::Queen:
        pstVal = queenPST[sqIndex];
        break;
      case Chess::Piece::King:
        pstVal = kingMiddlePST[sqIndex];
        break;
      default:
        break;
      }

      if (sq.getPieceSide() == Chess::Side::White) {
        whiteScore += val + pstVal;
      } else {
        blackScore += val + pstVal;
      }
    }
  }

  return whiteScore - blackScore;
}

int FlameBoth::Bot::getCastlingScore(const Chess::Board &board) {
  int castlingScore = 0;
  int castlingScoreWhite = 0;
  int castlingScoreBlack = 0;

  // if(board.isCastled(Side::Black))
  // if(board.isCastled(Side::White))

  castlingScore = castlingScoreWhite - castlingScoreBlack;
  return castlingScore;
}

int FlameBoth::Bot::getKingMoveScore(const Chess::Board &board) { return 0; }

int FlameBoth::Bot::evaluate(const Chess::Board &board) {
  int scoreSum = 0;
  scoreSum += getMaterialScore(board);

  return scoreSum;
}

std::vector<Chess::Move>
FlameBoth::Bot::getAllValidMoves(const Chess::Board &board, Chess::Side side) {
  std::vector<Chess::Move> validMoves;
  validMoves.reserve(64);

  for (int r1 = 1; r1 <= 8; ++r1) {
    for (int f1 = 1; f1 <= 8; ++f1) {
      Chess::Square srcSq = board.getSquare({(Chess::File)f1, (Chess::Rank)r1});

      if (srcSq.getPieceSide() == side) {
        Chess::Piece pieceType = srcSq.getPieceType();
        std::vector<std::pair<int, int>> targets;
        targets.reserve(28);

        if (pieceType == Chess::Piece::Knight) {
          static const int offsets[8][2] = {{-2, -1}, {-2, 1}, {-1, -2},
                                            {-1, 2},  {1, -2}, {1, 2},
                                            {2, -1},  {2, 1}};
          for (auto &off : offsets)
            targets.push_back({f1 + off[0], r1 + off[1]});
        } else if (pieceType == Chess::Piece::King) {
          static const int offsets[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                                            {0, 1},   {1, -1}, {1, 0},  {1, 1}};
          for (auto &off : offsets)
            targets.push_back({f1 + off[0], r1 + off[1]});
          if (f1 == 5) {
            targets.push_back({3, r1}); // Queen-side castling
            targets.push_back({7, r1}); // King-side castling
          }
        } else if (pieceType == Chess::Piece::Pawn) {
          int dir = (side == Chess::Side::White) ? 1 : -1;
          targets.push_back({f1, r1 + dir});
          targets.push_back({f1, r1 + 2 * dir});
          targets.push_back({f1 - 1, r1 + dir});
          targets.push_back({f1 + 1, r1 + dir});
        } else {
          // Sliders (Bishop, Rook, Queen)
          int dr[] = {-1, -1, -1, 0, 0, 1, 1, 1};
          int df[] = {-1, 0, 1, -1, 1, -1, 0, 1};
          for (int d = 0; d < 8; ++d) {
            if (pieceType == Chess::Piece::Bishop && (dr[d] == 0 || df[d] == 0))
              continue;
            if (pieceType == Chess::Piece::Rook && (dr[d] != 0 && df[d] != 0))
              continue;
            for (int step = 1; step <= 7; ++step) {
              int nr = r1 + step * dr[d];
              int nf = f1 + step * df[d];
              if (nr < 1 || nr > 8 || nf < 1 || nf > 8)
                break;
              targets.push_back({nf, nr});
              Chess::Square blockSq =
                  board.getSquare({(Chess::File)nf, (Chess::Rank)nr});
              if (blockSq.getPieceType() != Chess::Piece::Empty)
                break;
            }
          }
        }

        for (auto &target : targets) {
          int f2 = target.first;
          int r2 = target.second;
          if (f2 < 1 || f2 > 8 || r2 < 1 || r2 > 8)
            continue;

          Chess::Square destSq =
              board.getSquare({(Chess::File)f2, (Chess::Rank)r2});
          if (destSq.getPieceSide() == side)
            continue;

          Chess::Move potentialMove(srcSq, destSq);
          Chess::MoveType type =
              Chess::MoveValidator(potentialMove, side, board);

          if (type != Chess::MoveType::Invalid &&
              type != Chess::MoveType::inCheck) {
            validMoves.push_back(potentialMove);
          }
        }
      }
    }
  }
  return validMoves;
}

/**
 * @todo eklenecekler:
 * - vezir çıkma
 * - rok
 * - böyle şeyler işte
 */
int FlameBoth::Bot::fastMoveOrdering(const Chess::Move &move,
                                     const Chess::Board &board) {
  int score = 0;

  Chess::Square targetSquare = board.getSquare(move.getTo().getCoordinate());
  Chess::Square sourceSquare = board.getSquare(move.getFrom().getCoordinate());

  if (targetSquare.getPieceType() != Chess::Piece::Empty) {
    int victimValue = 0;

    switch (targetSquare.getPieceType()) {
    case Chess::Piece::Pawn:
      victimValue = 100;
      break;
    case Chess::Piece::Knight:
      victimValue = 300;
      break;
    case Chess::Piece::Bishop:
      victimValue = 300;
      break;
    case Chess::Piece::Rook:
      victimValue = 500;
      break;
    case Chess::Piece::Queen:
      victimValue = 900;
      break;
    case Chess::Piece::King:
      victimValue = 2000;
      break;
    default:
      break;
    }

    int aggressorValue = 0;
    switch (sourceSquare.getPieceType()) {
    case Chess::Piece::Pawn:
      aggressorValue = 100;
      break;
    case Chess::Piece::Knight:
      aggressorValue = 300;
      break;
    case Chess::Piece::Bishop:
      aggressorValue = 300;
      break;
    case Chess::Piece::Rook:
      aggressorValue = 500;
      break;
    case Chess::Piece::Queen:
      aggressorValue = 900;
      break;
    case Chess::Piece::King:
      aggressorValue = 2000;
      break;
    default:
      break;
    }

    score = (victimValue * 10) - aggressorValue;

    score += 10000;
  }
  return score;
}
