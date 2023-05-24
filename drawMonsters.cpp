#include <view.h>
#include <QDebug>
#include <cmath>

struct MonsterRelative {
    Monster monster;
    double angle;
    double length;
    double deviation;
};

bool comparison(const MonsterRelative& first, const MonsterRelative& second) {
    return first.length > second.length;
}

void View::drawMonsters(QPainter *painter) {
    const std::vector<Monster>& monsters = model->getMonsters().getMonsters();
    const Player& player = model->getPlayer();
    const QPointF& position = player.getPosition();

    std::vector<MonsterRelative> monstersRelative;
    // count monsters' angles relatively to angleLeft
    for (std::size_t i = 0; i < monsters.size(); ++i) {
        // absolute value
        // use the coordinate transformation formula
        // x' = (x - a1)
        // y' = (y - a2)
        double dx = monsters[i].getPosition().x() - position.x();
        double dy = -monsters[i].getPosition().y() + position.y();

        double length = sqrt(dx * dx + dy * dy);

        // deviation of the borders of the monster
        double deviation = asin(monsters[i].getHitboxRadius() / length);

        double angle;
        if (dx > -0.000001 && dx < 0.000001) {
            angle = (M_PI / 2.0) * ((dy < 0) ? -1 : 1);
        } else {
            angle = atan(dy / dx) + ((dx < 0) ? M_PI : 0);
        }
        if (angle < 0) {
            angle += 2 * M_PI;
        }

        // recount angle relatively to center [-pi, pi]
        angle = player.getAngle() - angle;
        if (angle < -M_PI) {
            angle += 2 * M_PI;
        }
        if (angle > M_PI) {
            angle -= 2 * M_PI;
        }
        // it can't be less than -pi, because viewAngle < pi / 2
        monstersRelative.push_back(MonsterRelative({monsters[i], angle, length, deviation}));
    }
    // sorting monsters by their length to the player
    std::sort(monstersRelative.begin(), monstersRelative.end(), comparison);

    // drawing monsters
    double viewVertical = player.getViewAngle() / width() * height();
    for (std::size_t i = 0; i < monstersRelative.size(); ++i) {
        const MonsterRelative& monsterRelative = monstersRelative[i];
        QImage image = monsterRelative.monster.getImage();
        // half of the width
        int imageWidthHalf = monsterRelative.deviation / player.getViewAngle() * (width() / 2.0);
        int imageX = (width() / 2.0) * (1 + monsterRelative.angle / player.getViewAngle());

        if (monsterRelative.length < 0.0001) {
            continue;
        }
        double height = player.getHeight() + player.getJumpHeight();
        int imageButtomY = this->height() / 2 * (1 + atan(height / monsterRelative.length) / viewVertical);
        int imageTopY =
                this->height() / 2 * (1 + atan((height - monsterRelative.monster.getHeight())/ monsterRelative.length) / viewVertical);
        image = image.scaled(2 * imageWidthHalf, imageButtomY - imageTopY, Qt::IgnoreAspectRatio);

        painter->drawImage(imageX - imageWidthHalf, imageTopY, image);
    }
}
