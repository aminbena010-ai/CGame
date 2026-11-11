#include <CGame/CGame.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

struct Pipe {
    cg::Rect topRect;
    cg::Rect bottomRect;
    float x;
    bool passed = false;
};

int main() {
    cg::init();
    {
    // La ventana y los recursos se destruyen antes de glfwTerminate()
    cg::Window window("CGame++ - Flappy Bird", 400, 600);
    cg::Clock clock;

    cg::Texture birdTex("assets/bird.png");
    cg::Texture bgTex("assets/background.png");
    cg::Font font("assets/font.ttf", 24);
    cg::Sound jumpSound("assets/jump.wav");

    cg::Vector2 birdPos = {100.0f, 300.0f};
    float birdVelocity = 0.0f;
    const float gravity = 900.0f;
    const float jumpForce = -350.0f;
    const float birdSize = 34.0f;

    std::vector<Pipe> pipes;
    float pipeSpawnTimer = 0.0f;
    const float pipeSpawnInterval = 2.0f;
    const float pipeSpeed = 200.0f;
    const float pipeWidth = 60.0f;
    const float pipeGap = 160.0f;

    int score = 0;
    bool gameOver = false;

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    while (window.isOpen()) {
        float dt = clock.getDeltaTime();

        cg::Event event;
        while (window.pollEvent(event)) {
            if (event.type == cg::EventType::Quit) {
                window.close();
            }

            const bool flapPressed =
                (event.type == cg::EventType::KeyDown && event.key == cg::Key::Space) ||
                (event.type == cg::EventType::MouseButtonDown && event.mouseButton == cg::MouseButton::Left);

            if (flapPressed) {
                if (!gameOver) {
                    birdVelocity = jumpForce;
                    jumpSound.play();
                } else {
                    birdPos = {100.0f, 300.0f};
                    birdVelocity = 0.0f;
                    pipes.clear();
                    score = 0;
                    gameOver = false;
                }
            }
        }

        if (!gameOver) {
            birdVelocity += gravity * dt;
            birdPos.y += birdVelocity * dt;

            pipeSpawnTimer += dt;
            if (pipeSpawnTimer >= pipeSpawnInterval) {
                pipeSpawnTimer = 0.0f;
                float minY = 100.0f;
                float maxY = 400.0f;
                float topHeight = minY + static_cast<float>(std::rand()) / (static_cast<float>(RAND_MAX / (maxY - minY)));
                
                Pipe p;
                p.x = 400.0f;
                p.topRect = {p.x, 0.0f, pipeWidth, topHeight};
                p.bottomRect = {p.x, topHeight + pipeGap, pipeWidth, 600.0f - (topHeight + pipeGap)};
                pipes.push_back(p);
            }

            cg::Rect birdRect = {birdPos.x, birdPos.y, birdSize, birdSize};

            for (auto it = pipes.begin(); it != pipes.end();) {
                it->x -= pipeSpeed * dt;
                it->topRect.x = it->x;
                it->bottomRect.x = it->x;

                if (birdRect.colliderect(it->topRect) || birdRect.colliderect(it->bottomRect)) {
                    gameOver = true;
                }

                if (!it->passed && it->x + pipeWidth < birdPos.x) {
                    score++;
                    it->passed = true;
                }

                if (it->x + pipeWidth < 0) {
                    it = pipes.erase(it);
                } else {
                    ++it;
                }
            }

            if (birdPos.y < 0 || birdPos.y + birdSize > 600.0f) {
                gameOver = true;
            }
        }

        // --- RENDERIZADO CON CG::RENDERER ---
        window.clear(cg::Color(113, 197, 207));

        cg::Renderer::drawSprite(bgTex, {0.0f, 0.0f});

        for (const auto& p : pipes) {
            cg::draw::rect(window, cg::Color(96, 185, 87), p.topRect);
            cg::draw::rect(window, cg::Color(96, 185, 87), p.bottomRect);
        }

        cg::Renderer::drawSprite(birdTex, birdPos);

        cg::Renderer::drawText(font, "Puntuacion: " + std::to_string(score), cg::Color::White, {20.0f, 20.0f});

        if (gameOver) {
            cg::Renderer::drawText(font, "GAME OVER - Pulsa Espacio", cg::Color::Red, {40.0f, 280.0f});
        }

        window.display();
        clock.tick(60);
    }
    }

    cg::quit();
    return 0;
}