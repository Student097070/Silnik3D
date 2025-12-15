#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <locale.h>

using namespace std;

class Engine {
private:
    GLFWwindow* window;
    int width, height;
    bool isFullscreen;
    bool isPerspective;
    bool vsyncEnabled;
    bool depthTestEnabled;
    float clearColor[4];
    int targetFPS;
    double lastFrameTime;
    bool rotateCamera;  // Dodane: stan obrotu kamery

    // Matryca projekcji (uproszczona)
    float projectionMatrix[16];
    float orthoMatrix[16];
    float perspectiveMatrix[16];

    void setIdentityMatrix(float* matrix) {
        for (int i = 0; i < 16; i++) {
            matrix[i] = 0.0f;
        }
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
    }
   
    void setOrthographic(float left, float right, float bottom, float top, float near, float far) {
        setIdentityMatrix(orthoMatrix);
        orthoMatrix[0] = 2.0f / (right - left);
        orthoMatrix[5] = 2.0f / (top - bottom);
        orthoMatrix[10] = -2.0f / (far - near);
        orthoMatrix[12] = -(right + left) / (right - left);
        orthoMatrix[13] = -(top + bottom) / (top - bottom);
        orthoMatrix[14] = -(far + near) / (far - near);
    }

    void setPerspective(float fov, float aspect, float near, float far) {
        setIdentityMatrix(perspectiveMatrix);
        float f = 1.0f / tan(fov * 3.14159f / 360.0f);

        perspectiveMatrix[0] = f / aspect;
        perspectiveMatrix[5] = f;
        perspectiveMatrix[10] = (far + near) / (near - far);
        perspectiveMatrix[11] = -1.0f;
        perspectiveMatrix[14] = (2.0f * far * near) / (near - far);
        perspectiveMatrix[15] = 0.0f;
    }

    void applyProjectionMatrix() {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(projectionMatrix);
        glMatrixMode(GL_MODELVIEW);
    }

