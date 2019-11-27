#include "World.h"
#include "HelperFunctions.h"
#include <iostream>
#include <sstream>

std::vector<std::shared_ptr<IObject>> World::getAllObjects()
{
    std::vector<std::shared_ptr<IObject>> objects;
    objects.insert(objects.end(), m_balls.begin(), m_balls.end());
    objects.insert(objects.end(), m_plates.begin(), m_plates.end());
    objects.insert(objects.end(), m_bricks.begin(), m_bricks.end());
    objects.insert(objects.end(), m_walls.begin(), m_walls.end());
    objects.insert(objects.end(), m_bonuses.begin(), m_bonuses.end());
    return objects;
}

bool World::isObjectOutOfBorder(std::shared_ptr<IObject> object)
{
    auto objectPos = object->state().getPos();
    if (objectPos.x < 0 || objectPos.y < 0 ||
        objectPos.x > m_worldSize.x ||
        objectPos.y > m_worldSize.y)
    {
        return true;
    }

    return false;
}

World::World(std::shared_ptr<IObjectFactory> objectFactory, sf::Vector2f worldSize)
{
    m_worldSize = worldSize;
    m_objectFactory = objectFactory;
    m_font = hf::getDefaultFont();
    m_isGameOver = true;
    m_scopes = 0;
}

void World::generate()
{
    m_isGameOver = false;
    removeAllObjects();

    auto& of = m_objectFactory;
    sf::Vector2f ballPos = {m_worldSize.x * 0.5f, m_worldSize.y * 0.9f};
    auto ball = of->createObject(ObjectType::Ball);
    ball->state().setPos(ballPos);
    ball->state().setSize({20, 20});
    m_balls.push_back(ball);

    sf::Vector2f brickZoneSize = {m_worldSize.x * 0.8f, m_worldSize.y * 0.3f};
    sf::Vector2f brickZoneLeftTopPos = {m_worldSize.x * 0.1f, m_worldSize.y * 0.2f};
    sf::Vector2i resolutionInBricks = {10, 5};
    float brickGap = 8;
    sf::Vector2f brickSize = {
        brickZoneSize.x / resolutionInBricks.x,
        brickZoneSize.y / resolutionInBricks.y
    };
    for (auto brickCol = 0; brickCol < resolutionInBricks.x; ++brickCol)
    {
        for (auto brickRow = 0; brickRow < resolutionInBricks.y; ++brickRow)
        {
            auto brick = of->createObject(ObjectType::Brick);
            sf::Vector2f brickPos = {
                brickSize.x / 2 + brickCol * brickSize.x + brickZoneLeftTopPos.x,
                brickSize.y / 2 + brickRow * brickSize.y + brickZoneLeftTopPos.y
            };
            brick->state().setPos(brickPos);
            brick->state().setSize({brickSize.x - brickGap, brickSize.y - brickGap});
            m_bricks.push_back(brick);
        }
    }

    float wallKoefThinkness = 0.02;
    float wallTopOffset = 0.05;
    sf::Vector2f verticalWallSize = {m_worldSize.x * wallKoefThinkness, m_worldSize.y * (1 - wallTopOffset)};
    auto leftWall = of->createObject(ObjectType::Wall);
    leftWall->state().setCollisionRect(verticalWallSize,
                                       {m_worldSize.x * wallKoefThinkness / 2, m_worldSize.y * (0.5f + wallTopOffset)});
    auto rightWall = of->createObject(ObjectType::Wall);
    rightWall->state().setCollisionRect(verticalWallSize,
                                        {
                                            m_worldSize.x * (1 - wallKoefThinkness / 2),
                                            m_worldSize.y * (0.5f + wallTopOffset)
                                        });
    sf::Vector2f horizontalWallSize = {m_worldSize.x, m_worldSize.y * wallKoefThinkness};
    auto topWall = of->createObject(ObjectType::Wall);
    topWall->state().setCollisionRect(horizontalWallSize,
                                      {m_worldSize.x / 2, m_worldSize.y * (wallKoefThinkness / 2 + wallTopOffset)});

    m_walls.push_back(leftWall);
    m_walls.push_back(rightWall);
    m_walls.push_back(topWall);

    float plateKoefThinkness = 0.04;
    float plateKoefSize = 0.2;
    auto plate = of->createObject(ObjectType::Plate);
    plate->state().setSize({m_worldSize.x * plateKoefSize, m_worldSize.y * plateKoefThinkness});
    plate->state().setPos({m_worldSize.x / 2, m_worldSize.y * (1 - plateKoefThinkness)});
    plate->setOnBumpingCallBack([&](auto, std::vector<Collision>& collisions)
    {
        m_scopes += collisions.size();
    });
    m_plates.push_back(plate);

    m_collisionProcessors.push_back({m_balls, m_walls, {}});
    m_collisionProcessors.push_back({m_balls, m_plates, {}});
    m_collisionProcessors.emplace_back(
        m_balls, m_bricks, [&](std::shared_ptr<IObject> thisObject, std::vector<Collision>& collisions)
        {
            if (!collisions.empty())
            {
                auto bonus = m_objectFactory->createObject(ObjectType::Bonus);
                bonus->state().setSize({5, 5});
                bonus->state().setPos(thisObject->state().getPos());
                m_bonuses.push_back(bonus);
            }
        }
    );
    m_collisionProcessors.push_back({m_plates, m_walls, {}});
    m_collisionProcessors.emplace_back(
        m_plates, m_bonuses, [&](std::shared_ptr<IObject> thisObject, std::vector<Collision>& collisions)
        {
            for (auto& collision : collisions)
            {
                auto obj = collision.getObject();
                obj->state().setDestroyFlag(true);
                m_scopes++;
            }
        }
    );
}

