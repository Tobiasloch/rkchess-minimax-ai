#include "boardEvaluation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*

    Milestone 2: Game Stages in Early/Mid/End Game

    Shortcuts:
    - P   = Piece (King, Queen, Rook, Bishop, kNight)
    - P\K = Piece without King
    - PST = Piece Square Table
    
    Early Game:
    - Priority: develop P\K movement
        - Progress positioning for mid game
        - All P\K have key (higher valued, like "nice-to-have-you-more-there") position in PST
        - Kick pieces is more worth
        - King movement result lower score increase

    Mid Game:
    - starts if King reach row 4, also starts automatically in round 6 to force King movement slightly
    - Priority: develop P movement 
        - Still progress positioning P/K but with no real key position in PST (more like "nice-to-have-you-there" position), depends on Early Game
        - King movement is equal to more worth than any other piece movement
        - Kick pieces more worth than any piece movement

    End Game:
    - starts if King reach row 6, also starts automatically in round 12 to force King movement greatly
    - Priority: develop King movement
        - P\K PST uniformly, no "nice-to-have-you-there" position, depends on Mid Game
        - Place Queen, Rook and kNight on upper field to free or block King movement
        - Place Bishop on lower field to block King movement
        - King movement is more worth (+30 > any P\K move)
        - Blocking King movement is more worth than King movement

*/

int EVALUATE_PIECE_INITED = FALSE;

