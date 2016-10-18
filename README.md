# nl-solver

nl-solver is a machine learning-based Numberlink puzzle solver.

## Requirements

バージョンは我々が使用しているものなので、もっと古くても動くかもしれない。

* Python 2.7.11
* Numpy 1.11.1 (Install: `pip install numpy`)
* Chainer 1.13.0 (Install: `pip install chainer`)


## Usage

### 2015年版ソルバ

(2015年版) 機械学習トレーニングフェーズ:
```
cd ml
python train_window.py -h  # ヘルプ表示
```

(2015年版) ソルブフェーズ:
```
python top.py -h  # ヘルプ表示
```


## Publications

1. 寺田晃太朗, 川村一志, 多和田雅師, 藤原晃一, 戸川望, "機械学習を用いたナンバーリンクソルバ," 情報処理学会DAシンポジウム2015ポスター発表, Aug. 2015.
1. 多和田雅師, 阿部晋矢, 川村一志, 藤代美佳, "発見的アルゴリズムによるナンバーリンクソルバ," 情報処理学会DAシンポジウム2014ポスター発表, Aug. 2014.


## Awards

1. 情報処理学会 DAシンポジウム2015 アルゴリズムデザインコンテスト 優秀賞 (学生部門)
1. 情報処理学会 DAシンポジウム2014 アルゴリズムデザインコンテスト 特別賞


## License

This software is released under GPL v3 License, see [LICENSE](/LICENSE).