    void updateProjection() {
        float aspect = (float)width / (float)height;

        if (isPerspective) {
            // Perspektywiczne (60 stopni FOV)
            setPerspective(60.0f, aspect, 0.1f, 100.0f);
            for (int i = 0; i < 16; i++) {
                projectionMatrix[i] = perspectiveMatrix[i];
            }
        }
        else {
            // Ortogonalne
            setOrthographic(-10.0f * aspect, 10.0f * aspect, -10.0f, 10.0f, 0.1f, 100.0f);
            for (int i = 0; i < 16; i++) {
                projectionMatrix[i] = orthoMatrix[i];
            }
        }
        applyProjectionMatrix();
    }
    void printControlInfo() {
        std::cout << "\n=== KONTROLA SILNIKA 3D ===\n";
        std::cout << "STEROWANIE KLAWIATURĄ:\n";
        std::cout << "  [ESC]     - Zamknij aplikację\n";
        std::cout << "  [P]       - Przełącz rzutowanie (perspektywiczne/ortogonalne)\n";
        std::cout << "  [F]       - Przełącz pełny ekran/okno\n";
        std::cout << "  [V]       - Włącz/wyłącz VSync\n";
        std::cout << "  [D]       - Włącz/wyłącz test głębokości (Z-buffer)\n";
        std::cout << "  [C]       - Zmień losowy kolor tła\n";
        std::cout << "  [R]       - Resetuj widok (kolor i rzutowanie)\n";
        std::cout << "  [SPACJA]  - Włącz/wyłącz obrót kamery\n";
        std::cout << "  [↑]/[↓]   - Zwiększ/zmniejsz limit FPS (+/-10)\n";
        std::cout << "\n STEROWANIE MYSZĄ:\ n";
        std::cout << "  [Lewy przycisk]    - Wyświetl pozycję kursora\n";
        std::cout << "  [Prawy przycisk]   - Wyświetl pozycję kursora\n";
        std::cout << "  [Środkowy przycisk]- Wyświetl pozycję kursora\n";
        std::cout << "  [Kółko myszy]      - Wyświetl przesunięcie\n";
        std::cout << "\nINFORMACJE:\n";
        std::cout << "  Aktualne okno: " << width << "x" << height << "\n";
        std::cout << "  Rzutowanie: " << (isPerspective ? "Perspektywiczne" : "Ortogonalne") << "\n";
        std::cout << "  Tryb: " << (isFullscreen ? "Pełny ekran" : "Okno") << "\n";
        std::cout << "  VSync: " << (vsyncEnabled ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "  Test głębokości: " << (depthTestEnabled ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "  Celowy FPS: " << targetFPS << "\n";
        std::cout << "  Obrót kamery: " << (rotateCamera ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "=============================\n" << std::endl;
    }
public:
    Engine(int w = 800, int h = 600, const char* title = "3D Engine")
        : width(w), height(h), isFullscreen(false), isPerspective(true),
        vsyncEnabled(true), depthTestEnabled(true), targetFPS(60),
        lastFrameTime(0), rotateCamera(false) {  // Inicjalizacja rotateCamera

        // Inicjalizacja generatora liczb losowych
        srand(static_cast<unsigned>(time(nullptr)));

        // Domyślny kolor tła
        clearColor[0] = 0.2f;
        clearColor[1] = 0.3f;
        clearColor[2] = 0.3f;
        clearColor[3] = 1.0f;

        // Inicjalizacja GLFW
        if (!glfwInit()) {
            std::cerr << "Błąd inicjalizacji GLFW!" << std::endl;
            return;
        }

        // Konfiguracja OpenGL
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        // Utworzenie okna
        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!window) {
            std::cerr << "Błąd tworzenia okna!" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);

        // VSync
        glfwSwapInterval(vsyncEnabled ? 1 : 0);

        // Ustawienie callbacków
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallbackStatic);
        glfwSetMouseButtonCallback(window, mouseCallbackStatic);
        glfwSetScrollCallback(window, scrollCallbackStatic);
        glfwSetFramebufferSizeCallback(window, resizeCallbackStatic);
        glfwSetWindowCloseCallback(window, closeCallbackStatic);

        // Inicjalizacja OpenGL
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Inicjalizacja macierzy projekcji
        updateProjection();

        std::cout << "\n=== KONTROLA SILNIKA 3D ===\n";
        std::cout << "STEROWANIE KLAWIATURĄ:\n";
        std::cout << "  [ESC]     - Zamknij aplikację\n";
        std::cout << "  [P]       - Przełącz rzutowanie (perspektywiczne/ortogonalne)\n";
        std::cout << "  [F]       - Przełącz pełny ekran/okno\n";
        std::cout << "  [V]       - Włącz/wyłącz VSync\n";
        std::cout << "  [D]       - Włącz/wyłącz test głębokości (Z-buffer)\n";
        std::cout << "  [C]       - Zmień losowy kolor tła\n";
        std::cout << "  [R]       - Resetuj widok (kolor i rzutowanie)\n";
        std::cout << "  [H]       - Wyświetlanie pomocy\n";
        std::cout << "  [SPACJA]  - Włącz/wyłącz obrót kamery\n";
        std::cout << "  [↑]/[↓]   - Zwiększ/zmniejsz limit FPS (+/-10)\n";
        std::cout << "\n STEROWANIE MYSZĄ:\ n";
        std::cout << "  [Lewy przycisk]    - Wyświetl pozycję kursora\n";
        std::cout << "  [Prawy przycisk]   - Wyświetl pozycję kursora\n";
        std::cout << "  [Środkowy przycisk]- Wyświetl pozycję kursora\n";
        std::cout << "  [Kółko myszy]      - Wyświetl przesunięcie\n";
        std::cout << "\nINFORMACJE:\n";
        std::cout << "  Aktualne okno: " << width << "x" << height << "\n";
        std::cout << "  Rzutowanie: " << (isPerspective ? "Perspektywiczne" : "Ortogonalne") << "\n";
        std::cout << "  Tryb: " << (isFullscreen ? "Pełny ekran" : "Okno") << "\n";
        std::cout << "  VSync: " << (vsyncEnabled ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "  Test głębokości: " << (depthTestEnabled ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "  Celowy FPS: " << targetFPS << "\n";
        std::cout << "  Obrót kamery: " << (rotateCamera ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "=============================\n" << std::endl;
    }

    ~Engine() {
        shutdown();
    }

    void setClearColor(float r, float g, float b, float a = 1.0f) {
        clearColor[0] = r;
        clearColor[1] = g;
        clearColor[2] = b;
        clearColor[3] = a;
    }

    void clearScreen() {
        glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void toggleFullscreen() {
        isFullscreen = !isFullscreen;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);

        if (isFullscreen) {
            glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
            width = mode->width;
            height = mode->height;
        }
        else {
            glfwSetWindowMonitor(window, NULL, 100, 100, 800, 600, 0);
            width = 800;
            height = 600;
        }

        updateProjection();
        std::cout << "Tryb: " << (isFullscreen ? "Pełny ekran" : "Okno") << std::endl;
    }

    void toggleProjection() {
        isPerspective = !isPerspective;
        updateProjection();
        std::cout << "Rzutowanie: " << (isPerspective ? "Perspektywiczne" : "Ortogonalne") << std::endl;
    }

    void toggleVSync() {
        vsyncEnabled = !vsyncEnabled;
        glfwSwapInterval(vsyncEnabled ? 1 : 0);
        std::cout << "VSync: " << (vsyncEnabled ? "Włączony" : "Wyłączony") << std::endl;
    }

    void toggleDepthTest() {
        depthTestEnabled = !depthTestEnabled;
        if (depthTestEnabled) {
            glEnable(GL_DEPTH_TEST);
        }
        else {
            glDisable(GL_DEPTH_TEST);
        }
        std::cout << "Test głębokości: " << (depthTestEnabled ? "Włączony" : "Wyłączony") << std::endl;
    }

    void setTargetFPS(int fps) {
        targetFPS = fps;
        std::cout << "Celowa liczba FPS: " << targetFPS << std::endl;
    }

    void limitFPS() {
        double targetFrameTime = 1.0 / targetFPS;
        double currentTime = glfwGetTime();
        double elapsed = currentTime - lastFrameTime;

        if (elapsed < targetFrameTime) {
            double sleepTime = (targetFrameTime - elapsed) * 1000.0;
            if (sleepTime > 0) {
                // Proste opóźnienie
                double start = glfwGetTime();
                while (glfwGetTime() - start < sleepTime / 1000.0) {
                    // Busy wait
                }
            }
        }

        lastFrameTime = glfwGetTime();
    }

    void drawCube(float x, float y, float z, float size = 1.0f) {
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(size, size, size);

        glBegin(GL_QUADS);

        // Przód (niebieski)
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);

        // Tył (zielony)
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);

        // Góra (żółty)
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(-0.5f, 0.5f, -0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);

        // Dół (czerwony)
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);

        // Lewo (cyjan)
        glColor3f(0.0f, 1.0f, 1.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, 0.5f);
        glVertex3f(-0.5f, 0.5f, -0.5f);

        // Prawo (magenta)
        glColor3f(1.0f, 0.0f, 1.0f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, -0.5f);
        glVertex3f(0.5f, 0.5f, 0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);

        glEnd();

        glPopMatrix();
    }

    void drawPyramid(float x, float y, float z, float size = 1.0f) {
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(size, size, size);

        glBegin(GL_TRIANGLES);

        // Podstawa
        glColor3f(1.0f, 0.5f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);

        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);

        // Ściany boczne
        glColor3f(0.0f, 0.8f, 0.8f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, -0.5f);

        glColor3f(0.8f, 0.8f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(0.5f, -0.5f, -0.5f);
        glVertex3f(0.5f, -0.5f, 0.5f);

        glColor3f(0.8f, 0.0f, 0.8f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, 0.5f);

        glColor3f(0.0f, 0.8f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f);
        glVertex3f(-0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f);

        glEnd();

        glPopMatrix();
    }

    void run() {
        double rotation = 0.0;

        while (!glfwWindowShouldClose(window)) {
            // Ograniczenie FPS
            limitFPS();

            // Czyszczenie ekranu
            clearScreen();

            // Resetuj macierz model-widok
            glLoadIdentity();

            // Ustawienie kamery
            if (rotateCamera) {
                rotation += 1.0;
                if (rotation >= 360.0) rotation -= 360.0;
            }

            glTranslatef(0.0f, 0.0f, -10.0f);
            glRotatef(rotation * 0.5f, 0.0f, 1.0f, 0.0f);
            glRotatef(20.0f, 1.0f, 0.0f, 0.0f);

            // Rysowanie obiektów 3D
            drawCube(-2.0f, 0.0f, 0.0f, 1.0f);
            drawPyramid(2.0f, 0.0f, 0.0f, 1.5f);

            // Rysowanie siatki referencyjnej
            glBegin(GL_LINES);
            glColor3f(0.5f, 0.5f, 0.5f);
            for (int i = -5; i <= 5; i++) {
                glVertex3f(i, -5.0f, 0.0f);
                glVertex3f(i, 5.0f, 0.0f);
                glVertex3f(-5.0f, i, 0.0f);
                glVertex3f(5.0f, i, 0.0f);
            }
            glEnd();

            // Osie współrzędnych
            glBegin(GL_LINES);
            // Oś X (czerwona)
            glColor3f(1.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(3.0f, 0.0f, 0.0f);
            // Oś Y (zielona)
            glColor3f(0.0f, 1.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 3.0f, 0.0f);
            // Oś Z (niebieska)
            glColor3f(0.0f, 0.0f, 1.0f);
            glVertex3f(0.0f, 0.0f, 0.0f);
            glVertex3f(0.0f, 0.0f, 3.0f);
            glEnd();

            // Zamiana buforów
            glfwSwapBuffers(window);

            // Sprawdzenie zdarzeń
            glfwPollEvents();
        }
    }

    void shutdown() {
        std::cout << "Zamykanie silnika..." << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        std::cout << "Silnik zamknięty." << std::endl;
    }

    // Statyczne funkcje callback dla GLFW
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine) engine->keyCallback(key, action);
    }

