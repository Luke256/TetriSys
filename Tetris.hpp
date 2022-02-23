#pragma once

# include <Siv3D.hpp> // OpenSiv3D v0.6.2

namespace TetriSys {

struct TetriMino {
    const Grid<Grid<int32>>SHAPE = {
        { // O
            {
                {0,1,1,0},
                {0,1,1,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,1,1,0},
                {0,1,1,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,1,1,0},
                {0,1,1,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,1,1,0},
                {0,1,1,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            
        },
        
        { // I
            {
                {0,0,0,0},
                {2,2,2,2},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,0,2,0},
                {0,0,2,0},
                {0,0,2,0},
                {0,0,2,0}
            },
            {
                {0,0,0,0},
                {0,0,0,0},
                {2,2,2,2},
                {0,0,0,0}
            },
            {
                {0,2,0,0},
                {0,2,0,0},
                {0,2,0,0},
                {0,2,0,0}
            }
        },
        
        { // T
            {
                {-2, 3,-2,0},
                { 3, 3, 3,0},
                {-1, 0,-1,0},
                {0,0,0,0}
            },
            {
                {-1, 3,-2,0},
                { 0, 3, 3,0},
                {-1, 3,-2,0},
                {0,0,0,0}
            },
            {
                {-1, 0,-1,0},
                { 3, 3, 3,0},
                {-2, 3,-2,0},
                {0,0,0,0}
            },
            {
                {-2, 3,-1,0},
                { 3, 3, 0,0},
                {-2, 3,-1,0},
                {0,0,0,0}
            }
        },
        
        { // L
            {
                {0,0,4,0},
                {4,4,4,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,4,0,0},
                {0,4,0,0},
                {0,4,4,0},
                {0,0,0,0}
            },
            {
                {0,0,0,0},
                {4,4,4,0},
                {4,0,0,0},
                {0,0,0,0}
            },
            {
                {4,4,0,0},
                {0,4,0,0},
                {0,4,0,0},
                {0,0,0,0}
            }
        },
        
        { // J
            {
                {5,0,0,0},
                {5,5,5,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,5,5,0},
                {0,5,0,0},
                {0,5,0,0},
                {0,0,0,0}
            },
            {
                {0,0,0,0},
                {5,5,5,0},
                {0,0,5,0},
                {0,0,0,0}
            },
            {
                {0,5,0,0},
                {0,5,0,0},
                {5,5,0,0},
                {0,0,0,0}
            }
        },
        
        { // S
            {
                {0,6,6,0},
                {6,6,0,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,6,0,0},
                {0,6,6,0},
                {0,0,6,0},
                {0,0,0,0}
            },
            {
                {0,0,0,0},
                {0,6,6,0},
                {6,6,0,0},
                {0,0,0,0}
            },
            {
                {6,0,0,0},
                {6,6,0,0},
                {0,6,0,0},
                {0,0,0,0}
            }
        },
        
        { // Z
            {
                {7,7,0,0},
                {0,7,7,0},
                {0,0,0,0},
                {0,0,0,0}
            },
            {
                {0,0,7,0},
                {0,7,7,0},
                {0,7,0,0},
                {0,0,0,0}
            },
            {
                {0,0,0,0},
                {7,7,0,0},
                {0,7,7,0},
                {0,0,0,0}
            },
            {
                {0,7,0,0},
                {7,7,0,0},
                {7,0,0,0},
                {0,0,0,0}
            }
        }
    };
    
    int32 m_type;
    int32 m_face;
    Transition m_lockdown;
    Vec2 m_Pos;
    
    TetriMino();
    TetriMino(int32 type);
    Grid<int32>body();
    TetriMino& operator=(TetriMino &x);
    void clockwise();
    void count_clockwise();
};

class Tetris {
    const Grid<Array<Vec2>> m_SRD = {
        {// I
            {{ 0, 0}, {-2, 0}, {+1, 0}, {-2,+1}, {+1,-2}}, // from : north (A)
            {{ 0, 0}, {-1, 0}, {+2, 0}, {-1,-2}, {+2,+1}}, // from : east (B)
            {{ 0, 0}, {+2, 0}, {-1, 0}, {+2,-1}, {-1,+2}}, // from : south (C)
            {{ 0, 0}, {-2, 0}, {+1, 0}, {+1,+2}, {-2,-1}}  // from : west (D)
        },

        {
            {{ 0, 0}, {-1, 0}, {+2, 0}, {-1,-2}, {+2,+1}}, // to : D
            {{ 0, 0}, {+2, 0}, {-1, 0}, {+2,-1}, {-1,+2}}, // to : A
            {{ 0, 0}, {+1, 0}, {-2, 0}, {+1,+2}, {-2,-1}}, // to : B
            {{ 0, 0}, {+1, 0}, {-2, 0}, {-2,+1}, {+1,-2}}  // to : C
        },

        { // other
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}}, // to : AB
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}}, // to : BC
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}}, // to : CD
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}}  // to : DA
        },

        {
            {{ 0, 0}, { 1, 0}, { 1,-1}, { 0, 2}, { 1, 2}}, // to : AD
            {{ 0, 0}, { 1, 0}, { 1, 1}, { 0,-2}, { 1,-2}}, // to : BA
            {{ 0, 0}, {-1, 0}, {-1,-1}, { 0, 2}, {-1, 2}}, // to : CB
            {{ 0, 0}, {-1, 0}, {-1, 1}, { 0,-2}, {-1,-2}}  // to : DC
        }
    };
    const Array<Grid<int32>>SHAPE = {
        { // O
            {0,1,1,0},
            {0,1,1,0},
            {0,0,0,0},
            {0,0,0,0}
            
        },
        
        { // I
            {0,0,0,0},
            {2,2,2,2},
            {0,0,0,0},
            {0,0,0,0}
        },
        { // T
            {0,3,0,0},
            {3,3,3,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        { // L
            {0,0,4,0},
            {4,4,4,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        { // J
            {5,0,0,0},
            {5,5,5,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        { // S
            {0,6,6,0},
            {6,6,0,0},
            {0,0,0,0},
            {0,0,0,0}
        },
        { // Z
            {7,7,0,0},
            {0,7,7,0},
            {0,0,0,0},
            {0,0,0,0}
        }
    };
    
    Transition m_auto_repeat;
    Grid<int32>m_state;
    TetriMino m_mino;
    Array<int32>m_next;
    int32 m_level;
    int32 m_hold;
    int32 m_last_tspin;
    bool m_is_btb;
    int32 m_score;
    Font m_score_font;
    void m_generate_next();
    int32 m_update_mino();
    bool m_is_intersect();
    bool m_is_onground();
    void m_settle();
    int32 m_erase();
public:
    Tetris();
    void update();
    void draw();
    int32 get_score();
    bool is_btb();
};

};

/*
---memo---
TetriMinoは普通のxy座標系で考えていい
 
TetriMino:
 type:
  1:O
  2:I
  3:T
  4:L
  5:J
  6:S
  7:Z
 
 face:
  0:north
  1:east
  2:south
  3:west
*/