// call this function before evaluateBoard(board* board) !
void initEvaluatePiece(){
    if (EVALUATE_PIECE_INITED) return;
    EVALUATE_PIECE_INITED = TRUE;
    /* King 
        - characteristic move score: 
            - Early Game: 5 * x (+5 for avoiding a/h files)
            - Mid Game: 15 * x (+10 for avoiding a/h files) 
            - End Game: 30 * x (+15 for avoiding a/h files)
    */
    // Material Value (King cannot beaten)
    ep->k.value = 0;

    memcpy( ep->k.pstValue, 
            (int[3][64]){

                // Early Game: King move grants +5 (+5 for avoiding a/h files!)
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   5, 10, 10, 10, 10, 10, 10,  5,
                  10, 15, 15, 15, 15, 15, 15, 10,
                  20, 25, 25, 25, 25, 25, 25, 20,
                  30, 35, 35, 35, 35, 35, 35, 30,
                  40, 45, 45, 45, 45, 45, 45, 40,
                 900,900,900,900,900,900,900,900}, 

                // Mid Game: King move forward grants +15 +(10 for avoiding a/h files!)
                // NOTE: more worth to move King (+15 > moves any other pieces but still worth to kick pieces)
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                  15, 25, 25, 25, 25, 25, 25, 15,
                  30, 40, 40, 40, 40, 40, 40, 30,
                  45, 55, 55, 55, 55, 55, 55, 45,
                  60, 70, 70, 70, 70, 70, 70, 60,
                  75, 85, 85, 85, 85, 85, 85, 75,
                 900,900,900,900,900,900,900,900}, 

                // End Game: King move forward grants +30 (+15 for avoiding a/h files!)
                // NOTE: outscore moves of any other piece (only kick Queen and Rook is more worth)
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                  30, 45, 45, 45, 45, 45, 45, 30,
                  60, 75, 75, 75, 75, 75, 75, 60,
                  90,105,105,105,105,105,105, 90,
                 120,135,135,135,135,135,135,120,
                 150,165,165,165,165,165,165,150,
                 900,900,900,900,900,900,900,900}
            }, 
            sizeof (ep->k.pstValue));
    
    /* Queen
        - characteristic move score: 
            - Early Game: 6, 11, 16
            - Mid Game: 6 , 11 
            - End Game: 11
    */
    // Material Value
    ep->q.value = 70;
 
    memcpy( ep->q.pstValue, 
            (int[3][64]){

                // Early Game: Try to move Queen out the corner and place in center with priority in mid
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   6, 11, 11, 11, 11, 11, 11,  6,
                   6, 11, 16, 16, 16, 16, 11,  6,
                   6, 11, 16, 16, 16, 16, 11,  6,
                   6, 11, 11, 11, 11, 11, 11,  6,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0
                  }, 

                 // Mid Game: Try to move Queen out the corner and place in upper field with priority field
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   6, 11, 11, 11, 11, 11, 11,  6,
                   6, 11, 11, 11, 11, 11, 11,  6,
                   6, 11, 11, 11, 11, 11, 11,  6,
                   6, 11, 11, 11, 11, 11, 11,  6,
                   0,  0,  0,  0,  0,  0,  0,  0
                  }, 

                // End Game: Keep Queen in upper field to control king
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                  11, 11, 11, 11, 11, 11, 11, 11,
                  11, 11, 11, 11, 11, 11, 11, 11,
                  11, 11, 11, 11, 11, 11, 11, 11,
                  11, 11, 11, 11, 11, 11, 11, 11}
            }, 
            sizeof (ep->q.pstValue));

    /* Rook
        - characteristic move score: 
            - Early Game: 7, 12, 17
            - Mid Game: 7, 12
            - End Game: 12
    */
    // Material Value
    ep->r.value = 50;

    memcpy( ep->r.pstValue, 
            (int[3][64]){

                // Early Game: position Rook in upper field, but on players side
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   7, 17, 17, 12, 12, 17, 17,  7,
                   7, 17, 17, 12, 12, 17, 17,  7,
                   7, 17, 17, 12, 12, 17, 17,  7,
                   7,  7,  7,  7,  7,  7,  7,  7,
                   0,  0,  0,  0,  0,  0,  0,  0}, 

                // Mid Game: position Rook in upper field, but on players side with no priority
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   7, 12, 12, 12, 12, 12, 12,  7,
                   7, 12, 12, 12, 12, 12, 12,  7,
                   7, 12, 12, 12, 12, 12, 12,  7,
                   7,  7,  7,  7,  7,  7,  7,  7}, 
        
                // End Game: position Rook in upper field to defend and block king movement
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                  12, 12, 12, 12, 12, 12, 12, 12,
                  12, 12, 12, 12, 12, 12, 12, 12,
                  12, 12, 12, 12, 12, 12, 12, 12,
                  12, 12, 12, 12, 12, 12, 12, 12}
            }, 
            sizeof (ep->r.pstValue));

    /* Bishop
        - characteristic move score: 
            - Early Game: 8, 13, 18
            - Mid Game: 8, 13
            - End Game: 13
    */
    // Material Value
    ep->b.value = 20;

    memcpy( ep->b.pstValue, 
            (int[3][64]){

                // Eary Game: position Bishop in lower field on players side
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   8,  8,  8,  8,  8,  8,  8,  8,
                   8, 18, 18, 13, 13, 18, 18,  8,
                   8, 18, 18, 13, 13, 18, 18,  8,
                   8, 18, 18, 13, 13, 18, 18,  8,
                   8,  8,  8,  8,  8,  8,  8,  8,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0}, 

                // Mid Game: position Bishop in lower field on players side, prepare king block for mid game with no priority
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   8,  8,  8,  8,  8,  8,  8,  8,
                   8, 13, 13, 13, 13, 13, 13,  8,
                   8, 13, 13, 13, 13, 13, 13,  8,
                   8, 13, 13, 13, 13, 13, 13,  8,
                   8,  8,  8,  8,  8,  8,  8,  8,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0}, 

                // End Game: Keep Bishop in lower field on players side to block king movement on the diagonal
                { 13, 13, 13, 13, 13, 13, 13, 13,
                  13, 13, 13, 13, 13, 13, 13, 13,
                  13, 13, 13, 13, 13, 13, 13, 13,
                  13, 13, 13, 13, 13, 13, 13, 13,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0}
            }, 
            sizeof (ep->b.pstValue));
    
    /* kNight
        - characteristic move score: 
            - Early Game: 9, 14, 19, 24
            - Mid Game: 9, 14, 19
            - End Game: 14
    */
    // Material Value
    ep->n.value = 20;

    memcpy( ep->n.pstValue, 
            (int[3][64]){

                // Early Game: position kNight in center for better movement 
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  9, 14, 14,  9,  0,  0,
                   0,  9, 14, 19, 19, 14,  9,  0,
                   0, 14, 19, 24, 24, 19, 14,  0,
                   0, 14, 19, 24, 24, 19, 14,  0,
                   0,  9, 14, 19, 19, 14,  9,  0,
                   0,  0,  9, 14, 14,  9,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0}, 

                // Mid Game: keep pst but with no key position
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  9, 14, 14,  9,  0,  0,
                   0,  9, 14, 19, 19, 14,  9,  0,
                   0, 14, 19, 19, 19, 19, 14,  0,
                   0, 14, 19, 19, 19, 19, 14,  0,
                   0,  9, 14, 19, 19, 14,  9,  0,
                   0,  0,  9, 14, 14,  9,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0}, 
        
                // End Game: position kNight in upper field to defend and block king movement
                {  0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                   0,  0,  0,  0,  0,  0,  0,  0,
                  14, 14, 14, 14, 14, 14, 14, 14,
                  14, 14, 14, 14, 14, 14, 14, 14,
                  14, 14, 14, 14, 14, 14, 14, 14,
                  14, 14, 14, 14, 14, 14, 14, 14}
            }, 
            sizeof (ep->n.pstValue));

}

