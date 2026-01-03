#include "ChessApp.h"
#include "../Chess/Chess.h" 
#include "../FlameBoth/FlameBoth.h" 
#include <iostream>
#include <thread>
// #include <slint/slint.h>

ChessApp::ChessApp() : ui(AppWindow::create()), selected_square_index(-1), is_game_over(false) 
{
    board = std::make_shared<Chess::Board>();
    bot = std::make_shared<FlameBoth::Bot>(); 
}

void ChessApp::run()
{
    logToTerminal("> GAME STARTED");
    logToTerminal("> TURN: WHITE");
    updateBoardVisuals();
    
    ui->on_square_clicked([this](int index) { this->onSquareClicked(index); });
    ui->on_moveButtonClicked([this](int id) { this->onButtonClicked(id); });

    ui->run();
}

std::string ChessApp::indexToNotation(int index)
{
    int row = index / 8;     
    int col = index % 8;     
    char file = 'a' + col;
    int rank = 8 - row;
    return std::string(1, file) + std::to_string(rank);
}

/**
 * @brief gui daki terminale log vermek için
 * @note sadece bir satır yazdırır. her çağrıldığında önceki yazılan satır silinir
 */
void ChessApp::logToTerminal(std::string message)
{
    ui->set_log_text(slint::SharedString(message));
}

/**
 * @brief daki tahta kordinatlarını Chess::BoardCoordinate formatına çevirir
 */
Chess::BoardCoordinate ChessApp::indexToBoardCoord(int index)
{
    int col = (index % 8) + 1; 
    int row = index / 8;
    int rankVal = 8 - row; 

    /**
     * çok gıcık etti static cast yaptım
     */
    Chess::File file = static_cast<Chess::File>(col); 
    Chess::Rank rank = static_cast<Chess::Rank>(rankVal); 

    return Chess::BoardCoordinate(file, rank);
}

/**
 * @brief Chess::Board board'ı gui da yazdırılacak formata çevirip yazdırır
 */
void ChessApp::updateBoardVisuals()
{
    std::string board_str = board->getBoardVisual(); 
    std::vector<slint::Image> ui_pieces;
    
    for (char c : board_str)
    {
        slint::SharedString path = pieceToImagePath(c);
        
        if (!path.empty()) ui_pieces.push_back(slint::Image::load_from_path(path));
        else ui_pieces.push_back(slint::Image());
    }
    
    auto model = std::make_shared<slint::VectorModel<slint::Image>>(ui_pieces);
    
    ui->set_board_icons(model);
}

// oyun sonu kontrolü
void ChessApp::checkGameState()
{
    // enum türü yazmaya üşendim
    auto state = Chess::getGameState(*board);
    
    if(state == Chess::GameState::Checkmate)
    {
        is_game_over = true;
        std::string winner = (board->getTurn() == Chess::Side::White) ? "BLACK" : "WHITE";
        logToTerminal("CHECKMATE WINNER: " + winner);
        logToTerminal("> FOR RELOAD PRESS RELOAD.");
    }
    else if(state == Chess::GameState::Stalemate)
    {
        is_game_over = true;
        logToTerminal("STEALMATE");
    }
    else if(Chess::isKingInCheck(*board, board->getTurn()))
    {
        logToTerminal("CHECK!");
    }
}

// 
void ChessApp::onSquareClicked(int index)
{
    if(is_game_over) { logToTerminal("GAMEOVER PLEASE RELOAD GAME"); return; }
    if(board->getTurn() == Chess::Side::Black) { logToTerminal("BOT'S TURN PLEASE WAIT"); return; }

    if(selected_square_index == -1)
    {
        selected_square_index = index;
        logToTerminal("> CHOSED ()");
    }
    else
    {
        if(selected_square_index == index)
        {
            selected_square_index = -1;
            logToTerminal("> ABORTED ()");
            return;
        }
    
        // tür : Chess::BoardCoordinate
        auto fromCoord = indexToBoardCoord(selected_square_index);
        auto toCoord = indexToBoardCoord(index);

        Chess::Square fromSquare = board->getSquare(fromCoord);
        Chess::Square toSquare = board->getSquare(toCoord);
        Chess::Move move(fromSquare, toSquare);

        Chess::Side turnBefore = board->getTurn();

        // Hamleyi Yap
        Chess::makeMove(move, board->getTurn(), *board);
        
        if(board->getTurn() != turnBefore) 
        {
            updateBoardVisuals();
            selected_square_index = -1;
            
            checkGameState();
            
            if(!is_game_over) logToTerminal("> MOVE IS SUCCESFULL. BOT'S TURN");
        }
        else 
        {
            logToTerminal("> INVALID MOVE");
            selected_square_index = -1;
        }
    }
}

/**
 * @brief botun hamle yapığı fonksiyon
 * @note multithread yapılıyor bu nedenle bot daha yavaş çalışabilir
 */
void ChessApp::onButtonClicked(int id)
{
    // id 1 : bot çalıştırma butonu
    if(id == 1) // Bot Oyna
    {
        if(is_game_over) { logToTerminal("> GAME OVER"); return; }
        if(board->getTurn() == Chess::Side::White) { logToTerminal("> YOUR TURN"); return; }

        logToTerminal("> BOT IS THINKING PLEASE WAIT (10 SECOND)");
        
        // Multithreading
        std::thread([this]()
        {
            auto botMove = bot->getBestMove(*board, 10); 

            slint::invoke_from_event_loop([this, botMove]()
            {
                Chess::makeMove(botMove, board->getTurn(), *board);
                updateBoardVisuals();
                checkGameState();
                
                if(!is_game_over) { logToTerminal("> BOT PLAYED YOUR TURN"); }
            });
        }).detach();
    }
    else // id 0 : sıfırlama butonu
    {   
        board = std::make_shared<Chess::Board>(); /** < @note board costructor unda direk yeniliyor zaten*/
        is_game_over = false;
        updateBoardVisuals();
        logToTerminal("> GAME RELOADED");
        selected_square_index = -1;
    }
}