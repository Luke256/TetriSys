//
//  Tetris.cpp
//  Tetris
//
//  Created by Luke on 2022/02/21.
//

#include "Tetris.hpp"

namespace TetriSys{

// Tetrimino
TetriMino::TetriMino():
    m_type(0),
    m_face(0),
    m_lockdown(0.5s, 0.001s),
    m_Pos(3, 18)
{
    
}

TetriMino::TetriMino(int32 type):
    m_type(type),
    m_face(0),
    m_lockdown(0.5s, 0.001s),
    m_Pos(3, 18)
{
    
}

Grid<int32>TetriMino::body() {
    return SHAPE[m_type-1][m_face];
}

TetriMino& TetriMino::operator=(TetriMino &x) {
    m_Pos = x.m_Pos;
    m_face = x.m_face;
    m_type = x.m_type;
    m_lockdown = x.m_lockdown;
    return *this;
}

void TetriMino::clockwise() {
    ++m_face;
    m_face %= 4;
}
void TetriMino::count_clockwise() {
    --m_face;
    m_face += 4;
    m_face %= 4;
}

// Tetris
Tetris::Tetris():
    m_auto_repeat(0.2s, 0.001s),
    m_state(10, 40),
    m_mino(),
    m_level(1),
    m_hold(0),
    m_last_tspin(0),
    m_is_btb(false),
    m_score(0),
    m_score_font(32)
{
    m_generate_next();
    
    TetriMino next_mino(m_next.front());
    m_mino = next_mino;
    m_next.pop_front();
}

void Tetris::update() {
    if(m_next.size() < 7) {
        m_generate_next();
    }
    FMT_MAYBE_UNUSED int32 result = m_update_mino();
}

void Tetris::m_generate_next() {
    Array<int32>next = {1,2,3,4,5,6,7};
    next.shuffle();
    for(auto i : next) {
        m_next << i;
    }
}

int32 Tetris::m_update_mino() {
    double speed = 1 / pow(0.8-((m_level-1)*0.007), m_level-1); // block / second
    if(KeyDown.pressed()) speed *= 20;
    m_mino.m_Pos.y += speed * Scene::DeltaTime();
    if(m_is_intersect()) {
        --m_mino.m_Pos.y;
        m_mino.m_Pos.y = floor(m_mino.m_Pos.y);
    }
    
    // move right and left
    if(KeyRight.down() or (m_auto_repeat.isOne() and KeyRight.pressed())) {
        m_mino.m_Pos.x += 1;
        if(m_is_intersect()) m_mino.m_Pos.x -= 1;
    }
    if(KeyLeft.down() or (m_auto_repeat.isOne() and KeyLeft.pressed())) {
        m_mino.m_Pos.x -= 1;
        if(m_is_intersect()) m_mino.m_Pos.x += 1;
    }
    
    m_auto_repeat.update((KeyLeft|KeyRight).pressed());
    
    if((KeyShift | KeyC).down()) {
        if(m_hold == 0) {
            m_hold = m_mino.m_type;
            TetriMino next_mino(m_next.front());
            m_mino = next_mino;
            m_next.pop_front();
        }
        else {
            int32 holded = m_hold;
            m_hold = m_mino.m_type;
            TetriMino next_mino(holded);
            m_mino = next_mino;
        }
        m_last_tspin = 0;
        return 0;
    }
    
    // ハードドロップ
    if(KeySpace.down()) {
        while(not m_is_onground()) ++m_mino.m_Pos.y;
        
        m_settle(); // 設置 + 次のミノ
        return 0;
    }
    
    int32 rotate_point = 0;
    // clockwise
    if((KeyUp | KeyX).down()) {
        Vec2 base = m_mino.m_Pos;
        int32 prev_face = m_mino.m_face;
        m_mino.clockwise();
        if(m_mino.m_type == 1) { // O
            if(m_is_intersect()) {
                m_mino.count_clockwise();
            }
        }
        else { // others
            bool ok = false;
            for(auto move : m_SRD[m_mino.m_type==2 ? 0 : 2][prev_face]) {
                ++rotate_point;
                m_mino.m_Pos += move;
                if(not m_is_intersect()) {
                    ok = true;
                    break;
                }
                m_mino.m_Pos = base;
            }
            if(not ok) {
                m_mino.count_clockwise();
            }
        }
    }
    if((KeyZ | KeyControl).down()) {
        Vec2 base = m_mino.m_Pos;
        int32 prev_face = m_mino.m_face;
        m_mino.count_clockwise();
        // 回転入れ
        if(m_mino.m_type == 1) { // O
            if(m_is_intersect()) {
                m_mino.clockwise();
            }
        }
        else { // others
            bool ok = false;
            for(auto move : m_SRD[m_mino.m_type==2 ? 1 : 3][prev_face]) {
                ++rotate_point;
                m_mino.m_Pos += move;
                if(not m_is_intersect()) {
                    ok = true;
                    break;
                }
                m_mino.m_Pos = base;
            }
            if(not ok) {
                m_mino.clockwise();
            }
        }
    }
    
    // T-spin判定
    int32 count = 0;
    int32 tspin_type = 0;
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)m_mino.m_Pos.y;
            int32 tj = j + (int32)m_mino.m_Pos.x;
            if (m_mino.body()[i][j] >= 0) continue;
            if (not InRange(ti, 0, 39) or not InRange(tj, 0, 9)) {
                count += -m_mino.body()[i][j];
                continue;
            }
            if (m_state[ti][tj] > 0) {
                count += -m_mino.body()[i][j];
            }
        }
    }
    if((KeyUp|KeyZ|KeyX|KeyControl).down()) {
        Console << count;
        if(count > 4) tspin_type = 2; // T-spin
        if(count == 4 and rotate_point == 5) tspin_type = 1; // Mini T-spin
    }
    if(tspin_type) {
        m_last_tspin = tspin_type;
        Console << U"t-spin";
    }
    
    
    m_mino.m_lockdown.update(m_is_onground());
    if((KeyControl | KeyZ | KeyX | KeyRight | KeyLeft | KeyUp).pressed()) {
        m_mino.m_lockdown = Transition(0.5s, 0.001s);
    }
    
    // 落ちる
    if(m_mino.m_lockdown.isOne()) {
        m_settle(); // 設置 + 次のミノ
        
        // ゲームオーバー判定
        if(m_is_intersect()) return -1;
        else return 0;
    }
    
    return 0;
}

