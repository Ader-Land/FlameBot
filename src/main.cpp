#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


#include "Chess/Chess.h"
#include "FlameBoth/FlameBoth.h"
#include "OpeningBook/OpeningBook.h"


// ──────────────────────────────────────────────
//  Yardımcı fonksiyonlar
// ──────────────────────────────────────────────

Chess::Piece charToPiece(char c) {
  switch (tolower(c)) {
  case 'p':
    return Chess::Piece::Pawn;
  case 'n':
    return Chess::Piece::Knight;
  case 'b':
    return Chess::Piece::Bishop;
  case 'r':
    return Chess::Piece::Rook;
  case 'q':
    return Chess::Piece::Queen;
  case 'k':
    return Chess::Piece::King;
  default:
    return Chess::Piece::Empty;
  }
}

/// FEN string -> Board
void parseFEN(const std::string &fen, Chess::Board &board) {
  // Boş board oluştur: tüm kareleri temizle
  for (int f = 1; f <= 8; ++f)
    for (int r = 1; r <= 8; ++r)
      board.setSquare(
          Chess::Square(Chess::BoardCoordinate((Chess::File)f, (Chess::Rank)r),
                        Chess::Piece::Empty, Chess::Side::None));

  std::istringstream ss(fen);
  std::string piecePlacement, activeColor, castling, enPassant;
  ss >> piecePlacement >> activeColor >> castling >> enPassant;

  // 1) Taş yerleşimi
  int rank = 8, file = 1;
  for (char c : piecePlacement) {
    if (c == '/') {
      rank--;
      file = 1;
    } else if (c >= '1' && c <= '8') {
      file += (c - '0');
    } else {
      Chess::Side side = isupper(c) ? Chess::Side::White : Chess::Side::Black;
      Chess::Piece piece = charToPiece(c);
      Chess::BoardCoordinate coord((Chess::File)file, (Chess::Rank)rank);
      board.setSquare(Chess::Square(coord, piece, side));
      file++;
    }
  }

  // 2) Aktif renk — Board default olarak White ile başlar
  if (activeColor == "b")
    board.passTurn();

  // 3) Rok hakları
  board.setKingMoved(Chess::Side::White, true);
  board.setKingRookMoved(Chess::Side::White, true);
  board.setQueenRookMoved(Chess::Side::White, true);
  board.setKingMoved(Chess::Side::Black, true);
  board.setKingRookMoved(Chess::Side::Black, true);
  board.setQueenRookMoved(Chess::Side::Black, true);

  if (castling != "-") {
    for (char c : castling) {
      switch (c) {
      case 'K':
        board.setKingMoved(Chess::Side::White, false);
        board.setKingRookMoved(Chess::Side::White, false);
        break;
      case 'Q':
        board.setKingMoved(Chess::Side::White, false);
        board.setQueenRookMoved(Chess::Side::White, false);
        break;
      case 'k':
        board.setKingMoved(Chess::Side::Black, false);
        board.setKingRookMoved(Chess::Side::Black, false);
        break;
      case 'q':
        board.setKingMoved(Chess::Side::Black, false);
        board.setQueenRookMoved(Chess::Side::Black, false);
        break;
      }
    }
  }

  // 4) En passant hedefi
  if (enPassant != "-" && enPassant.length() >= 2) {
    Chess::File epFile = (Chess::File)(tolower(enPassant[0]) - 'a' + 1);
    Chess::Rank epRank = (Chess::Rank)(enPassant[1] - '0');
    board.setEnPassantTarget(Chess::BoardCoordinate(epFile, epRank));
  } else {
    board.clearEnPassantTarget();
  }
}