int getMobilityValue() {

    return 0;
}

// get Pst = Piece Square Value on position p and game stage
int getPstValue(uint8_t p, int8_t character, evaluatePiece* ep, int gameStage) {

    switch(character) {
        case QUEEN: return ep->q.pstValue[gameStage][p];         
        case KING: return ep->k.pstValue[gameStage][p];         
        case BISHOP: return ep->b.pstValue[gameStage][p];         
        case KNIGHT: return ep->n.pstValue[gameStage][p];        
        case ROOK: return ep->r.pstValue[gameStage][p];        
        default: return 0;              
    }      

}

// get Material Value
int getPieceValue(int8_t character, evaluatePiece* ep) {

    switch(character) {
        case QUEEN: return ep->q.value;         
        case KING: return ep->k.value;         
        case BISHOP: return ep->b.value;       
        case KNIGHT: return ep->n.value;       
        case ROOK: return ep->r.value;     
        default: return 0;
    }
}

int kingDifference(board* b) {
    uint64_t myKing = b->board[KING] & b->board[4+b->player];
    uint64_t otherKing = b->board[KING] & b->board[5+(b->player%2)];

    int kingScore = 10;
    return ((bitboardIndexOf(myKing)/8) - (bitboardIndexOf(otherKing)/8))*kingScore;
}

// check if king is on specific rank or above 
int kingOnRank(board* b, int rank) {
    uint64_t myKing = b->board[KING] & b->board[4+b->player];
    uint64_t otherKing = b->board[KING] & b->board[5+(b->player%2)];

    if ((bitboardIndexOf(myKing)/8) >= rank || (bitboardIndexOf(otherKing)/8) >= rank){
        return 1;
    } else {
        return 0;
    }

}

// call this function !
// (black) - <- 0 -> + (white)
int evaluateBoard(board* board) {
    int score = 0;
    uint8_t p;
    int8_t character;
    p = 0;
    character = -1;
    int gameStage = 0;

    int countBits = __builtin_popcountll((board->board[WHITE] || board->board[BLACK]));

    // End Game: starts if King is on rank 6, trigger End Game at Round 12 automatically
    if (board->rounds >= 12 || countBits <= 6) {
        gameStage = 2;
    // Mid Game: starts if King is on rank 4, trigger Mid Game at Round 6 automatically
    } else if (board->rounds >= 6 || countBits <= 10) {
        gameStage = 1;
    }
    
    int bishopCount[2] = {0,0};
    for (int i = 0; i < 5; i++) {
        uint64_t characterBoard = board->board[i];
        
        while(characterBoard != 0) {
            int index = bitboardIndexOf(characterBoard); // get index
            uint64_t iMask = ((uint64_t)1) << index;
            characterBoard = characterBoard ^ iMask; // remove index from characterBoard
            int player = -1; // enemy character
            if (iMask & board->board[4+(board->player)]) player = 1; // my character

            p = index;
            character = i;
            int pieceValue = getPieceValue(character, ep)*player;
            score += pieceValue;
            score += getPstValue(p, character, ep, gameStage)*player;

            if (i == BISHOP) {
                if (player == 1) bishopCount[0]++;
                else bishopCount[1]++;
            }
        }
    }
    if (bishopCount[0] == 2) score += 10;
    if (bishopCount[1] == 2) score += -10;

    return score;
}