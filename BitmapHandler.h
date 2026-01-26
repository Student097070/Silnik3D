#pragma once
#ifndef BITMAP_HANDLER_H
#define BITMAP_HANDLER_H

#include <string>
#include <iostream>

/**
 * @brief Klasa obs³uguj¹ca wczytywanie bitmap do pamiêci.
 */
class BitmapHandler {
public:
    /**
     * @brief Konstruktor klasy BitmapHandler.
     */
    BitmapHandler();

    /**
     * @brief Destruktor klasy BitmapHandler.
     */
    ~BitmapHandler();

    /**
     * @brief Blokuje kopiowanie obiektu (zapobiega problemom z pamiêci¹).
     */
    BitmapHandler(const BitmapHandler&) = delete;
    BitmapHandler& operator=(const BitmapHandler&) = delete;

    /**
     * @brief Wczytuje dane tekstury z pliku.
     * @param filePath Œcie¿ka do pliku obrazu.
     * @param flipY Czy odwróciæ obraz w osi Y (domyœlnie true).
     * @return True jeœli wczytanie siê powiod³o.
     */
    bool Load(const std::string& filePath, bool flipY = true);

    /**
     * @brief Zwalnia pamiêæ zajmowan¹ przez dane obrazu.
     */
    void Free();

    /**
     * @brief Zwraca wskaŸnik na dane obrazu.
     * @return WskaŸnik na dane bitmapy.
     */
    unsigned char* GetData() const { return data; }

    /**
     * @brief Zwraca szerokoœæ obrazu.
     * @return Szerokoœæ w pikselach.
     */
    int GetWidth() const { return width; }

    /**
     * @brief Zwraca wysokoœæ obrazu.
     * @return Wysokoœæ w pikselach.
     */
    int GetHeight() const { return height; }

    /**
     * @brief Zwraca liczbê kana³ów obrazu.
     * @return Liczba kana³ów (np. 3 = RGB, 4 = RGBA).
     */
    int GetChannels() const { return channels; }

    /**
     * @brief Zwraca ³¹czny rozmiar danych obrazu w bajtach.
     * @return Rozmiar danych w bajtach.
     */
    size_t GetTotalSize() const { return (size_t)width * height * channels; }

private:
    unsigned char* data; /**< WskaŸnik na dane bitmapy */
    int width;           /**< Szerokoœæ obrazu w pikselach */
    int height;          /**< Wysokoœæ obrazu w pikselach */
    int channels;        /**< Liczba kana³ów obrazu */
};

#endif
