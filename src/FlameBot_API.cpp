#include "FlameBot_API.h"

#include "BoardHash/BoardHash.h"
#include "Chess/Chess.h"
#include "FlameBoth/FlameBoth.h"
#include "OpeningBook/OpeningBook.h"

#include <cctype>
#include <cstring>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

// ──────────────────────────────────────────────
//  Global Motor Nesneleri
// ──────────────────────────────────────────────

static Chess::Board g_board;
static FlameBoth::Bot g_bot;
static bool g_initialized = false;
static std::mutex g_engineMutex;
static OpeningBook g_book;
static bool g_bookLoaded = false;

FLAMEBOT_API void FlameBot_LoadBook(const char *path) {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  if (!path)
    return;
  g_book.load(path);
  g_bookLoaded = true;
}

// ──────────────────────────────────────────────
//  Yardımcı Fonksiyonlar
// ──────────────────────────────────────────────

static int writeString(const std::string &src, char *outBuf, int bufSize) {
  if (!outBuf || bufSize <= 0)
    return 0;
  int len = (int)src.length();
  if (len >= bufSize)
    len = bufSize - 1;
  memcpy(outBuf, src.c_str(), len);
  outBuf[len] = '\0';
  return len;
}

// ──────────────────────────────────────────────
//  API Implementasyonları
// ──────────────────────────────────────────────

FLAMEBOT_API void FlameBot_Init() {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  if (g_initialized)
    return;
  BoardHash::init();
  g_initialized = true;
}

FLAMEBOT_API void FlameBot_Destroy() {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  // Bellek temizliği (Zobrist table delete)
  if (g_initialized) {
    g_initialized = false;
  }
}

FLAMEBOT_API void FlameBot_NewGame() {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  g_board = Chess::Board();
  BoardHash::clear();
}

FLAMEBOT_API void FlameBot_SetPosition(const char *fen) {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  if (!fen)
    return;
  Chess::Utils::parseFEN(std::string(fen), g_board);
}

FLAMEBOT_API int FlameBot_MakeMove(const char *uciMove) {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  if (!uciMove)
    return 0;
  std::string s(uciMove);
  if (s.length() < 4)
    return 0;

  if (tolower(s[0]) < 'a' || tolower(s[0]) > 'h')
    return 0;
  if (s[1] < '1' || s[1] > '8')
    return 0;
  if (tolower(s[2]) < 'a' || tolower(s[2]) > 'h')
    return 0;
  if (s[3] < '1' || s[3] > '8')
    return 0;

  Chess::BoardCoordinate from = {(Chess::File)(tolower(s[0]) - 'a' + 1),
                                 (Chess::Rank)(s[1] - '0')};
  Chess::BoardCoordinate to = {(Chess::File)(tolower(s[2]) - 'a' + 1),
                               (Chess::Rank)(s[3] - '0')};

  Chess::Square sqFrom = g_board.getSquare(from);
  Chess::Square sqTo = g_board.getSquare(to);
  Chess::Move move(sqFrom, sqTo);

  Chess::Side currentSide = g_board.getTurn();
  Chess::MoveType type = Chess::MoveValidator(move, currentSide, g_board);

  if (type == Chess::MoveType::Invalid || type == Chess::MoveType::inCheck)
    return 0;

  Chess::makeMove(move, currentSide, g_board);
  return 1;
}

