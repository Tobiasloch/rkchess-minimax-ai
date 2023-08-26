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
            int height,
            const sf::Color& lightColor,
            const sf::Color& darkColor,
            const sf::Color& hoverColor
            )
    : x_(x), y_(y), width_(width), height_(height), lightColor_(lightColor), darkColor_(darkColor), hoverColor_(hoverColor) {
    parse((char*)INITIAL_RACING_KINGS_FEN, &board_);
    initMinimax();

    // load the textures
    loadTextures();

    // Create the chess grid with alternating white and black tiles
    for (int row = 0; row < getGridSize(); ++row) {
        for (int col = 0; col < getGridSize(); ++col) {
            uint8_t pos = to_position(row, col);
            int character = getField(&board_, pos);

            sf::RectangleShape tile = sf::RectangleShape();
            sf::Color& tileColor = (row + col) % 2 == 0 ? lightColor_ : darkColor_;
            tile.setFillColor(tileColor);

            sf::RectangleShape tileHovering = sf::RectangleShape();
            tileHovering.setFillColor(hoverColor_);
            
            ChessField& field = fields_[row][col];
            field.x_ = x_+col * getTileSizeX();
            field.y_ = y_+row * getTileSizeY();
            field.width_ = getTileSizeX();
            field.height_ = getTileSizeY();
            field.character_ = character;
            field.tile_ = tile;
            field.tileHovering_ = tileHovering;
            field.piece_ = sf::Sprite(textures_[character]);
            field.row_ = row;
            field.col_ = col;
        }
    }

    // updateTextureScale(width, height);
    setBoard(&board_);
}

void ChessGrid::draw(sf::RenderWindow& window) {
    // Draw the chess grid
    for (int i = 0; i < getGridSize(); ++i) {
        for (int j = 0; j < getGridSize(); ++j) {
            ChessField& field = fields_[i][j];
            
            field.draw(window);
        }
    }
}

void ChessGrid::setBoard(struct board* board) {
    for (int col = 0; col < getGridSize(); ++col) {
        for (int row = 0; row < getGridSize(); ++row) {
            uint8_t pos = to_position(row, col);
            int character = getField(board, pos);
            setPiece(row, col, character);
        }
    }
}

ChessField* ChessGrid::fieldFromPixels(int x, int y) {
    int col = x / getTileSizeX();
    int row = y / getTileSizeY();

    if (row >= 0 && row < getGridSize() && col >= 0 && col < getGridSize()) {
        return &fields_[row][col];
    }
    return NULL;
}

sf::Vector2i ChessGrid::normalise(sf::Vector2i& vec, const sf::Window& window) {
    return {
        (float)(vec.x-x_) * (float)width_ / (float)window.getSize().x,
        (float)(vec.y-y_) * (float)height_ / (float)window.getSize().y
    };
}

void ChessGrid::onMouseMoved(sf::Vector2i& mousePos, const sf::Window& window) {
    sf::Vector2i pos = normalise(mousePos, window);
    ChessField* field = fieldFromPixels(pos.x, pos.y);

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
}

void ChessGrid::onMouseClick(sf::Vector2i& mousePos, const sf::Window& window) {
    sf::Vector2i pos = normalise(mousePos, window);
    ChessField* field = fieldFromPixels(pos.x, pos.y);

    bool active = field->active_;

    // reset old active fields
    for (int col = 0; col < getGridSize(); col++) {
        for (int row = 0; row < getGridSize(); row++) {
            fields_[row][col].active_ = false;
        }
    }
    
    if (playerTypes_[board_.player-1] == HUMAN_PLAYER) {
        if (activeField_ != NULL && active) {
            struct move m;
            m.from = to_position(activeField_->row_, activeField_->col_);
            m.to = to_position(field->row_, field->col_);

            movePlayer(&board_, &m);

            setPiece(field->row_, field->col_, activeField_->character_);
            setPiece(activeField_->row_, activeField_->col_, EMPTYFIELD);

            if (playerTypes_[board_.player-1] == AI_PLAYER) {
                doMoveAI(*this);
            }
        } else {
            activateField(fields_[field->row_][field->col_]);
        }
    }
}

void ChessGrid::setPiece(int row, int col, int character) {
    ChessField& field = fields_[row][col];
    // sf::Texture texture = sf::Texture();

    // if (character != EMPTYFIELD) {
    //     sf::Image image = textures_[character].rasterize(10);
    //     texture.loadFromImage(image);
    // }
    field.piece_.setTexture(textures_[character], true);
    field.character_ = character;
}
