#include <SFML/Graphics.hpp>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

// a mathematical normalization of a vector in order to obtain a direction of the
// spaceships, used later
sf::Vector2f normalize(sf::Vector2f vect) {
    float length = std::sqrt(vect.x * vect.x + vect.y * vect.y);
    if (length == 0.f) // it cannot be 0
        return sf::Vector2f(0.f, 0.f);
    return sf::Vector2f(vect.x / length, vect.y / length);
}

// returns a random position on the screen,
// but not too close to the edges (by margin)
sf::Vector2f randomPosition(const sf::RenderWindow& win, float margin = 100.f) {
    float windowWidth = static_cast<float>(win.getSize().x); //width of window
    float windowHeight = static_cast<float>(win.getSize().y); //height of window
    float x = margin + rand() % static_cast<int>(windowWidth - 2 * margin); // random x
    float y = margin + rand() % static_cast<int>(windowHeight - 2 * margin); // random y
    return sf::Vector2f(x, y); // 2D vector (a,b)
}

// calculates the distance between two points (a and b) by Pythagorean
float distance(sf::Vector2f a, sf::Vector2f b) {
    float dx = a.x - b.x;  // Difference in X
    float dy = a.y - b.y;  // Difference in Y
    return std::sqrt(dx * dx + dy * dy);
}