FLAMEBOT_API int FlameBot_GetFEN(char *outBuf, int bufSize) {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  // Basit FEN üretici
  std::string fen;

  for (int rank = 8; rank >= 1; --rank) {
    int emptyCount = 0;
    for (int file = 1; file <= 8; ++file) {
      Chess::Square sq =
          g_board.getSquare({(Chess::File)file, (Chess::Rank)rank});
      if (sq.getPieceType() == Chess::Piece::Empty) {
        emptyCount++;
      } else {
        if (emptyCount > 0) {
          fen += std::to_string(emptyCount);
          emptyCount = 0;
        }
        char c = ' ';
        switch (sq.getPieceType()) {
        case Chess::Piece::Pawn:
          c = 'p';
          break;
        case Chess::Piece::Knight:
          c = 'n';
          break;
        case Chess::Piece::Bishop:
          c = 'b';
          break;
        case Chess::Piece::Rook:
          c = 'r';
          break;
        case Chess::Piece::Queen:
          c = 'q';
          break;
        case Chess::Piece::King:
          c = 'k';
          break;
        default:
          break;
        }
        if (sq.getPieceSide() == Chess::Side::White)
          c = toupper(c);
        fen += c;
      }
    }
    if (emptyCount > 0)
      fen += std::to_string(emptyCount);
    if (rank > 1)
      fen += '/';
  }

  fen += (g_board.getTurn() == Chess::Side::White) ? " w " : " b ";

  // Rok hakları
  std::string castleStr;
  if (!g_board.isKingMoved(Chess::Side::White)) {
    if (!g_board.isKingRookMoved(Chess::Side::White))
      castleStr += 'K';
    if (!g_board.isQueenRookMoved(Chess::Side::White))
      castleStr += 'Q';
  }
  if (!g_board.isKingMoved(Chess::Side::Black)) {
    if (!g_board.isKingRookMoved(Chess::Side::Black))
      castleStr += 'k';
    if (!g_board.isQueenRookMoved(Chess::Side::Black))
      castleStr += 'q';
  }
  fen += castleStr.empty() ? "-" : castleStr;
  fen += " - 0 1"; // En passant, halfmove, fullmove basitleştirildi

  return writeString(fen, outBuf, bufSize);
}

FLAMEBOT_API char FlameBot_GetTurn() {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  return (g_board.getTurn() == Chess::Side::White) ? 'w' : 'b';
}

FLAMEBOT_API int FlameBot_GetLegalMoves(const char *square, char *outBuf,
                                        int bufSize) {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  if (!square || strlen(square) < 2)
    return 0;

  int sqFile = tolower(square[0]) - 'a' + 1;
  int sqRank = square[1] - '0';

  std::vector<Chess::Move> allMoves =
      g_bot.getAllValidMoves(g_board, g_board.getTurn());
  std::string result;

  for (const auto &m : allMoves) {
    if ((int)m.getFrom().getCoordinate().file == sqFile &&
        (int)m.getFrom().getCoordinate().rank == sqRank) {
      if (!result.empty())
        result += ' ';
      int tf = (int)m.getTo().getCoordinate().file;
      int tr = (int)m.getTo().getCoordinate().rank;
      result += (char)('a' + tf - 1);
      result += (char)('0' + tr);
    }
  }

  return writeString(result, outBuf, bufSize);
}

FLAMEBOT_API int FlameBot_GetAllLegalMoves(char *outBuf, int bufSize) {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  if (!outBuf || bufSize <= 0)
    return 0;

  Chess::Side currentSide = g_board.getTurn();
  std::vector<Chess::Move> allMoves =
      g_bot.getAllValidMoves(g_board, currentSide);

  std::string result;
  for (const auto &m : allMoves) {
    if (!result.empty())
      result += ' ';
    result += Chess::Utils::moveToString(m);
  }

  return writeString(result, outBuf, bufSize);
}

FLAMEBOT_API int FlameBot_GetBestMove(int moveTimeMs, char *outBuf,
                                      int bufSize) {
  if (!outBuf || bufSize <= 0)
    return 0;

  Chess::Board boardCopy;
  {
    std::lock_guard<std::mutex> lock(g_engineMutex);
    boardCopy = g_board;

    // Kitaba bak
    if (g_bookLoaded) {
      std::string bookMove = g_book.getBookMove(g_board);
      if (!bookMove.empty()) {
        return writeString(bookMove, outBuf, bufSize);
      }
    }
  }

  double timeSeconds = moveTimeMs / 1000.0;
  if (timeSeconds < 0.01)
    timeSeconds = 1.0;

  g_bot.setTimeLimit(timeSeconds);

  Chess::Move bestMove = g_bot.getBestMove(boardCopy, 64);

  std::lock_guard<std::mutex> lock(g_engineMutex);
  std::string result = Chess::Utils::moveToString(bestMove);

  return writeString(result, outBuf, bufSize);
}

FLAMEBOT_API int FlameBot_GetGameState() {
  std::lock_guard<std::mutex> lock(g_engineMutex);
  Chess::GameState state = Chess::getGameState(g_board);
  if (state == Chess::GameState::Checkmate)
    return 1;
  if (state == Chess::GameState::Stalemate)
    return 2;
  return 0; // Ongoing
}
