#pragma once
#ifndef BITMAP_HANDLER_H
#define BITMAP_HANDLER_H

#include <string>
#include <iostream>

class BitmapHandler {
public:
    BitmapHandler();
    ~BitmapHandler();

    // Blokujemy kopiowanie obiektu, aby unikn¹æ problemów z pamiêci¹ (pointer data)
    BitmapHandler(const BitmapHandler&) = delete;
    BitmapHandler& operator=(const BitmapHandler&) = delete;

    // Wczytuje dane tekstury z pliku
    bool Load(const std::string& filePath, bool flipY = true);

    // Zwalnia pamiêæ zajmowan¹ przez dane obrazu
    void Free();

    // Gettery
    unsigned char* GetData() const { return data; }
    int GetWidth() const { return width; }
    int GetHeight() const { return height; }
    int GetChannels() const { return channels; }

    // Dodatkowy pomocnik: zwraca rozmiar danych w bajtach
    size_t GetTotalSize() const { return (size_t)width * height * channels; }

private:
    unsigned char* data;
    int width;
    int height;
    int channels;
};

#endif
