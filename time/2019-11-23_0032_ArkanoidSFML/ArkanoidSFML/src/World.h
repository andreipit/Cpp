#pragma once
#include "IObjectFactory.h"
#include "IWorld.h"
#include "CollisionProcessor.h"

class World : public IWorld
{
    sf::Vector2f m_worldSize;
    std::shared_ptr<IObjectFactory> m_objectFactory;
    std::vector<std::shared_ptr<IObject>> m_balls;
    std::vector<std::shared_ptr<IObject>> m_plates;
    std::vector<std::shared_ptr<IObject>> m_bricks;
    std::vector<std::shared_ptr<IObject>> m_walls;
    std::vector<std::shared_ptr<IObject>> m_bonuses;
    std::vector<CollisionProcessor> m_collisionProcessors;
    sf::Font m_font;
    unsigned m_scopes;
    bool m_isGameOver;

    std::vector<std::shared_ptr<IObject>> getAllObjects();
    bool isObjectOutOfBorder(std::shared_ptr<IObject> object);
    void removeObjectsIfDestroyed(std::vector<std::shared_ptr<IObject>>& objects);
    void removeAllDestroyedObjects();
    void removeAllObjects();
    void generate();
    void updateGameOverStatus();
    void initPlates();
    void initWalls();
    void initBricks();
    void initBalls();
    void initCollisionProcessors();

public:
    World(std::shared_ptr<IObjectFactory> objectFactory, sf::Vector2f worldSize);
    void updateState(std::optional<sf::Event> event, sf::Time timeStep) override;
    void draw(sf::RenderWindow& window) override;
};