bool Tetris::m_is_intersect() {
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)m_mino.m_Pos.y;
            int32 tj = j + (int32)m_mino.m_Pos.x;
            if (m_mino.body()[i][j] <= 0) continue;
            if (not InRange(ti, 0, 39) or not InRange(tj, 0, 9)) return true;
            if (m_state[ti][tj] > 0) return true;
        }
    }
    return false;
}

bool Tetris::m_is_onground() {
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)m_mino.m_Pos.y+1;
            int32 tj = j + (int32)m_mino.m_Pos.x;
            if (m_mino.body()[i][j] <= 0) continue;
            if (not InRange(ti, 0, 39) or not InRange(tj, 0, 9)) return true;
            if (m_state[ti][tj] > 0) return true;
        }
    }
    return false;
}

void Tetris::m_settle() {
    // 設置
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)m_mino.m_Pos.y;
            int32 tj = j + (int32)m_mino.m_Pos.x;
            if (m_mino.body()[i][j] <= 0) continue;
            m_state[ti][tj] = m_mino.body()[i][j];
        }
    }
    
    int32 erase_count = m_erase();
    
    // スコア計算
    int32 step_score = 0;
    
    if (m_last_tspin == 0 and erase_count < 4) { // 普通に消す
        step_score += Max(0, erase_count * 200 - 100);
    }
    if(m_last_tspin == 1) { // Mini T-spin
        step_score += 100 + 100 * erase_count;
        Console << U"Mini T-spin";
    }
    if(m_last_tspin == 2) { // T-spin
        int32 t = 400 + 400 * erase_count;
        Console << U"T-spin";
        if(m_is_btb) t *= 1.5;
        step_score += t;
    }
    if(erase_count == 4) { // Tetris
        Console << U"Tetris";
        int32 t = 800;
        if(m_is_btb) t *= 1.5;
        step_score += t;
    }
    
    if(erase_count > 0) {
        if(erase_count < 4 and m_last_tspin == 0) m_is_btb = false;
        else m_is_btb = true;
    }
    
    m_score += step_score;
    m_last_tspin = 0;
    
    TetriMino next_mino(m_next.front());
    m_mino = next_mino;
    m_next.pop_front(); // 再生成して次に
}