    static void mouseCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine) engine->mouseCallback(button, action);
    }

    static void scrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine) engine->scrollCallback(xoffset, yoffset);
    }

    static void resizeCallbackStatic(GLFWwindow* window, int width, int height) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine) engine->resizeCallback(width, height);
    }

    static void closeCallbackStatic(GLFWwindow* window) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine) engine->closeCallback();
    }

private:
    void keyCallback(int key, int action) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, GLFW_TRUE);
                break;
            case GLFW_KEY_H:  // Dodane: klawisz pomocy
                printControlInfo();
                break;
            case GLFW_KEY_P:
                toggleProjection();
                break;
            case GLFW_KEY_F:
                toggleFullscreen();
                break;
            case GLFW_KEY_V:
                toggleVSync();
                break;
            case GLFW_KEY_D:
                toggleDepthTest();
                break;
            case GLFW_KEY_C:
                setClearColor(
                    (float)rand() / RAND_MAX,
                    (float)rand() / RAND_MAX,
                    (float)rand() / RAND_MAX,
                    1.0f
                );
                std::cout << "Zmieniono kolor tła" << std::endl;
                break;
            case GLFW_KEY_R:
                setClearColor(0.2f, 0.3f, 0.3f, 1.0f);
                updateProjection();
                std::cout << "Zresetowano widok" << std::endl;
                break;
            case GLFW_KEY_SPACE:
                rotateCamera = !rotateCamera;  // Poprawione: użycie zmiennej członkowskiej
                std::cout << "Obrót kamery: " << (rotateCamera ? "Włączony" : "Wyłączony") << std::endl;
                break;
            case GLFW_KEY_UP:
                targetFPS += 10;
                std::cout << "Celowe FPS: " << targetFPS << std::endl;
                break;
            case GLFW_KEY_DOWN:
                if (targetFPS > 10) {
                    targetFPS -= 10;
                    std::cout << "Celowe FPS: " << targetFPS << std::endl;
                }
                break;
            }
        }
    }

    void mouseCallback(int button, int action) {
        if (action == GLFW_PRESS) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            switch (button) {
            case GLFW_MOUSE_BUTTON_LEFT:
                std::cout << "Lewy przycisk myszy: (" << x << ", " << y << ")" << std::endl;
                break;
            case GLFW_MOUSE_BUTTON_RIGHT:
                std::cout << "Prawy przycisk myszy: (" << x << ", " << y << ")" << std::endl;
                break;
            case GLFW_MOUSE_BUTTON_MIDDLE:
                std::cout << "Środkowy przycisk myszy: (" << x << ", " << y << ")" << std::endl;
                break;
            }
        }
    }

    void scrollCallback(double xoffset, double yoffset) {
        std::cout << "Kółko myszy: x=" << xoffset << ", y=" << yoffset << std::endl;
    }

    void resizeCallback(int w, int h) {
        width = w;
        height = h;
        glViewport(0, 0, w, h);
        updateProjection();
        std::cout << "Rozmiar okna: " << w << "x" << h << std::endl;
    }

    void closeCallback() {
        std::cout << "Zamykanie aplikacji..." << std::endl;
    }
};

int main() {
    // Utworzenie i uruchomienie silnika
    setlocale(LC_CTYPE, "Polish");
    Engine engine(1024, 768, "3D Game Engine - GLFW Implementation");
    engine.run();

    return 0;
}