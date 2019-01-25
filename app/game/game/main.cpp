#include "stdafx.h"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <ctime>
#include <cstdlib>
#include <optional>
#include <sstream>
#include "ClientFactory.h"
#include "ServerPackage.h"
#include "Paddle.h"

class Ball
{
    sf::CircleShape m_shape;
    float ballRadius = 10.f;
    sf::Sound ballSound;

public:
    Ball()
    {
        m_shape.setRadius(ballRadius - 3);
        m_shape.setOutlineThickness(3);
        m_shape.setOutlineColor(sf::Color::Black);
        m_shape.setFillColor(sf::Color::White);
        m_shape.setOrigin(ballRadius / 2, ballRadius / 2);        
    }
    void setPosition(float x, float y) { m_shape.setPosition(x, y); }
    const sf::Vector2f& getPosition() const { return m_shape.getPosition(); }
    void move(float offsetX, float offsetY) { m_shape.move(offsetX, offsetY); }
    void changeRandomColor()
    {
        const sf::Color newColor(std::rand() % 0xff, std::rand() % 0xff, std::rand() % 0xff);
        m_shape.setFillColor(newColor);
    }
    void draw(sf::RenderWindow& window) { window.draw(m_shape); }
    float getRadius() const { return m_shape.getRadius(); }
    void setSoundBuffer(const sf::SoundBuffer & ballSoundBuffer) { ballSound.setBuffer(ballSoundBuffer); }
    void playSound() { ballSound.play(); }
};

class PauseMessage
{
    sf::Text pauseMessage;

public:
    PauseMessage()
    {
        sf::Text pauseMessage;
        pauseMessage.setCharacterSize(40);
        pauseMessage.setPosition(170.f, 150.f);
        pauseMessage.setFillColor(sf::Color::White);
        pauseMessage.setString("Welcome to SFML pong!\nPress space to start the game");        
    }
    void setFont(const sf::Font& font) { pauseMessage.setFont(font); }
    void setString(const sf::String& string) { pauseMessage.setString(string); }
    void draw(sf::RenderWindow& window) { window.draw(pauseMessage); }
};

class Server
{
    // Define some constants;
    const int gameWidth;
    const int gameHeight;

    Paddle leftPaddle;
    Paddle rightPaddle;
    Ball ball;
    sf::SoundBuffer ballSoundBuffer;
    sf::Font font;
    PauseMessage pauseMessage;

    bool isAutoChangeColor = true;
    bool isSurpriseMode = true;

    bool isPlaying = false;

    const float paddleSpeed = 400.f;
    const float ballSpeed = 400.f;
    float ballAngle = 0.f; // to be changed later

    sf::Clock clock;

    std::shared_ptr<IClient> client01;
    std::shared_ptr<IClient> client02;

public:
    Server(const int gameWidht, const int gameHeight)
        : gameWidth(gameWidht)
        , gameHeight(gameHeight)
    {
        // Load the sounds used in the game
        const std::string soundFileName = "resources/ball.wav";
        if (!ballSoundBuffer.loadFromFile(soundFileName))
        {
            std::stringstream ss;
            ss << "can't load sound from file " << soundFileName;
            throw std::runtime_error(ss.str());
        }
        ball.setSoundBuffer(ballSoundBuffer);

        leftPaddle.setFillColor(sf::Color(100, 100, 200));
        rightPaddle.setFillColor(sf::Color(200, 100, 100));

        // Load the text font
        const std::string fontFileName = "resources/sansation.ttf";
        if (!font.loadFromFile(fontFileName))
        {
            std::stringstream ss;
            ss << "can't load font from file " << fontFileName;
            throw std::runtime_error(ss.str());
        }

        pauseMessage.setFont(font);
        pauseMessage.setString("Welcome to SFML pong!\nPress space to start the game");
    }

    void restartGame()
    {
        if (!isPlaying)
        {
            // (re)start the game
            isPlaying = true;
            clock.restart();

            // Reset the position of the paddles and ball
            leftPaddle.setPosition(10 + leftPaddle.getSize().x / 2, gameHeight / 2);
            rightPaddle.setPosition(gameWidth - 10 - leftPaddle.getSize().x / 2, gameHeight / 2);
            ball.setPosition(gameWidth / 2, gameHeight / 2);

            // Reset the ball angle
            do
            {
                // Make sure the ball initial angle is not too much vertical
                ballAngle = Math::degToRad(std::rand() % 360);
            } while (std::abs(std::cos(ballAngle)) < 0.7f);
        }
    }

    void draw(sf::RenderWindow & window)
    {
        // Clear the window
        window.clear(sf::Color(50, 200, 50));

        if (isPlaying)
        {
            leftPaddle.draw(window);
            rightPaddle.draw(window);
            ball.draw(window);
        }
        else
        {
            pauseMessage.draw(window);
        }        
    }

