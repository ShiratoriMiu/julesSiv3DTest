#include <Siv3D.hpp>

void Main()
{
    s3d::Scene::SetBackground(s3d::ColorF{ 0.8, 0.9, 1.0 });

    s3d::Vec2 playerPosition{ 400, 300 };

    while (s3d::System::Update())
    {
        if (s3d::KeyW.pressed())
        {
            playerPosition.y -= 5.0;
        }
        if (s3d::KeyS.pressed())
        {
            playerPosition.y += 5.0;
        }
        if (s3d::KeyA.pressed())
        {
            playerPosition.x -= 5.0;
        }
        if (s3d::KeyD.pressed())
        {
            playerPosition.x += 5.0;
        }

        s3d::Circle(playerPosition, 30).draw(s3d::Palette::Orange);
    }
}
