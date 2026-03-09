#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "BoardHash/BoardHash.h"
#include "Chess/Chess.h"
#include "FlameBoth/FlameBoth.h"
#include "OpeningBook/OpeningBook.h"

// ──────────────────────────────────────────────
//  UCI Ana Döngüsü
// ──────────────────────────────────────────────

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
      BoardHash::clear();
    }

    // ─── position ───
    else if (token == "position") {
      ss >> token;

      if (token == "startpos") {
        Chess::Utils::parseFEN(
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", board);
      } else if (token == "fen") {
        std::string fen;
        for (int i = 0; i < 6; ++i) {
          std::string s;
          if (!(ss >> s))
            break;
          fen += s + " ";
        }
        Chess::Utils::parseFEN(fen, board);
      }

      // moves kısmını işle
      while (ss >> token) {
        if (token == "moves")
          continue;
        Chess::Move m;
        if (Chess::Utils::parseCoordinate(token, m, board)) {
          Chess::makeMove(m, board.getTurn(), board);
        }
      }
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

      double timeToThink = -1.0; // unconstrained time limit
      // Zaman kontrolüne göre derinlik ve süre ayarla
      if (movetime > 0) {
        timeToThink = (movetime - 50) / 1000.0; // 50ms margin
        if (timeToThink < 0.01)
          timeToThink = 0.01;
        depth = 64; // Max depth, Time management will stop it
      } else if (wtime > 0 || btime > 0) {
        int myTime = (board.getTurn() == Chess::Side::White) ? wtime : btime;
        int myInc = (board.getTurn() == Chess::Side::White) ? winc : binc;

        // Basit zaman yönetimi: kalan sürenin 1/30'u + artış
        double thinkTimeMs = myTime / 30.0 + (myInc * 0.8);
        if (thinkTimeMs > myTime - 100)
          thinkTimeMs = myTime - 100; // never exceed total time - 100ms
        if (thinkTimeMs < 10)
          thinkTimeMs = 10;

        timeToThink = thinkTimeMs / 1000.0;
        depth = 64; // Max depth, Time management will stop it
      }

      // Derinliği makul sınırlarda tut
      if (depth < 1)
        depth = 1;

      bot.setTimeLimit(timeToThink);

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
      std::cout << "bestmove " << Chess::Utils::moveToString(bestMove)
                << std::endl;
    }

    // ─── legalmoves ───
    else if (token == "legalmoves") {
      std::string squareStr;
      Chess::Side currentSide = board.getTurn();
      std::vector<Chess::Move> allMoves =
          bot.getAllValidMoves(board, currentSide);

      if (ss >> squareStr) {
        // Belirli bir kare için: legalmoves e2 → o karedeki taşın hedefleri
        if (squareStr.length() >= 2 && tolower(squareStr[0]) >= 'a' &&
            tolower(squareStr[0]) <= 'h' && squareStr[1] >= '1' &&
            squareStr[1] <= '8') {

          int sqFile = tolower(squareStr[0]) - 'a' + 1;
          int sqRank = squareStr[1] - '0';

          std::cout << "legalmoves " << squareStr;
          for (const auto &m : allMoves) {
            int mf = (int)m.getFrom().getCoordinate().file;
            int mr = (int)m.getFrom().getCoordinate().rank;
            if (mf == sqFile && mr == sqRank) {
              // Hedef kareyi yaz
              int tf = (int)m.getTo().getCoordinate().file;
              int tr = (int)m.getTo().getCoordinate().rank;
              std::string target = "";
              target += (char)('a' + tf - 1);
              target += (char)('0' + tr);
              std::cout << " " << target;
            }
          }
          std::cout << std::endl;
        } else {
          std::cout << "legalmoves none" << std::endl;
        }
      } else {
        // Kare belirtilmedi: tüm legal hamleleri listele
        std::cout << "legalmoves";
        for (const auto &m : allMoves) {
          std::cout << " " << Chess::Utils::moveToString(m);
        }
        std::cout << std::endl;
      }
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

  BoardHash::init();

  uci_loop();
  return 0;
}