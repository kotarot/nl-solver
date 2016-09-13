# single_3d_hls_route

※ これは試しに高位合成・論理合成・配置配線までをやったやつだから動かない

Vivado HLS 用 single_3d (routing関数部分)


## メモ

* Vivado HLS 2016.2
* Vivado 2016.2

高位合成は、Clock period 10.0ns, Uncertainty 2.5ns で合成。


## ディレクティブによる結果メモ

ARRAY_PARTITIONなし  
* レイテンシ: 1976 / 1895021
* リソース: 31 / 29 / 5191 / 9686

nextなんちゃらをARRAY_PARTITION  
* レイテンシ: 1976 / 1895021
* リソース: 31 / 29 / 5191 / 9686

ARRAY_PARTITIONを消してrouting_arrange()関数に切り出した  
* レイテンシ: 1976 / 1894213  
* リソース: 31 / 29 / 5255 / 9933

routing_arrange(), inInserted_1(), isInserted_2(), countLine() に `#pragma HLS INLINE` を追加した  
* レイテンシ: 2174 / 1891609
* リソース: 31 / 29 / 5131 / 9714

PIPELINEをいろんなループに仕込む (入れすぎると合成できないこともあるし、リソースが足りなくなることもある)  
* レイテンシ: 2065 / 1564679
* リソース: 31 / 29 / 5394 / 9759
