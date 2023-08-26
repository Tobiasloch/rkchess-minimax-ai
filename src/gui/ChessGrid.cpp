#include "ChessGrid.h"
#include "sf-svg/SFC/Svg.hpp"
#ifdef SVGIMAGES
    #define FILEEXTENSION ".svg"
#else
    #define FILEEXTENSION ".png"
#endif

const std::string imageFolder = "assets";
const std::string characterToImageFileBasename[] = {
        "wQ",
        "wK",
        "wB",
        "wN",
        "wR",
        "bQ",
        "bK",
        "bB",
        "bN",
        "bR"
    };

std::string getPieceImagePath(int character) {
    return imageFolder + "/" + characterToImageFileBasename[character] + FILEEXTENSION;
}

void ChessGrid::loadTextures() {
    // load the textures
    for (int i = 0; i < 10; ++i) {
        std::string file = getPieceImagePath(i);
        #ifdef SVGIMAGES
            svgImages_[i].loadFromFile(file);
            sf::Image image = svgImages_[i].rasterize(svgFactor_);
            textures_[i].loadFromImage(image);
            std::cout << "texture size: (" << textures_[i].getSize().x << "," << textures_[i].getSize().y << ")" << std::endl;
        #else
            textures_[i].loadFromFile(file);
        #endif
        textures_[i].setSmooth(true);
    }
    // setting the texture of 10 to a blank texture
    textures_[10] = sf::Texture();
}

void ChessGrid::updateTextureScale(int widnowWidth, int windowHeight) {
    for (int i = 0; i < 10; ++i) {
        sf::Image image = svgImages_[i].rasterize(svgFactor_);
        textures_[i].loadFromImage(image);
    }
}


ChessGrid::ChessGrid(
            int x,
            int y,
            int width,
            int height
            )
    : x_(x), y_(y), width_(width), height_(height) {
    parse((char*)INITIAL_RACING_KINGS_FEN, &board_);
    initMinimax();

    // load the textures
    loadTextures();

    // Create the chess grid with alternating white and black tiles
    for (int row = 0; row < getGridSize(); ++row) {
        for (int col = 0; col < getGridSize(); ++col) {
            sf::RectangleShape tile = sf::RectangleShape();
            sf::Color& tileColor = (row + col) % 2 == 0 ? lightColor_ : darkColor_;
            tile.setFillColor(tileColor);

            sf::RectangleShape tileHovering = sf::RectangleShape();
            tileHovering.setFillColor(hoverColor_);
            
            ChessField& field = fields_[row][col];
            field.x_ = x_+margin_ +col * getTileSizeX();
            field.y_ = y_+margin_ +row * getTileSizeY();
            field.width_ = getTileSizeX();
            field.height_ = getTileSizeY();
            field.tile_ = tile;
            field.tileHovering_ = tileHovering;
            field.row_ = row;
            field.col_ = col;
        }
    }

    background_.setPosition(x_, y_);
    background_.setSize(sf::Vector2f(width_, height_));
    background_.setFillColor(backgroundColor_);

    // init field labels which are on the left border(1-8) and on the bottom border (a-h)
    font_.loadFromFile("assets/font.ttf");
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 8; j++) {
            sf::Text& label = fieldLabels_[i][j];
            label.setFont(font_);
            label.setCharacterSize(margin_/2);
            label.setFillColor(fieldLabelColor_);
            label.setStyle(sf::Text::Bold);
            
            if (i == 0) { // left border 
                label.setString(std::to_string(8-j));
                label.setOrigin(label.getLocalBounds().width/2, label.getLocalBounds().height/2);
                label.setPosition(
                    x_ + margin_/2, 
                    y_ + margin_ + getTileSizeY()/2 + j * getTileSizeY()
                );
            } else { // bottom border
                label.setString(std::string(1, 'a'+j));
                label.setOrigin(label.getLocalBounds().width/2, label.getLocalBounds().height/2);
                label.setPosition(
                    x_ + margin_ + getTileSizeX()/2 + j * getTileSizeX(), 
                    (y_+margin_+(getTileSizeY()*getGridSize())) + margin_/2
                );
            }
        }
    }

    // init current player text
    for (int i = 0; i < 2; i++) {
        sf::Text& text = currentPlayerText_[i];
        text.setFont(font_);
        text.setCharacterSize(margin_/2);
        text.setFillColor(fieldLabelColor_);
        text.setStyle(sf::Text::Bold);

        std::string str = "";
        if ((playerTypes_[0] == HUMAN_PLAYER && playerTypes_[1] == AI_PLAYER) ||
            (playerTypes_[0] == AI_PLAYER && playerTypes_[1] == HUMAN_PLAYER)) { // one player against AI
            str.append((playerTypes_[i] == HUMAN_PLAYER) ? "Your Turn" : "AIs Turn");
        } else { // PvP or AI vs AI
            str.append((i == 0) ? "Player 1" : "Player 2");
        }
        str.append((i == 0) ? " (White)" : " (Black)");
        if (playerTypes_[i] == AI_PLAYER) str.append(" calculating");
        text.setString(str);

        text.setOrigin(0, text.getLocalBounds().height/2);
        text.setPosition(
            x_ + margin_ + getTileSizeX()/8, 
            y_+margin_/2
        );
    }

    // init endGameText_
    endGameText_.setCharacterSize(getTileSizeX());
    endGameText_.setFont(font_);
    endGameText_.setFillColor(sf::Color::Red);
    checkForEndGame();

    setBoard(&board_);
}

