#pragma once

#include <memory>

class GraphicsImage;

void GameOver_Setup();
std::unique_ptr<GraphicsImage> CreateWinnerCertificate();
