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
    m_auto_repeat_r(0.2s, 0.001s),
    m_auto_repeat_l(0.2s, 0.001s),
    m_state(10, 40),
    m_mino(),
    m_level(1),
    m_hold(0),
    m_last_tspin(0),
    m_is_btb(false),
    m_score(0),
    m_score_font(32),
    m_gameover(false),
    m_ren_count(0),
    m_ren_max(0)
{
    m_generate_next();
    
    TetriMino next_mino(m_next.front());
    m_mino = next_mino;
    m_next.pop_front();
}

void Tetris::update() {
    update(0, UseKeyBoard::Yes);
}


void Tetris::update(int32 action, UseKeyBoard keybord){
    if(not m_gameover){
        if(m_next.size() < 7) {
            m_generate_next();
        }
        FMT_MAYBE_UNUSED int32 result = m_update_mino(action, keybord);
    }
}

void Tetris::m_generate_next() {
    Array<int32>next = {1,2,3,4,5,6,7};
    next.shuffle();
    for(auto i : next) {
        m_next << i;
    }
}

int32 Tetris::m_update_mino(int32 action, UseKeyBoard keybord) {
    const double MoveRight = action == 1 or (keybord and ( KeyRight.down() or (m_auto_repeat_r.isOne() and KeyRight.pressed()) ));
    const double MoveLeft = action == 2 or (keybord and ( KeyLeft.down() or (m_auto_repeat_l.isOne() and KeyLeft.pressed()) ));
    const double SoftDrop = action == 3 or (keybord and ( KeyDown.pressed() ));
    const double HardDrop = action == 4 or (keybord and ( KeySpace.down() ));
    const double RotateClock = action == 5 or (keybord and ( (KeyUp | KeyX).down() ));
    const double RotateCntClock = action == 6 or (keybord and ( (KeyZ | KeyControl).down() ));
    const double Hold = action == 7 or ( keybord and ( KeyC.down() ));
    
    double speed = 1 / pow(0.8-((m_level-1)*0.007), m_level-1); // block / second
    if(SoftDrop) speed *= 20;
    int32 prev_y = m_mino.m_Pos.y;
    m_mino.m_Pos.y += speed * Scene::DeltaTime();
    if(m_is_intersect()) {
        --m_mino.m_Pos.y;
        m_mino.m_Pos.y = floor(m_mino.m_Pos.y);
    }
    if(SoftDrop) m_score += floor(m_mino.m_Pos.y) - prev_y;
    
    // move right and left
    if(MoveRight) {
        m_mino.m_Pos.x += 1;
        if(m_is_intersect()) m_mino.m_Pos.x -= 1;
    }
    if(MoveLeft) {
        m_mino.m_Pos.x -= 1;
        if(m_is_intersect()) m_mino.m_Pos.x += 1;
    }
    
    m_auto_repeat_r.update(KeyRight.pressed());
    m_auto_repeat_l.update(KeyLeft.pressed());
    
    if(Hold) {
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
    if(HardDrop) {
        int32 fall_count = 0;
        while(not m_is_onground()) {
            ++m_mino.m_Pos.y;
            ++fall_count;
        }
        
        m_score += 2 * fall_count;
        
        m_settle(); // 設置 + 次のミノ
        return 0;
    }
    
    int32 rotate_point = 0;
    // clockwise
    if(RotateClock) {
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
    if(RotateCntClock) {
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
    if(RotateClock or RotateCntClock) {
        Console << count;
        if(count > 4) tspin_type = 2; // T-spin
        if(count == 4 and rotate_point == 5) tspin_type = 1; // Mini T-spin
    }
    if(tspin_type) {
        m_last_tspin = tspin_type;
    }
    
    
    m_mino.m_lockdown.update(m_is_onground());
    if(MoveRight or MoveLeft or RotateClock or RotateCntClock) {
        m_mino.m_lockdown = Transition(0.5s, 0.001s);
    }
    
    // 落ちる
    if(m_mino.m_lockdown.isOne()) {
        m_settle(); // 設置 + 次のミノ
        
        // ゲームオーバー判定
        if(m_is_intersect()) {
            m_gameover = true;
        }
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
    bool is_gameover = true;
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)m_mino.m_Pos.y;
            int32 tj = j + (int32)m_mino.m_Pos.x;
            if (m_mino.body()[i][j] <= 0) continue;
            if(ti >= 20) is_gameover = false;
            m_state[ti][tj] = m_mino.body()[i][j];
        }
    }
    
    if(is_gameover) {
        m_gameover = true;
    }
    
    int32 erase_count = m_erase();
    
    // スコア計算
    int32 step_score = 0;
    
    if (m_last_tspin == 0 and erase_count < 4) { // 普通に消す
        step_score += Max(0, erase_count * 200 - 100);
    }
    if(m_last_tspin == 1) { // Mini T-spin
        step_score += 100 + 100 * erase_count;
    }
    if(m_last_tspin == 2) { // T-spin
        int32 t = 400 + 400 * erase_count;
        if(m_is_btb) t *= 1.5;
        step_score += t;
    }
    if(erase_count == 4) { // Tetris
        int32 t = 800;
        if(m_is_btb) t *= 1.5;
        step_score += t;
    }
    
    if(erase_count > 0) {
        if(erase_count < 4 and m_last_tspin == 0) m_is_btb = false;
        else m_is_btb = true;
        ++m_ren_count;
        if (m_ren_max < m_ren_count) {
            m_ren_max = m_ren_count;
        }
    }
    else {
        m_ren_count = 0;
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
    
    // ゴースト
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
            Rect(Pos.x + size * tj, Pos.y + size * ti, size).draw(Color(255, 100));
        }
    }
    
    // 操作ミノ
    for(auto i : step(4)) {
        for(auto j : step(4)) {
            int32 ti = i + (int32)m_mino.m_Pos.y-20;
            int32 tj = j + (int32)m_mino.m_Pos.x;
            if (m_mino.body()[i][j] <= 0) continue;
            Rect(Pos.x + size * tj, Pos.y + size * ti, size).draw(cols[m_mino.body()[i][j]]);
        }
    }

    // 盤面
    for(auto i : step(20)) {
        for(auto j : step(10)) {
            Rect(Pos.x + size * j, Pos.y + size * i, size).draw(cols[m_state[i+20][j]]).drawFrame(1,0,Palette::White.withAlpha(80));
        }
    }
    Rect(Pos.x, Pos.y, size * 10, size * 20).drawFrame(0, 3, Palette::White);
    
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

bool Tetris::gameover() {
    return m_gameover;
}

void Tetris::reset() {
    m_state.fill(0);
    m_level = 1;
    m_hold = 0;
    m_last_tspin = 0;
    m_is_btb = 0;
    m_score = 0;
    m_gameover = false;
    m_next.clear();
    m_ren_count = 0;
    m_ren_max = 0;
    
    m_generate_next();
        
    TetriMino next_mino(m_next.front());
    m_mino = next_mino;
    m_next.pop_front();
}

int32 Tetris::ren_now() {
    return m_ren_count;
}

int32 Tetris::ren_max() {
    return m_ren_max;
}

Grid<int32>Tetris::get_state() {
    return m_state;
}

};