void ChessGrid::draw(sf::RenderWindow& window) {
    // draw background
    window.draw(background_);

    // Draw the chess grid
    for (int i = 0; i < getGridSize(); ++i) {
        for (int j = 0; j < getGridSize(); ++j) {
            ChessField& field = fields_[i][j];
            
            field.draw(window);
        }
    }

    for (int i = 0; i < 2; i ++) {
        if (board_.player-1 == i) {
            if (playerTypes_[i] == AI_PLAYER) {
                updateCalculatingAnimation(currentPlayerText_[i]);
            }
            window.draw(currentPlayerText_[i]);
        }
        for (int j = 0; j < 8; j++) {
            window.draw(fieldLabels_[i][j]);
        }
    }

    if (isGameOver(&board_)) { // draw endGameText_
        window.draw(endGameText_);
    }
}

void ChessGrid::setBoard(struct board* board) {
    resetActiveFields();
    for (int col = 0; col < getGridSize(); ++col) {
        for (int row = 0; row < getGridSize(); ++row) {
            uint8_t pos = to_position(row, col);
            int character = getField(board, pos);
            setPiece(row, col, character);
        }
    }
}

ChessField* ChessGrid::fieldFromPixels(int x, int y) {
    int col = (x-margin_) / getTileSizeX();
    int row = (y-margin_) / getTileSizeY();

    if (row >= 0 && row < getGridSize() && col >= 0 && col < getGridSize()) {
        return &fields_[row][col];
    }
    return NULL;
}

sf::Vector2i ChessGrid::normalise(sf::Vector2i& vec, const sf::Window& window) {
    return {
        (int)((float)(vec.x-x_) * (float)width_ / (float)window.getSize().x),
        (int)((float)(vec.y-y_) * (float)height_ / (float)window.getSize().y)
    };
}

void ChessGrid::onMouseMoved(sf::Vector2i& mousePos, const sf::Window& window) {
    if (isGameOver(&board_)) return;

    sf::Vector2i pos = normalise(mousePos, window);
    ChessField* field = fieldFromPixels(pos.x, pos.y);
    if (field == NULL) return;

    if (hoveredField_ != NULL) {
        hoveredField_->hovering_ = false;
        hoveredField_ = NULL;
    }
    if (field != NULL) {
        field->hovering_ = true;
        hoveredField_ = field;
    }

}

uint8_t ChessGrid::to_position(int row, int col) {
    return col + ((getGridSize() - row-1) * getGridSize());
}

