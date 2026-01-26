#include "BitmapHandler.h"

/**
 * @brief Implementacja biblioteki stb_image.
 */
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

 /**
  * @brief Konstruktor klasy BitmapHandler.
  */
BitmapHandler::BitmapHandler()
    : data(nullptr), width(0), height(0), channels(0) {
}

/**
 * @brief Destruktor klasy BitmapHandler.
 */
BitmapHandler::~BitmapHandler() {
    Free();
}

/**
 * @brief Wczytuje plik graficzny do pamiêci.
 * @param filePath Œcie¿ka do pliku obrazu.
 * @param flipY Czy odwróciæ obraz w osi Y.
 * @return True jeœli wczytanie siê powiod³o.
 */
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

/**
 * @brief Zwalnia pamiêæ zajêt¹ przez obraz.
 */
void BitmapHandler::Free() {
    if (data) {
        stbi_image_free(data);
        data = nullptr;
        width = 0;
        height = 0;
        channels = 0;
    }
}