int32 Tetris::m_erase() {
    int32 bottom = 39;
    int32 count = 0;
    for(int t : step(40)) {
        int32 i = 39-t;
        bool empty = true;
        bool filled = true;
        for(int j : step(10)) {
            if(m_state[i][j]) empty = false;
            else filled = false;
        }
        if(not empty and not filled) {
            for(int j : step(10)) {
                m_state[bottom][j] = m_state[i][j];
                if(bottom != i) m_state[i][j] = 0;
            }
            --bottom;
        }
        else if(filled) {
            for(int j : step(10)) {
                m_state[i][j] = 0;
            }
            ++count;
        }
    }
    return count;
}

void Tetris::draw() {
    const Array<Color>cols = {
        Color(0,0,0,0),
        Palette::Gold,
        Palette::Lightblue,
        Palette::Purple,
        Palette::Orange,
        Palette::Blue,
        Palette::Green,
        Palette::Red
    };
    const Vec2 Pos(200, 100);
    const int32 size = 20;

    for(auto i : step(20)) {
        for(auto j : step(10)) {
            Rect(Pos.x + size * j, Pos.y + size * i, size).draw(cols[m_state[i+20][j]]).drawFrame(1,0,Palette::White.withAlpha(80));
        }
    }
    Rect(Pos.x, Pos.y, size * 10, size * 20).drawFrame(0, 3, Palette::White);
    
    // 操作ミノ
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)m_mino.m_Pos.y-20;
            int32 tj = j + (int32)m_mino.m_Pos.x;
            if (m_mino.body()[i][j] <= 0) continue;
            Rect(Pos.x + size * tj, Pos.y + size * ti, size).draw(cols[m_mino.body()[i][j]]);
        }
    }
    
    TetriMino ghost = m_mino;
    while(not m_is_onground()) ++m_mino.m_Pos.y;
    auto t = ghost;
    ghost = m_mino;
    m_mino = t;
    
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)ghost.m_Pos.y-20;
            int32 tj = j + (int32)ghost.m_Pos.x;
            if (ghost.body()[i][j] <= 0) continue;
            Rect(Pos.x + size * tj, Pos.y + size * ti, size).draw(cols[ghost.body()[i][j]].withAlpha(100));
        }
    }
    
    // Hold
    if(m_hold != 0) {
        for(auto i : step(4)) {
            for(auto j : step(4)) {
                if (SHAPE[m_hold-1][i][j] <= 0) continue;
                Rect(Pos.x + size * j - size * 5, Pos.y + size * i, size).draw(cols[SHAPE[m_hold-1][i][j]]);
            }
        }
    }
    
    // Next
    for(auto n : step(7)) {
        for(auto i : step(4)) {
            for(auto j : step(4)) {
                if (SHAPE[m_next[n]-1][i][j] <= 0) continue;
                Rect(Pos.x + size * j + size * 12, Pos.y + size * i + size * 3 * n, size).draw(cols[SHAPE[m_next[n]-1][i][j]]);
            }
        }
    }
    
    // score
    m_score_font(U"{:0>9}"_fmt(m_score)).drawAt(Pos.x+size*5, Pos.y+size*22);
}

int32 Tetris::get_score() {
    return m_score;
}

bool Tetris::is_btb() {
    return m_is_btb;
}

};
