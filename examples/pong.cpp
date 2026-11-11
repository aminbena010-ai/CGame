#include <CGame/CGame.hpp>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <string>

namespace {

constexpr float kWinW = 800.0f;
constexpr float kWinH = 600.0f;

constexpr float kPaddleW = 15.0f;
constexpr float kPaddleH = 90.0f;
constexpr float kPaddleSpeed = 420.0f;
constexpr float kPaddleMargin = 30.0f;

constexpr float kBallSize = 15.0f;
constexpr float kBallSpeedStart = 340.0f;
constexpr float kBallSpeedMax = 760.0f;

constexpr int kWinScore = 11;

void drawCentered(const cg::Font& font, const std::string& text, const cg::Color& color, float y,
                  float centerX = kWinW * 0.5f) {
    const float pixel = std::max(1.0f, static_cast<float>(font.getFontSize()) / 7.0f);
    const float advance = 6.0f * pixel;
    const float width = advance * static_cast<float>(text.size()) - pixel;
    cg::Renderer::drawText(font, text, color, {centerX - width * 0.5f, y});
}

} // namespace

int main() {
    cg::init();
    {
        cg::Window window("CGame++ - Pong", static_cast<int>(kWinW), static_cast<int>(kWinH));
        cg::Clock clock;

        cg::Font scoreFont("assets/font.ttf", 56);
        cg::Font uiFont("assets/font.ttf", 24);

        cg::Rect leftPaddle{kPaddleMargin, (kWinH - kPaddleH) * 0.5f, kPaddleW, kPaddleH};
        cg::Rect rightPaddle{kWinW - kPaddleMargin - kPaddleW, (kWinH - kPaddleH) * 0.5f, kPaddleW, kPaddleH};

        cg::Rect ball{(kWinW - kBallSize) * 0.5f, (kWinH - kBallSize) * 0.5f, kBallSize, kBallSize};
        cg::Vector2 ballVelocity{0.0f, 0.0f};
        float ballSpeed = kBallSpeedStart;

        int leftScore = 0;
        int rightScore = 0;
        bool gameOver = false;
        float serveTimer = 0.6f;
        int serveDirection = 1;

        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        while (window.isOpen()) {
            const float dt = clock.getDeltaTime();

            cg::Event event;
            while (window.pollEvent(event)) {
                if (event.type == cg::EventType::Quit) {
                    window.close();
                } else if (event.type == cg::EventType::KeyDown && event.key == cg::Key::Escape) {
                    window.close();
                } else if (event.type == cg::EventType::KeyDown && event.key == cg::Key::Space && gameOver) {
                    leftScore = 0;
                    rightScore = 0;
                    gameOver = false;
                    ballSpeed = kBallSpeedStart;
                    ball.x = (kWinW - kBallSize) * 0.5f;
                    ball.y = (kWinH - kBallSize) * 0.5f;
                    ballVelocity = {0.0f, 0.0f};
                    serveDirection = (std::rand() % 2 == 0) ? 1 : -1;
                    serveTimer = 0.6f;
                }
            }

            if (!gameOver) {
                // Jugador izquierdo: W / S. Jugador derecho: Flecha arriba / Flecha abajo.
                if (cg::key::isPressed(cg::Key::W)) leftPaddle.y -= kPaddleSpeed * dt;
                if (cg::key::isPressed(cg::Key::S)) leftPaddle.y += kPaddleSpeed * dt;
                if (cg::key::isPressed(cg::Key::Up)) rightPaddle.y -= kPaddleSpeed * dt;
                if (cg::key::isPressed(cg::Key::Down)) rightPaddle.y += kPaddleSpeed * dt;

                leftPaddle.y = std::clamp(leftPaddle.y, 0.0f, kWinH - kPaddleH);
                rightPaddle.y = std::clamp(rightPaddle.y, 0.0f, kWinH - kPaddleH);

                if (serveTimer > 0.0f) {
                    serveTimer -= dt;
                    if (serveTimer <= 0.0f) {
                        const float angle = static_cast<float>(std::rand() % 120 - 60) * 3.14159265f / 180.0f;
                        ballVelocity = {std::cos(angle) * ballSpeed * static_cast<float>(serveDirection),
                                        std::sin(angle) * ballSpeed};
                    }
                } else {
                    ball.x += ballVelocity.x * dt;
                    ball.y += ballVelocity.y * dt;

                    if (ball.y <= 0.0f) {
                        ball.y = 0.0f;
                        ballVelocity.y = std::abs(ballVelocity.y);
                    } else if (ball.y + ball.h >= kWinH) {
                        ball.y = kWinH - ball.h;
                        ballVelocity.y = -std::abs(ballVelocity.y);
                    }

                    if (ballVelocity.x < 0.0f && ball.colliderect(leftPaddle)) {
                        ball.x = leftPaddle.x + leftPaddle.w;
                        ballSpeed = std::min(ballSpeed * 1.05f, kBallSpeedMax);
                        const float offset = (ball.y + ball.h * 0.5f - (leftPaddle.y + leftPaddle.h * 0.5f)) / (leftPaddle.h * 0.5f);
                        ballVelocity = {ballSpeed, offset * 420.0f};
                    } else if (ballVelocity.x > 0.0f && ball.colliderect(rightPaddle)) {
                        ball.x = rightPaddle.x - ball.w;
                        ballSpeed = std::min(ballSpeed * 1.05f, kBallSpeedMax);
                        const float offset = (ball.y + ball.h * 0.5f - (rightPaddle.y + rightPaddle.h * 0.5f)) / (rightPaddle.h * 0.5f);
                        ballVelocity = {-ballSpeed, offset * 420.0f};
                    }

                    if (ball.x + ball.w < 0.0f) {
                        rightScore++;
                        gameOver = rightScore >= kWinScore;
                        ballSpeed = kBallSpeedStart;
                        ball.x = (kWinW - kBallSize) * 0.5f;
                        ball.y = (kWinH - kBallSize) * 0.5f;
                        ballVelocity = {0.0f, 0.0f};
                        serveDirection = -1;
                        serveTimer = 0.6f;
                    } else if (ball.x > kWinW) {
                        leftScore++;
                        gameOver = leftScore >= kWinScore;
                        ballSpeed = kBallSpeedStart;
                        ball.x = (kWinW - kBallSize) * 0.5f;
                        ball.y = (kWinH - kBallSize) * 0.5f;
                        ballVelocity = {0.0f, 0.0f};
                        serveDirection = 1;
                        serveTimer = 0.6f;
                    }
                }
            }

            const cg::Color white(240, 240, 240);

            window.clear(cg::Color::Black);

            // Red central discontinua
            for (float y = 0.0f; y < kWinH; y += 32.0f) {
                cg::draw::rect(window, cg::Color(50, 50, 50), {kWinW * 0.5f - 3.0f, y, 6.0f, 18.0f});
            }

            cg::draw::rect(window, white, leftPaddle);
            cg::draw::rect(window, white, rightPaddle);
            cg::draw::rect(window, white, ball);

            drawCentered(scoreFont, std::to_string(leftScore), white, 24.0f, kWinW * 0.5f - 110.0f);
            drawCentered(scoreFont, std::to_string(rightScore), white, 24.0f, kWinW * 0.5f + 110.0f);

            if (gameOver) {
                const std::string winner = (leftScore > rightScore) ? "GANA EL JUGADOR 1" : "GANA EL JUGADOR 2";
                drawCentered(uiFont, winner, cg::Color(255, 255, 0), kWinH * 0.5f - 60.0f);
                drawCentered(uiFont, "PULSA ESPACIO PARA REINICIAR", white, kWinH * 0.5f - 20.0f);
            } else {
                drawCentered(uiFont, "J1: W/S    J2: FLECHAS    ESC: SALIR", cg::Color(140, 140, 140), kWinH - 40.0f);
            }

            window.display();
            clock.tick(60);
        }
    }

    cg::quit();
    return 0;
}