/// UCI koordinat notasyonunu parse et (ör. "e2e4", "e7e8q")
bool parseCoordinate(const std::string &s, Chess::Move &outMove,
                     Chess::Board &board) {
  if (s.length() < 4)
    return false;

  if (tolower(s[0]) < 'a' || tolower(s[0]) > 'h')
    return false;
  if (s[1] < '1' || s[1] > '8')
    return false;
  if (tolower(s[2]) < 'a' || tolower(s[2]) > 'h')
    return false;
  if (s[3] < '1' || s[3] > '8')
    return false;

  Chess::BoardCoordinate coord1 = {(Chess::File)(tolower(s[0]) - 'a' + 1),
                                   (Chess::Rank)(s[1] - '0')};
  Chess::BoardCoordinate coord2 = {(Chess::File)(tolower(s[2]) - 'a' + 1),
                                   (Chess::Rank)(s[3] - '0')};

  Chess::Square sqFrom(board.getSquare(coord1));
  Chess::Square sqTo(board.getSquare(coord2));

  outMove = {sqFrom, sqTo};

  return true;
}

/// Move -> UCI string (ör. "e2e4", "e7e8q")
std::string moveToString(const Chess::Move &move) {
  if (move.getFrom().getPieceType() == Chess::Piece::Empty)
    return "0000";

  int f1 = (int)move.getFrom().getCoordinate().file;
  int r1 = (int)move.getFrom().getCoordinate().rank;
  int f2 = (int)move.getTo().getCoordinate().file;
  int r2 = (int)move.getTo().getCoordinate().rank;

  std::string s = "";
  s += (char)('a' + f1 - 1);
  s += (char)('0' + r1);
  s += (char)('a' + f2 - 1);
  s += (char)('0' + r2);

  bool isPawn = (move.getFrom().getPieceType() == Chess::Piece::Pawn);

  // Piyon terfisi (varsayılan: vezir)
  if (isPawn && r2 == 8)
    s += 'q';
  else if (isPawn && r2 == 1)
    s += 'q';

  return s;
}

// ──────────────────────────────────────────────
//  UCI Ana Döngüsü
// ──────────────────────────────────────────────

