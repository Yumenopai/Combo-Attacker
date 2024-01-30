※2024/1/12(金)更新

## 就職作品(個人制作)

- タイトル：Combo Attacker
- 制作期間：2023年8月～
- 開発環境：自作ライブラリ　(C++ / DirectX11 / フルスクラッチ) 
- ジャンル：3Dアクション
  
[紹介動画](https://youtu.be/-WcjV7FEAKE)

## こだわり
### Attack
　プレイヤーが簡単に攻撃できるように動作を制御しており、爽快感が出るようにしています。
　一定距離かつそのエネミー方向に向いている時、移動を強制的にエネミーへ向かわせるようにしました。また、ただ前方約90ﾟのみにしているので攻撃しようと近付いているだけ矯正させており、それほど違和感なく攻撃、また回避も出来るようになっています。
　矯正させることで攻撃の空振りを大幅に減らせることが出来、プレイヤーの得る爽快感が上がりました。

 ### AIプレイヤー
　プレイヤーの仲間としてAIキャラクター(バディ)を制作しています。(ビヘイビアツリー switch文を使用して実装)
　基本はプレイヤーにつくようにし、プレイヤーが敵に攻撃するくらいまでの一定距離まで近づくとバディも攻撃するように作成しました。
  現在誠意制作中ですが、今後はこのAIプレイヤーに重点を置いて、攻撃時のコンボ、仲間への回復行動など、行動を増やしていきたいと思っています。

### Combo
　剣・槍・ハンマーの3種で攻撃できるように実装し、また剣・槍に関しては原神を参考に、5段階目で敵を吹っ飛ばす演出を入れています。
  今後さらに範囲攻撃技の実装、格好よく見せるためのエフェクトの導入、バディとプレイヤーとの組み合わせ技など、攻撃部分を充実させていきたいと思っています。

### Shader
　キャラクター、ステージの見た目に合わせてToonShaderで実装しました。
　また見た目をある程度良くするため、UVスクロールを使用した水面表現、球面スカイボックス、シャドウマップを実装しました。

![スライド2](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/fbd72ff4-d1bf-4b5f-8e08-1bdbd5b9680c)
![スライド3](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/beea0e63-48cf-4457-a267-99a4ae197ee1)
![スライド4](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/d080a5c3-d2e1-4b26-9813-5199d9a8c144)
![スライド5](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/a5dcbf5b-6139-4735-b925-f4b97e816215)
![スライド6](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/8035eb0d-a407-48dc-bc55-99a2c4951bed)
![スライド7](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/98ecdc52-c0cf-47e1-9a9b-5a50017c16f2)
![スライド8](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/718371ab-5be1-4f83-8436-9e5bc5808e1f)
![スライド9](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/a9212d9f-e93e-4f72-9c6a-1dd33a0cdb98)
![スライド10](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/3a5ebbe7-6aac-4562-be32-ee48b16923e5)
![スライド11](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/7c5d6c48-272d-4425-a953-ef9e25dcb0fb)
![スライド12](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/515320de-d49e-4c05-8866-e4eef19d54a1)
![スライド13](https://github.com/Yumenopai/Combo-Attacker/assets/107914133/d8f34419-a8e8-43dc-9338-92a1a22c3ea9)
