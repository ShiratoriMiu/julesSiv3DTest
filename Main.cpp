# include <Siv3D.hpp> // Siv3D v0.6.12

void Main()
{
	// 背景色を設定
	Scene::SetBackground(ColorF{ 0.8, 0.9, 1.0 });

	// プレイヤーの定数
	constexpr double Gravity = 2000.0; // 重力加速度
	constexpr double JumpVelocity = 800.0; // ジャンプの初速度
	constexpr double MoveSpeed = 300.0; // 移動速度
	constexpr SizeF PlayerSize{ 40, 60 }; // プレイヤーのサイズ

	// ステージの定数
	constexpr double GroundHeight = 50.0;
	constexpr double StageWidth = 3000.0; // ステージ全体の幅
	constexpr double GoalX = StageWidth - 100.0; // ゴールのX座標

	// ステージの要素
	enum class TileType
	{
		Ground,
		Hole,
	};

	struct StageTile
	{
		RectF rect;
		TileType type;
	};

	// ステージの生成
	Array<StageTile> stageTiles;
	double currentX = 0;
	const double segmentWidths[] = { 400, 200, 500, 150, 600, 250, 400 }; // 地面、穴、地面、穴...
	TileType currentTileType = TileType::Ground;

	for (const auto width : segmentWidths)
	{
		stageTiles.push_back({ RectF{ currentX, Scene::Height() - GroundHeight, width, GroundHeight }, currentTileType });
		currentX += width;
		currentTileType = (currentTileType == TileType::Ground) ? TileType::Hole : TileType::Ground;
	}
	// 最後の地面の後にゴールを追加
	stageTiles.push_back({ RectF{ GoalX - 50, Scene::Height() - GroundHeight - 100, 100, 100 }, TileType::Ground }); // ゴール地点の足場


	// プレイヤーの状態
	struct Player
	{
		Vec2 position;
		Vec2 velocity;
		bool isOnGround;
		RectF getRect() const { return RectF{ Arg::center = position, PlayerSize }; }
	};

	Player player{
		.position = Vec2{ 200, Scene::Height() - GroundHeight - PlayerSize.y / 2 - 100 }, // 初期位置
		.velocity = Vec2{ 0, 0 },
		.isOnGround = false
	};

	// カメラの設定
	Camera2D camera{ Scene::Center(), 1.0 }; // 初期カメラ（中央表示、ズーム1倍）

	// ゲームの状態
	enum class GameState
	{
		Playing,
		GameOver,
		Goal,
	};
	GameState gameState = GameState::Playing;
	Font font{ 60 };

	while (System::Update())
	{
		const double deltaTime = Scene::DeltaTime();

		if (gameState == GameState::Playing)
		{
			// プレイヤーの移動処理
			if (KeyLeft.pressed())
			{
				player.position.x -= MoveSpeed * deltaTime;
			}
			if (KeyRight.pressed())
			{
				player.position.x += MoveSpeed * deltaTime;
			}

			// プレイヤーのジャンプ処理
			if (player.isOnGround && KeySpace.down())
			{
				player.velocity.y = -JumpVelocity;
				player.isOnGround = false;
			}

			// 重力処理
			if (!player.isOnGround)
			{
				player.velocity.y += Gravity * deltaTime;
			}
			player.position.y += player.velocity.y * deltaTime;

			// ステージとの衝突判定
			player.isOnGround = false;
			RectF playerRect = player.getRect();

			for (const auto& tile : stageTiles)
			{
				if (tile.type == TileType::Ground && playerRect.intersects(tile.rect))
				{
					if (player.velocity.y > 0 && playerRect.bottom() > tile.rect.y && playerRect.top() < tile.rect.y) // 上から衝突
					{
						player.position.y = tile.rect.y - PlayerSize.y / 2;
						player.velocity.y = 0;
						player.isOnGround = true;
						break;
					}
				}
			}

			// 落とし穴判定 (画面外も含む)
			if (player.position.y > Scene::Height() + PlayerSize.y) // ある程度画面下に行ったら
			{
				bool onGroundTile = false;
				for (const auto& tile : stageTiles)
				{
					if (tile.type == TileType::Ground && playerRect.intersects(tile.rect))
					{
						onGroundTile = true;
						break;
					}
				}
				if (!onGroundTile) // どの地面にも接していなければゲームオーバー
				{
					gameState = GameState::GameOver;
				}
			}


			// ゴール判定
			if (player.getRect().right() > GoalX)
			{
				gameState = GameState::Goal;
			}
		}
		else if (gameState == GameState::GameOver)
		{
			// ゲームオーバー時の処理 (例: Rキーでリスタート)
			if (KeyR.down())
			{
				// プレイヤーを初期位置に戻すなどリセット処理
				player.position = Vec2{ 200, Scene::Height() - GroundHeight - PlayerSize.y / 2 - 100 };
				player.velocity = Vec2{ 0, 0 };
				player.isOnGround = false;
				gameState = GameState::Playing;
			}
		}
		else if (gameState == GameState::Goal)
		{
			// ゴール時の処理 (例: Rキーでリスタート)
			if (KeyR.down())
			{
				player.position = Vec2{ 200, Scene::Height() - GroundHeight - PlayerSize.y / 2 - 100 };
				player.velocity = Vec2{ 0, 0 };
				player.isOnGround = false;
				gameState = GameState::Playing;
			}
		}

		// カメラの更新 (プレイヤーを追従)
		camera.setTargetCenter(Vec2{ player.position.x + Scene::Width() * 0.2, Scene::Center().y }); // 少しプレイヤーを左に寄せる
		camera.update();
		const auto t = camera.createTransformer(); // これ以降の描画にカメラを適用

		// ステージの描画
		for (const auto& tile : stageTiles)
		{
			if (tile.type == TileType::Ground)
			{
				tile.rect.draw(Palette::Green);
			}
			// 落とし穴は描画しない (背景が見える)
		}

		// ゴールの描画
		RectF{ GoalX, Scene::Height() - GroundHeight - 50, 10, 50 }.draw(Palette::Gold);


		// プレイヤーの描画
		player.getRect().draw(ColorF{0.2, 0.5, 0.8});

		// UIの描画 (カメラの影響を受けないように、Transformer のスコープ外、または ScopedRenderStates2D(SamplerState::ClampNearest) などで描画)
		if (gameState == GameState::GameOver)
		{
			font(U"ゲームオーバー").drawAt(Scene::Center(), Palette::Red);
			font(U"Rキーでリスタート").drawAt(Scene::Center().movedBy(0, 80), Palette::Black);
		}
		else if (gameState == GameState::Goal)
		{
			font(U"ゴール！").drawAt(Scene::Center(), Palette::Orange);
			font(U"Rキーでリスタート").drawAt(Scene::Center().movedBy(0, 80), Palette::Black);
		}
	}
}