    void iterate()
    {
        if (isPlaying)
        {
            float deltaTime = clock.restart().asSeconds();

            // Move the player's paddle
            if (client01->paddleUp())
                leftPaddle.move(0.f, -paddleSpeed * deltaTime);

            if (client01->paddleDown())
                leftPaddle.move(0.f, paddleSpeed * deltaTime);

            // Move the player's paddle
            if (client02->paddleUp())
                rightPaddle.move(0.f, -paddleSpeed * deltaTime);

            if (client02->paddleDown())
                rightPaddle.move(0.f, paddleSpeed * deltaTime);

            // Move the ball
            float factor = ballSpeed * deltaTime;
            ball.move(std::cos(ballAngle) * factor, std::sin(ballAngle) * factor);

            // Check collisions between the ball and the screen
            // LEFT
            if (ball.getPosition().x - ball.getRadius() < 0.f)
            {
                isPlaying = false;
                pauseMessage.setString("You lost!\nPress space to restart or\nescape to exit");
            }
            // RIGHT
            if (ball.getPosition().x + ball.getRadius() > gameWidth)
            {
                isPlaying = false;
                pauseMessage.setString("You won!\nPress space to restart or\nescape to exit");
            }
            // TOP
            if (ball.getPosition().y - ball.getRadius() < 0.f)
            {
                ball.playSound();
                ballAngle = -ballAngle;
                if (isSurpriseMode)
                {
                    ballAngle += Math::degToRad(Math::randRange(-30, 30));
                }
                ball.setPosition(ball.getPosition().x, ball.getRadius() + 0.1f);
            }
            // BOTTOM
            if (ball.getPosition().y + ball.getRadius() > gameHeight)
            {
                ball.playSound();
                ballAngle = -ballAngle;
                if (isSurpriseMode)
                {
                    ballAngle += Math::degToRad(Math::randRange(-30, 30));
                }
                ball.setPosition(ball.getPosition().x, gameHeight - ball.getRadius() - 0.1f);
            }

            // Check the collisions between the ball and the paddles
            // Left Paddle
            if (ball.getPosition().x - ball.getRadius() < leftPaddle.getPosition().x + leftPaddle.getSize().x / 2 &&
                ball.getPosition().x - ball.getRadius() > leftPaddle.getPosition().x &&
                ball.getPosition().y + ball.getRadius() >= leftPaddle.getPosition().y - leftPaddle.getSize().y / 2 &&
                ball.getPosition().y - ball.getRadius() <= leftPaddle.getPosition().y + leftPaddle.getSize().y / 2)
            {
                if (ball.getPosition().y > leftPaddle.getPosition().y)
                    ballAngle = M_PI - ballAngle + Math::degToRad(std::rand() % 50);
                else
                    ballAngle = M_PI - ballAngle - Math::degToRad(std::rand() % 50);

                ball.playSound();
                if (isAutoChangeColor)
                {
                    ball.changeRandomColor();
                }
                ball.setPosition(leftPaddle.getPosition().x + ball.getRadius() + leftPaddle.getSize().x / 2 + 0.1f, ball.getPosition().y);
            }

            // Right Paddle
            if (ball.getPosition().x + ball.getRadius() > rightPaddle.getPosition().x - rightPaddle.getSize().x / 2 &&
                ball.getPosition().x + ball.getRadius() < rightPaddle.getPosition().x &&
                ball.getPosition().y + ball.getRadius() >= rightPaddle.getPosition().y - rightPaddle.getSize().y / 2 &&
                ball.getPosition().y - ball.getRadius() <= rightPaddle.getPosition().y + rightPaddle.getSize().y / 2)
            {
                if (ball.getPosition().y > rightPaddle.getPosition().y)
                    ballAngle = M_PI - ballAngle + Math::degToRad(std::rand() % 50);
                else
                    ballAngle = M_PI - ballAngle - Math::degToRad(std::rand() % 50);

                ball.playSound();
                if (isAutoChangeColor)
                {
                    ball.changeRandomColor();
                }
                ball.setPosition(rightPaddle.getPosition().x - ball.getRadius() - rightPaddle.getSize().x / 2 - 0.1f, ball.getPosition().y);
            }
        }        
    }

    void setClient01(std::shared_ptr<IClient> client) { client01 = client; }
    void setClient02(std::shared_ptr<IClient> client) { client02 = client; }

    ServerPackage getPackage()
    {
        ServerPackage sp {};
        sp.screenSize = { gameWidth, gameHeight };
        sp.leftPaddleCenter = leftPaddle.getPosition();
        sp.leftPaddleSize = leftPaddle.getSize();
        sp.rightPaddleCenter = rightPaddle.getPosition();
        sp.rightPaddleSize = rightPaddle.getSize();
        sp.ballCenter = ball.getPosition();
        sp.ballRadius = ball.getRadius();
        sp.paddleSpeed = paddleSpeed;
        sp.ballSpeed = ballSpeed;
        return sp;
    }
};

class World
{
    const int gameWidth = 800;
    const int gameHeight = 600;

    sf::RenderWindow window;
    Server server;
    ClientFactory clientFactory;
    std::shared_ptr<IClient> client01;
    std::shared_ptr<IClient> client02;

public:
    World()
        : window(sf::VideoMode(gameWidth, gameHeight, 32), "SFML Pong", 
            sf::Style::Titlebar | sf::Style::Close)
        , server(gameWidth, gameHeight)
    {
        std::srand(static_cast<unsigned int>(std::time(NULL)));
        window.setVerticalSyncEnabled(true);
        client01 = clientFactory.createClient(ClientType::User, "WS");
        client02 = clientFactory.createClient(ClientType::Bot, "PL");
        server.setClient01(client01);
        server.setClient02(client02);
    }

    int mainLoop()
    {
        while (window.isOpen())
        {
            // Handle events
            sf::Event event;
            while (window.pollEvent(event))
            {
                // Window closed or escape key pressed: exit
                if ((event.type == sf::Event::Closed) ||
                    ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Escape)))
                {
                    window.close();
                    break;
                }

                // Space key pressed: play
                if ((event.type == sf::Event::KeyPressed) && (event.key.code == sf::Keyboard::Space))
                {
                    server.restartGame();
                }
            }

            const auto serverPackage = server.getPackage();
            client01->setServerPackage(serverPackage);
            client01->updateState();
            client02->setServerPackage(serverPackage);
            client02->updateState();
            server.iterate();

            server.draw(window);

            // Display things on screen
            window.display();
        }

        return EXIT_SUCCESS;
    }    
};

int main()
{
    World world;
    return world.mainLoop();
}