# TetriSys
## 概要
C++(OpenSiv3D)用テトリスライブラリ

## 使用方法
`Tetris.cpp`と`Tetris.hpp`を適当な場所に置く

## 依存ライブラリ
OpenSiv3D(v0.6.2)

## コード例

```C++:main.cpp
# include <Siv3D.hpp> // OpenSiv3D v0.6.2
# include "Tetris.hpp"

void Main()
{
    TetriSys::Tetris tetris;

	while (System::Update())
	{
        tetris.update();
        tetris.draw();
	}
}
```