bool World::isGameOver()
{
    return m_isGameOver;
}

void World::removeObjectsIfDestroyed(std::vector<std::shared_ptr<IObject>>& objects)
{
    auto removeIt = std::remove_if(objects.begin(), objects.end(),
                                   [](std::shared_ptr<IObject> object)
                                   {
                                       return object->state().getDestroyFlag();
                                   });

    objects.erase(removeIt, objects.end());
}

void World::removeAllDestroyedObjects()
{
    removeObjectsIfDestroyed(m_balls);
    removeObjectsIfDestroyed(m_plates);
    removeObjectsIfDestroyed(m_bricks);
    removeObjectsIfDestroyed(m_walls);
    removeObjectsIfDestroyed(m_bonuses);
}

void World::removeAllObjects()
{
    m_balls.clear();
    m_plates.clear();
    m_bricks.clear();
    m_walls.clear();
    m_bonuses.clear();
    m_collisionProcessors.clear();
}

void World::updateState(std::optional<sf::Event> event, sf::Time timeStep)
{
    if (m_isGameOver)
    {
        generate();
    }

    for (auto& collisionProcessor : m_collisionProcessors)
    {
        collisionProcessor.process();
    }
    removeAllDestroyedObjects();

    auto allObjects = getAllObjects();
    for (auto object : allObjects)
    {
        object->calcState(event, timeStep);
        if (isObjectOutOfBorder(object))
        {
            object->state().setDestroyFlag(true);
        }
    }
    removeAllDestroyedObjects();

    auto isAllBallsOutOfBorder = std::none_of(m_balls.begin(), m_balls.end(), [&](auto ball)
    {
        return !isObjectOutOfBorder(ball);
    });

    if (isAllBallsOutOfBorder && !m_bricks.empty())
    {
        m_scopes = 0;
        m_isGameOver = true;
    }
    else if (m_bricks.empty())
    {
        std::for_each(m_balls.begin(), m_balls.end(), [](std::shared_ptr<IObject> ballObject)
        {
            ballObject->state().setDestroyFlag(true);
        });

        if (m_bonuses.empty())
            m_isGameOver = true;
    }
}

void World::draw(sf::RenderWindow& window)
{
    for (auto object : getAllObjects())
    {
        object->draw(window);
    }

    sf::Text text;
    text.setFont(m_font);
    text.setScale(0.7, 0.7);
    std::ostringstream ss;
    ss << "Scopes: " << m_scopes << std::endl;
    text.setString(ss.str());
    window.draw(text);
}
