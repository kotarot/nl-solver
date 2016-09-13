# single_3d_hls

Vivado HLS 用 single_3d


## メモ

* Vivado HLS 2016.2
* Vivado 2016.2

高位合成オプション:
* Clock Period: 10.0
* Clock Uncertainty 2.0ns

論理合成のオプション:
* Synthesis strategy: Vivado Synthesis Defaults
* Implementation strategy: Performance_ExplorePostRoutePhysOpt


## 入出力

nlsolver への入力は boardstr という問題ファイルをコンパクトに記述したようなワンラインの文字列。

nlsolver からの出力は char 型の配列で、各要素に解答ファイルに相当する数値が入っている。

例: 問題ファイル (`NL_Q00.txt`)

```
SIZE 10X5X3
LINE_NUM 4
LINE#1 (0,0,1) (7,4,1)
LINE#2 (0,4,1) (7,0,2)
LINE#3 (1,2,1) (2,2,1)
LINE#4 (9,0,1) (0,0,3)

VIA#a (4,2,1) (4,2,2)
VIA#zz (9,4,3) (9,4,2) (9,4,1)
```

これを boardstr で表記すると、

```
X10Y05Z3L010000107041L020004107002L030102102021L040900100003V010402104022V020904309041
```


## HLS 合成メモ

* routing

8.34 / 1041 / 2451556  
8.34 / 1022 / 2451477  
8.34 / 1022 / 2450457  
8.34 / 1022 / 508153 リソースオーバー  
8.34 / 1022 / 508153 リソースオーバー  
8.34 / 1022 / 2450457  
8.34 / 922 / 2450335  
8.34 / 922 / 2445575  
8.34 / 922 / 2442355  
8.34 / 922 / 2242816  

* final_routing

8.35 / 1846 / 27121  
8.35 / 1827 / 25975  
8.35 / 1808 / 25653  
8.35 / 1808 / 25493  

* nlsolver

8.35 / 25945 / 2407880047247  