void ChessGrid::activateField(ChessField& field) {
    struct moveIterator iter;
    moveGen_init(&iter, &board_);
    moveGen_next(&iter);
    activeField_ = &field;

    uint8_t field_pos = to_position(field.row_, field.col_);
    for (;iter.hasActive; moveGen_next(&iter)) {
        if (iter.active.from == field_pos) {
            int row = getGridSize()-1 - (iter.active.to / getGridSize());
            int col = iter.active.to % getGridSize();

            fields_[row][col].active_ = true;
        }
    }
}


void doMoveAI(ChessGrid& grid) {
    struct move m;
    struct board* b = grid.getBoard();
    bestMoveMinimax(b, &m, 9, 5000);
    int character = getField(b, m.from);

    movePlayer(b, &m);

    int from_row = grid.getGridSize()-1 - (m.from / grid.getGridSize());
    int from_col = m.from % grid.getGridSize();

    int to_row = grid.getGridSize()-1 - (m.to / grid.getGridSize());
    int to_col = m.to % grid.getGridSize();

    grid.setPiece(to_row, to_col, character);
    grid.setPiece(from_row, from_col, EMPTYFIELD);

    freeMinimaxMemory(0);
    grid.checkForEndGame();
}

void ChessGrid::updateCalculatingAnimation(sf::Text& text) {
    using namespace std::chrono;
    milliseconds ms = duration_cast< milliseconds >(
        system_clock::now().time_since_epoch()
    );
    if (ms.count() - lastLoadingAnimationExection_ > loadingAnimationSpeed_) {
        lastLoadingAnimationExection_ = ms.count();
    } else {
        return;
    }

    // animate
    if (dots_ > maxDots_) {
        resetCalculatingAnimation();
        dots_ = 0;
    } else {
        std::string str = text.getString();
        str.append(".");
        text.setString(str);
        dots_++;
    }
}

void ChessGrid::resetCalculatingAnimation() {
    for (int i = 0; i < 2; i++) {
        std::string str = currentPlayerText_[i].getString();
        currentPlayerText_[i].setString(str.substr(0, str.find(".")));
    }
}

void ChessGrid::checkForEndGame() {
    if (isGameOver(&board_)) {
        std::string str = "";
        str.append((board_.player == 1) ? "Black" : "White");
        str.append(" wins!");
        endGameText_.setString(str);
        
        endGameText_.setOrigin(endGameText_.getLocalBounds().width/2, endGameText_.getLocalBounds().height/2);
        endGameText_.setPosition(x_+width_/2,y_+height_/2);
    }
}

void ChessGrid::resetActiveFields() {
    for (int col = 0; col < getGridSize(); col++) {
        for (int row = 0; row < getGridSize(); row++) {
            fields_[row][col].active_ = false;
        }
    }
}

void ChessGrid::onMouseClick(sf::Vector2i& mousePos, const sf::Window& window) {
    if (isGameOver(&board_)) return;
    
    sf::Vector2i pos = normalise(mousePos, window);
    ChessField* field = fieldFromPixels(pos.x, pos.y);
    if (field == NULL) return;

    bool active = field->active_;

    // reset old active fields
    resetActiveFields();

    if (playerTypes_[board_.player-1] == HUMAN_PLAYER) {
        if (activeField_ != NULL && active) {
            struct move m;
            m.from = to_position(activeField_->row_, activeField_->col_);
            m.to = to_position(field->row_, field->col_);

            movePlayer(&board_, &m);

            setPiece(field->row_, field->col_, activeField_->character_);
            setPiece(activeField_->row_, activeField_->col_, EMPTYFIELD);

            resetCalculatingAnimation();
            if (playerTypes_[board_.player-1] == AI_PLAYER) {
                std::thread(doMoveAI, std::ref(*this)).detach();
            }
        } else {
            activateField(fields_[field->row_][field->col_]);
        }
        checkForEndGame();
    }
}

void ChessGrid::setPiece(int row, int col, int character) {
    ChessField& field = fields_[row][col];
    field.piece_.setTexture(textures_[character], true);
    field.character_ = character;
}
