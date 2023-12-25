# 概要
勉強目的で現在1人で制作中のゲームのライブラリです。(Windowsにのみ対応)  
Engineディレクトリ下にライブラリのソースコードがあります。  
Editorディレクトリにエディタを実装予定ですが現在は未完成です。  
Simulationディレクトリ下にこのライブラリを用いて制作したシミュレーションプログラムを追加していく予定です。

# 使用ツール/ライブラリ
- Visual Studio 2022
- DirectX12
- DirectXMath
- DirectX Shader Compiler
- FBX SDK
- ImGui(現在はEngineプロジェクトに既存の項目としてc++ファイルを追加している)

# 実装予定
- 他のシミュレーション
- コリジョン

# Simulation
現在はボイドシミュレーションと同期シミュレーションを実装しています。  
UIを用いてパラメータを調整できます。  
## ボイドシミュレーション
Boidsアルゴリズムによる群れのシミュレーションです。
![boid](https://github.com/kokoro-kanade/Rizityo/assets/49611290/8a2441bf-9f57-446e-badf-c4d81b1228af)
## 同期シミュレーション
振動子が移動しながら近傍の振動子と相互作用して同期するモデルのシミュレーションです。
![sync](https://github.com/kokoro-kanade/Rizityo/assets/49611290/fcb07c8b-46f1-4632-b432-e42b45bd34a6)
## Simulation操作
- Boidシミュレーション : 1キー
- 同期シミュレーション : 2キー
- UIの表示切替 : 0キー
## カメラ操作
- 前方移動：Wキー
- 後方移動：Sキー
- 左移動：Aキー
- 右移動：Dキー
- 上昇：Eキー
- 下降：Qキー
- 視点操作：左クリックを押しながらマウス移動
