#include "BitmapHandler.h"

// Implementacja biblioteki musi byæ w pliku .cpp
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

BitmapHandler::BitmapHandler()
    : data(nullptr), width(0), height(0), channels(0) {}

BitmapHandler::~BitmapHandler() {
    Free();
}

bool BitmapHandler::Load(const std::string& filePath, bool flipY) {
    // Upewniamy siê, ¿e poprzednie dane zosta³y wyczyszczone
    Free();

    // Standard w 3D: OpenGL oczekuje tekstur odwróconych pionowo
    stbi_set_flip_vertically_on_load(flipY);

    // Wczytywanie pliku
    data = stbi_load(filePath.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "[BitmapHandler Error] Failed to load: " << filePath
            << " | Reason: " << stbi_failure_reason() << std::endl;
        return false;
    }

    // Opcjonalnie: logowanie sukcesu (przydatne przy debugowaniu silnika)
    // std::cout << "[BitmapHandler] Loaded: " << filePath << " (" << width << "x" << height << ", " << channels << " channels)" << std::endl;

    return true;
}

void BitmapHandler::Free() {
    if (data) {
        stbi_image_free(data);
        data = nullptr;
        width = 0;
        height = 0;
        channels = 0;
    }
}