void uci_loop() {
  Chess::Board board;
  FlameBoth::Bot bot;
  Chess::Move move;

  OpeningBook openingBook;
  bool bookLoaded = false;

  std::string line, token;

  while (std::getline(std::cin, line)) {
    // Satırın sonundaki \r'yi temizle (Windows uyumluluğu)
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    if (line.empty())
      continue;

    std::istringstream ss(line);
    ss >> token;

    // ─── uci ───
    if (token == "uci") {
      std::cout << "id name FlameBot v0.9.1" << std::endl;
      std::cout << "id author Isa" << std::endl;
      std::cout
          << "option name Move Overhead type spin default 10 min 0 max 5000"
          << std::endl;
      std::cout << "option name Threads type spin default 1 min 1 max 512"
                << std::endl;
      std::cout << "option name Hash type spin default 16 min 1 max 33554432"
                << std::endl;
      std::cout << "option name OwnBook type check default false" << std::endl;
      std::cout << "option name BookFile type string default <empty>"
                << std::endl;
      std::cout << "uciok" << std::endl;
    }

    // ─── isready ───
    else if (token == "isready") {
      std::cout << "readyok" << std::endl;
    }

    // ─── setoption ───
    else if (token == "setoption") {
      // setoption name <name> value <value>
      std::string nameToken, optName, valueToken, optValue;
      ss >> nameToken; // "name"

      // İsim birden fazla kelimeden oluşabilir
      optName = "";
      while (ss >> token) {
        if (token == "value")
          break;
        if (!optName.empty())
          optName += " ";
        optName += token;
      }

      // Değer
      optValue = "";
      while (ss >> token) {
        if (!optValue.empty())
          optValue += " ";
        optValue += token;
      }

      if (optName == "BookFile" && !optValue.empty() && optValue != "<empty>") {
        openingBook.load(optValue);
        bookLoaded = true;
      } else if (optName == "OwnBook") {
        // OwnBook true/false — şimdilik sadece not ediyoruz
      }
    }

    // ─── ucinewgame ───
    else if (token == "ucinewgame") {
      board = Chess::Board();
    }

    // ─── position ───
    else if (token == "position") {
      ss >> token;

      if (token == "startpos") {
        board = Chess::Board();
      } else if (token == "fen") {
        // "position fen <fen_string> [moves ...]"
        // FEN 6 alandan oluşur ancak biz ilk 4'ünü kullanıyoruz
        std::string fenStr = "";
        int fenParts = 0;
        while (ss >> token && token != "moves") {
          if (!fenStr.empty())
            fenStr += " ";
          fenStr += token;
          fenParts++;
          if (fenParts >= 6)
            break;
        }

        board = Chess::Board(); // Tahtayı sıfırla
        parseFEN(fenStr, board);

        // Eğer "moves" token'ını son while'da aldıysak, buradan devam
        // Eğer fenParts >= 6 ise "moves" henüz okunmamış olabilir
        if (token != "moves" && fenParts >= 6) {
          // "moves" keyword'ünü okumaya çalış
          if (!(ss >> token) || token != "moves")
            continue; // moves yok, position tamamlandı
        } else if (token != "moves") {
          continue; // moves yok
        }

        // Hamleleri uygula
        while (ss >> token)
          if (parseCoordinate(token, move, board))
            Chess::makeMove(move, board.getTurn(), board);

        continue; // Ana döngüye dön
      }

      // "moves" bölümünü oku (startpos durumu)
      while (ss >> token)
        if (token == "moves")
          break;

      while (ss >> token)
        if (parseCoordinate(token, move, board))
          Chess::makeMove(move, board.getTurn(), board);
    }

    // ─── go ───
    else if (token == "go") {
      int depth = 8; // Varsayılan arama derinliği
      int wtime = 0, btime = 0, winc = 0, binc = 0, movetime = 0;
      bool infinite = false;

      // go parametrelerini ayrıştır
      while (ss >> token) {
        if (token == "depth") {
          ss >> depth;
        } else if (token == "wtime") {
          ss >> wtime;
        } else if (token == "btime") {
          ss >> btime;
        } else if (token == "winc") {
          ss >> winc;
        } else if (token == "binc") {
          ss >> binc;
        } else if (token == "movetime") {
          ss >> movetime;
        } else if (token == "infinite") {
          infinite = true;
          depth = 20;
        }
      }

      // Zaman kontrolüne göre derinlik ayarla
      if (movetime > 0) {
        // movetime verilmişse basit derinlik tahmini
        if (movetime < 500)
          depth = 4;
        else if (movetime < 2000)
          depth = 6;
        else if (movetime < 5000)
          depth = 8;
        else
          depth = 10;
      } else if (wtime > 0 || btime > 0) {
        int myTime = (board.getTurn() == Chess::Side::White) ? wtime : btime;
        int myInc = (board.getTurn() == Chess::Side::White) ? winc : binc;

        // Basit zaman yönetimi: kalan süreye göre derinlik
        int thinkTime = myTime / 30 + myInc;
        if (thinkTime < 500)
          depth = 4;
        else if (thinkTime < 2000)
          depth = 6;
        else if (thinkTime < 5000)
          depth = 8;
        else if (thinkTime < 15000)
          depth = 10;
        else
          depth = 12;
      }

      // Derinliği makul sınırlarda tut
      if (depth < 1)
        depth = 1;
      if (depth > 20)
        depth = 20;

      // Açılış kitabından hamle dene
      if (bookLoaded) {
        std::string bookMove = openingBook.getBookMove(board);
        if (!bookMove.empty()) {
          std::cout << "bestmove " << bookMove << std::endl;
          continue;
        }
      }

      // Motor ile en iyi hamleyi bul
      Chess::Move bestMove = bot.getBestMove(board, depth);
      std::cout << "bestmove " << moveToString(bestMove) << std::endl;
    }

    // ─── stop ───
    else if (token == "stop") {
      // Şu anda engelli arama yok, ama GUI uyumluluğu için gerekli
      // İleride iteratif derinleşme eklendiğinde burada arama durdurulacak
    }

    // ─── quit ───
    else if (token == "quit") {
      break;
    }
  }
}

// ──────────────────────────────────────────────
//  Giriş Noktası
// ──────────────────────────────────────────────

int main() {
  // stdout buffer'ını devre dışı bırak (UCI uyumluluğu)
  std::ios_base::sync_with_stdio(false);
  std::cin.tie(nullptr);

  uci_loop();
  return 0;
}