int main() {
    srand(static_cast<unsigned>(time(nullptr))); // time as seed for random generator
    sf::RenderWindow window(sf::VideoMode(800, 600), "Black Hole Game");
    window.setFramerateLimit(60); // setting maximum FPS

    // spaceship as sprite
    sf::Texture SpaceshipPic;
    if (!SpaceshipPic.loadFromFile("spaceship.png")) {
        std::cerr << "spaceship.png not loaded" << std::endl; // for the console information
        return -1; // ending the program if not found
    }
    sf::Sprite spaceship1(SpaceshipPic);
    sf::Sprite spaceship2(SpaceshipPic);

    spaceship1.setOrigin(SpaceshipPic.getSize().x / 2, SpaceshipPic.getSize().y / 2); // sets middle of picture as origin
    spaceship2.setOrigin(SpaceshipPic.getSize().x / 2, SpaceshipPic.getSize().y / 2);

    spaceship1.setScale(0.1f, 0.1f); // size
    spaceship2.setScale(0.1f, 0.1f);
    spaceship1.setColor(sf::Color(255, 255, 255, 180)); // setting a colour for both of starships to white
    spaceship2.setColor(sf::Color(255, 255, 255, 180));

    sf::CircleShape glow(SpaceshipPic.getSize().x * 0.06f);
    // glow to make spaceships more visible
    // with a bright theme there were nicely seen but when I added dark it changed
    glow.setOrigin(glow.getRadius(), glow.getRadius());
    glow.setFillColor(sf::Color(255, 255, 255, 100)); // white glow, transparent

    // Font: spacefuture downloaded from the Internet
    sf::Font font;
    if (!font.loadFromFile("spacefuture.ttf")) return -1;

    sf::Text scoreT("", font, 24), timeT("", font, 24), livesT("", font, 24), overT("GAME OVER", font, 64);
    sf::Text restartT("", font, 28);

    overT.setOrigin(overT.getLocalBounds().width / 2, overT.getLocalBounds().height / 2); // centering the text
    overT.setPosition(400, 200);
    restartT.setPosition(240, 300); // defualt position, dynamically actualised
    scoreT.setPosition(600, 10);
    timeT.setPosition(600, 40);
    livesT.setPosition(600, 70);

    // Blackhole texture animation
    std::vector<sf::Texture> holeAnimation; // vector for frames of blackhole
    for (int i = 1; i <= 64; ++i) {
        sf::Texture hole;
        std::string holefile = std::string("portal") + (i < 10 ? "0" : "") + std::to_string(i) + ".png";
        if (!hole.loadFromFile(holefile)) {
            std::cerr << "portalxx not loaded " << holefile << std::endl;
            continue;
        }
        holeAnimation.push_back(std::move(hole));
    }

    sf::Sprite holeSprite;
    holeSprite.setScale(0.2f, 0.2f); // size of the blackhole
    sf::Clock holeAnimClock; // animation timer
    int holeFrame = 0; // index of animation frame

    // backgrounds
    std::vector<sf::Texture> backgroundTextures; // vector for background images
    for (int i = 1; i <= 3; ++i) {
        sf::Texture tex;
        std::string filename = std::string("background") + std::to_string(i) + ".jpg";
        if (!tex.loadFromFile(filename)) {
            std::cerr << "backgroundx not loaded " << filename << std::endl;
            continue;
        }
        backgroundTextures.push_back(std::move(tex));
    }

    sf::Sprite backgroundSprite;
    int currentBackground = 0;
    sf::Clock backgroundClock;
    float backgroundInterval = 5.0f; // change of background every 5 seconds

    if (!backgroundTextures.empty()) { // protection of lack of backhground, had several errorrs related
        backgroundSprite.setTexture(backgroundTextures[currentBackground]);
    }

    sf::Clock clk, totalClock, roundClock;
    bool holeOn = false,
        gameOver = false,
        s1hit = false,
        s2hit = false,
        nextlevel = false;
    int score = 0, lives = 3;
    float speed = 100;
    float finalTime = 0.0f;

    auto placeShips = [&]() { // function to place spaceships in different random locations
        do {
            spaceship1.setPosition(randomPosition(window));
            spaceship2.setPosition(randomPosition(window));
        } while (distance(spaceship1.getPosition(), spaceship2.getPosition()) < 300); // making sure they don't spawn too close
    };
    placeShips();

    while (window.isOpen()) {
        sf::Event e;
        while (window.pollEvent(e)) {
            if (e.type == sf::Event::Closed)
                window.close();

            if (!gameOver && e.type == sf::Event::MouseButtonPressed && !holeOn) {
                holeFrame = 0;
                holeSprite.setTexture(holeAnimation[holeFrame]);
                holeSprite.setOrigin(holeAnimation[holeFrame].getSize().x / 2, holeAnimation[holeFrame].getSize().y / 2);
                holeSprite.setPosition(e.mouseButton.x, e.mouseButton.y);
                holeOn = true;
                holeAnimClock.restart(); // reset timer for animation
            }

            if (e.type == sf::Event::KeyPressed && e.key.code == sf::Keyboard::R) { // restart of the game for letter 'R'
                score = 0; lives = 3; speed = 100;
                placeShips();
                holeOn = s1hit = s2hit = nextlevel = gameOver = false;
                clk.restart(); totalClock.restart(); backgroundClock.restart();
            }
        }

        float deltatime = clk.restart().asSeconds(); // delta time (needed for speed)

        if (backgroundClock.getElapsedTime().asSeconds() > backgroundInterval && !backgroundTextures.empty()) {
            currentBackground = (currentBackground + 1) % backgroundTextures.size(); // change background every interval
            backgroundSprite.setTexture(backgroundTextures[currentBackground]);
            backgroundClock.restart();
        }

        if (!gameOver) {
            sf::Vector2f dir = normalize(spaceship2.getPosition() - spaceship1.getPosition());
            float angle1 = std::atan2(dir.y, dir.x) * 180 / 3.14159f + 90; // calculate angle necessary for the picture
            // there was an issue with the direction of the sprite as it was not directed in the proper position when
            // the spaceships were aimed into themselves
            float angle2 = angle1 + 180;
            spaceship1.setRotation(angle1);
            spaceship2.setRotation(angle2);

            if (!s1hit) spaceship1.move(dir * speed * deltatime);
            if (!s2hit) spaceship2.move(-dir * speed * deltatime); // minus to get different direction

            if (!nextlevel && !s1hit && !s2hit && distance(spaceship1.getPosition(), spaceship2.getPosition()) < 40) {
                if (--lives <= 0) {
                    gameOver = true;
                    finalTime = totalClock.getElapsedTime().asSeconds();
                } else placeShips(), s1hit = s2hit = holeOn = false; // restart round
            }

            if (holeOn && !nextlevel) {
                if (!s1hit && distance(spaceship1.getPosition(), holeSprite.getPosition()) < 40) s1hit = true;
                if (!s2hit && distance(spaceship2.getPosition(), holeSprite.getPosition()) < 40) s2hit = true;
                if (s1hit && s2hit) {
                    nextlevel = true; // both ships inside
                    roundClock.restart();
                    score++;
                    speed += 10; // to increase a level of the game
                }
            }

            if (nextlevel && roundClock.getElapsedTime().asSeconds() > 1.f)
                placeShips(), s1hit = s2hit = holeOn = nextlevel = false; // next round after 1 sec delay
        }

        if (holeOn && holeAnimClock.getElapsedTime().asMilliseconds() > 80) {
            holeFrame = (holeFrame + 1) % holeAnimation.size();
            holeSprite.setTexture(holeAnimation[holeFrame]);
            holeSprite.setOrigin(holeAnimation[holeFrame].getSize().x / 2, holeAnimation[holeFrame].getSize().y / 2);
            holeAnimClock.restart(); // update frame every 80 ms
        }

        char timeBuf[20];
        float displayTime = gameOver ? finalTime : totalClock.getElapsedTime().asSeconds();
        std::snprintf(timeBuf, sizeof(timeBuf), "Time: %.2fs", displayTime);

        scoreT.setString("Score: " + std::to_string(score));
        timeT.setString(timeBuf);
        livesT.setString("Lives: " + std::to_string(lives));

        window.clear();
        if (!backgroundTextures.empty()) window.draw(backgroundSprite); // background

        if (!nextlevel && !gameOver) { // draw only if game not paused or over
            if (!s1hit) {
                glow.setPosition(spaceship1.getPosition());
                window.draw(glow);
                window.draw(spaceship1);
            }
            if (!s2hit) {
                glow.setPosition(spaceship2.getPosition());
                window.draw(glow);
                window.draw(spaceship2);
            }
        }

        if (holeOn) window.draw(holeSprite); // draw hole if active

        if (!gameOver) { // draw texts if playing
            window.draw(scoreT);
            window.draw(timeT);
            window.draw(livesT);
        } else { // game over screen
            restartT.setString("Score: " + std::to_string(score) + "\n" + timeBuf + "\nClick R to restart the game.");
            restartT.setPosition(400 - restartT.getLocalBounds().width / 2, 300);
            window.draw(overT);
            window.draw(restartT);
        }

        window.display();
    }
    return 0;
}
