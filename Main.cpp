#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <locale.h>
#include <vector>

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "stb_image.h"
GLuint myTexture; // Globalny identyfikator tekstury
#include "BitmapHandler.h" // Upewnij się, że masz ten include



// ============== KLASA PLAYER ==============
class Player {
public:
    // === KAMERA ===
    enum CameraMode {
        STATIC_CAMERA,    // Domyślny tryb z obrotem
        FPS_CAMERA,       // Tryb FPS (WASD + mysz)
        MANUAL_CAMERA     // Tryb ręczny (klawisze I/K/J/L/U/O)
    };


private:
    // === KAMERA ===

    CameraMode cameraMode;
    GLFWwindow* window;

    // Zmienne dla trybu FPS
    float camX = 0.0f, camY = 0.0f, camZ = 10.0f;
    float yaw = -90.0f;    // obrót w poziomie
    float pitch = 0.0f;    // obrót w pionie
    float moveSpeed = 10.0f;
    float mouseSensitivity = 0.1f;
    bool firstMouse = true;
    double lastMouseX = 0.0, lastMouseY = 0.0;

    // Zmienne dla trybu statycznego
    double staticRotation = 0.0;
    bool rotateCamera = false;

    // Zmienne dla scrolla myszy
    float camZScroll = 10.0f;
    const float scrollSpeed = 1.0f;
    const float minZ = -50.0f;
    const float maxZ = 50.0f;

    // === OŚWIETLENIE ===
    bool lightingEnabled;
    bool shadowsEnabled;  // dla przyszłych implementacji cieni
    bool smoothShading;
    float lightPosition[4];
    float lightAmbient[4];
    float lightDiffuse[4];
    float lightSpecular[4];

    // === INNE ===
    bool showAxes;

public:
    Player(GLFWwindow* win) : window(win) {
        cameraMode = STATIC_CAMERA;
        rotateCamera = false;
        lightingEnabled = true;
        shadowsEnabled = false;
        smoothShading = true;
        showAxes = false;

        // Domyślne parametry światła
        lightPosition[0] = -5.0f; lightPosition[1] = 10.0f;
        lightPosition[2] = 5.0f; lightPosition[3] = 0.0f; // światło kierunkowe

        lightAmbient[0] = 0.2f; lightAmbient[1] = 0.2f;
        lightAmbient[2] = 0.2f; lightAmbient[3] = 1.0f;

        lightDiffuse[0] = 0.8f; lightDiffuse[1] = 0.8f;
        lightDiffuse[2] = 0.8f; lightDiffuse[3] = 1.0f;

        lightSpecular[0] = 1.0f; lightSpecular[1] = 1.0f;
        lightSpecular[2] = 1.0f; lightSpecular[3] = 1.0f;

        // Ustawienie kursoru
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    // === METODY KAMERY ===
    void setCameraMode(CameraMode mode) {
        cameraMode = mode;

        if (cameraMode == FPS_CAMERA) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
            std::cout << "TRYB KAMERY: FPS (W/S=Y, A/D=X, mysz=obrót, scroll=Z)" << std::endl;
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            if (cameraMode == STATIC_CAMERA) {
                std::cout << "TRYB KAMERY: STATYCZNY (obrót: SPACJA)" << std::endl;
            }
            else {
                std::cout << "TRYB KAMERY: RĘCZNY (I/K=Y, J/L=X, U/O=Z)" << std::endl;
            }
        }
    }

    void handleCameraMovement(float deltaTime) {
        if (cameraMode == FPS_CAMERA) {
            float velocity = moveSpeed * deltaTime;

            if (camZScroll >= 0) {
                // Normalne sterowanie - przed obiektami
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camY += velocity;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camY -= velocity;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camX -= velocity;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camX += velocity;
            }
            else {
                // Odwrócone sterowanie - za obiektami
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camY += velocity;
                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camY -= velocity;
                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camX += velocity;
                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camX -= velocity;
            }
        }
        else if (cameraMode == MANUAL_CAMERA) {
            float velocity = moveSpeed * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) camY += velocity;
            if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) camY -= velocity;
            if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) camX -= velocity;
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) camX += velocity;
            if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
                camZScroll -= velocity;
                if (camZScroll < minZ) camZScroll = minZ;
            }
            if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
                camZScroll += velocity;
                if (camZScroll > maxZ) camZScroll = maxZ;
            }
        }
    }

    void updateStaticRotation(float deltaTime) {
        if (cameraMode == STATIC_CAMERA && rotateCamera) {
            staticRotation += deltaTime * 30.0f; // 30 stopni na sekundę
            if (staticRotation >= 360.0f) staticRotation -= 360.0f;
        }
    }

    void applyCameraTransform() {
        glLoadIdentity();

        if (cameraMode == STATIC_CAMERA) {
            if (camZScroll >= 0) {
                glTranslatef(0.0f, 0.0f, -camZScroll);
            }
            else {
                glTranslatef(0.0f, 0.0f, -camZScroll);
                glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
            }
            glRotatef(staticRotation * 0.5f, 0.0f, 1.0f, 0.0f);
            glRotatef(20.0f, 1.0f, 0.0f, 0.0f);
        }
        else {
            if (cameraMode == FPS_CAMERA) {
                glRotatef(-pitch, 1.0f, 0.0f, 0.0f);
                glRotatef(-yaw, 0.0f, 1.0f, 0.0f);
            }

            if (camZScroll >= 0) {
                glTranslatef(-camX, -camY, -camZScroll);
            }
            else {
                glTranslatef(-camX, -camY, -camZScroll);
                if (cameraMode != FPS_CAMERA) {
                    glRotatef(180.0f, 0.0f, 1.0f, 0.0f);
                }
            }
        }
    }

    void handleMouseMove(double xpos, double ypos) {
        if (cameraMode == FPS_CAMERA) {
            if (firstMouse) {
                lastMouseX = xpos;
                lastMouseY = ypos;
                firstMouse = false;
                return;
            }

            float xoffset = static_cast<float>(xpos - lastMouseX);
            float yoffset = static_cast<float>(lastMouseY - ypos);

            lastMouseX = xpos;
            lastMouseY = ypos;

            xoffset *= mouseSensitivity;
            yoffset *= mouseSensitivity;

            yaw -= xoffset;
            pitch += yoffset;

            if (pitch > 89.0f) pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }
    }

    void handleMouseScroll(float yoffset) {
        camZScroll += yoffset * scrollSpeed;
        if (camZScroll < minZ) camZScroll = minZ;
        if (camZScroll > maxZ) camZScroll = maxZ;
        std::cout << "Pozycja Z: " << camZScroll << std::endl;
    }

    void toggleRotation() {
        if (cameraMode == STATIC_CAMERA) {
            rotateCamera = !rotateCamera;
            std::cout << "Obrót kamery: " << (rotateCamera ? "Włączony" : "Wyłączony") << std::endl;
        }
    }

    void resetCamera() {
        camX = 0.0f;
        camY = 0.0f;
        camZScroll = 10.0f;
        yaw = -90.0f;
        pitch = 0.0f;
        staticRotation = 0.0;
        rotateCamera = false;
        cameraMode = STATIC_CAMERA;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        std::cout << "Kamera zresetowana do pozycji domyślnej" << std::endl;
    }

    // === METODY OŚWIETLENIA ===
    void setupLighting() {
        if (lightingEnabled) {
            glEnable(GL_LIGHTING);
            glEnable(GL_LIGHT0);

            glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
            glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
            glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
            glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

            float mat_specular[] = { 0.5f, 0.5f, 0.5f, 1.0f };
            float mat_shininess[] = { 50.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

            updateLightPosition();
        }
        else {
            glDisable(GL_LIGHTING);
            glDisable(GL_LIGHT0);
        }
    }

    void updateLightPosition() {
        // Aktualizacja pozycji światła (można dodać animację)
        glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
    }

    void toggleLighting() {
        lightingEnabled = !lightingEnabled;
        setupLighting();
        std::cout << "Oświetlenie: " << (lightingEnabled ? "Włączone" : "Wyłączone") << std::endl;
    }

    void setLightPosition(float x, float y, float z, float w = 0.0f) {
        lightPosition[0] = x;
        lightPosition[1] = y;
        lightPosition[2] = z;
        lightPosition[3] = w;
        if (lightingEnabled) {
            glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
        }
    }

    void setLightColor(float r, float g, float b, float a = 1.0f) {
        lightDiffuse[0] = r; lightDiffuse[1] = g; lightDiffuse[2] = b; lightDiffuse[3] = a;
        if (lightingEnabled) {
            glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        }
    }

    // === METODY CIENIOWANIA ===
    void toggleShading() {
        smoothShading = !smoothShading;
        if (smoothShading) {
            glShadeModel(GL_SMOOTH);
            std::cout << "Cieniowanie: Gouraud (smooth)" << std::endl;
        }
        else {
            glShadeModel(GL_FLAT);
            std::cout << "Cieniowanie: Płaskie (flat)" << std::endl;
        }
    }

    bool isSmoothShading() const { return smoothShading; }

    // === METODY OSI ===
    void toggleAxes() {
        showAxes = !showAxes;
        std::cout << "Osie współrzędnych: " << (showAxes ? "Widoczne" : "Ukryte") << std::endl;
    }

    void drawAxes() {
        if (!showAxes) return;

        glDisable(GL_LIGHTING);
        glLineWidth(3.0f);

        glBegin(GL_LINES);
        // Oś X - CZERWONA
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(-10.0f, 0.0f, 0.0f);
        glVertex3f(10.0f, 0.0f, 0.0f);
        // Oś Y - ZIELONA
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, -10.0f, 0.0f);
        glVertex3f(0.0f, 10.0f, 0.0f);
        // Oś Z - NIEBIESKA
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.0f, -10.0f);
        glVertex3f(0.0f, 0.0f, 10.0f);
        glEnd();

        glLineWidth(1.0f);
        if (lightingEnabled) {
            glEnable(GL_LIGHTING);
        }
    }

    // === GETTERY I SETTERY ===
    CameraMode getCameraMode() const { return cameraMode; }
    bool isLightingEnabled() const { return lightingEnabled; }
    bool isShowingAxes() const { return showAxes; }
    float getZPosition() const { return camZScroll; }
    float getXPosition() const { return camX; }
    float getYPosition() const { return camY; }
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    bool isRotating() const { return rotateCamera; }

    void setMoveSpeed(float speed) { moveSpeed = speed; }
    void setMouseSensitivity(float sens) { mouseSensitivity = sens; }

    void printPlayerInfo() const {
        std::cout << "\n=== INFORMACJE GRACZA ===\n";
        std::cout << "Tryb kamery: ";
        switch (cameraMode) {
        case STATIC_CAMERA: std::cout << "STATYCZNY"; break;
        case FPS_CAMERA: std::cout << "FPS"; break;
        case MANUAL_CAMERA: std::cout << "RĘCZNY"; break;
        }
        std::cout << "\nPozycja: (" << camX << ", " << camY << ", " << camZScroll << ")\n";
        std::cout << "Rotacja: yaw=" << yaw << ", pitch=" << pitch << "\n";
        std::cout << "Obrót kamery: " << (rotateCamera ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "Oświetlenie: " << (lightingEnabled ? "Włączone" : "Wyłączone") << "\n";
        std::cout << "Cieniowanie: " << (smoothShading ? "Gouraud" : "Płaskie") << "\n";
        std::cout << "Osie: " << (showAxes ? "Widoczne" : "Ukryte") << "\n";
        std::cout << "Prędkość ruchu: " << moveSpeed << "\n";
        std::cout << "Czułość myszy: " << mouseSensitivity << std::endl;
    }
};

// ============== KLASA ENGINE ==============
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

    // Matryca projekcji
    float projectionMatrix[16];
    float orthoMatrix[16];
    float perspectiveMatrix[16];

    // Player
    Player* player;

    // Zmienne dla kuli
    int sphereSegments = 16;
    const int minSegments = 8;
    const int maxSegments = 64;
    const int baseSegments = 16;

    void setIdentityMatrix(float* matrix) {
        for (int i = 0; i < 16; i++) matrix[i] = 0.0f;
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
            setPerspective(60.0f, aspect, 0.1f, 100.0f);
            for (int i = 0; i < 16; i++) projectionMatrix[i] = perspectiveMatrix[i];
        }
        else {
            setOrthographic(-10.0f * aspect, 10.0f * aspect, -10.0f, 10.0f, 0.1f, 100.0f);
            for (int i = 0; i < 16; i++) projectionMatrix[i] = orthoMatrix[i];
        }
        applyProjectionMatrix();
    }

public:
    Engine(int w = 800, int h = 600, const char* title = "3D Engine")
        : width(w), height(h), isFullscreen(false), isPerspective(true),
        vsyncEnabled(true), depthTestEnabled(true), targetFPS(60),
        lastFrameTime(0) {

        srand(static_cast<unsigned>(time(nullptr)));

        clearColor[0] = 0.2f;
        clearColor[1] = 0.3f;
        clearColor[2] = 0.3f;
        clearColor[3] = 1.0f;

        if (!glfwInit()) {
            std::cerr << "Błąd inicjalizacji GLFW!" << std::endl;
            return;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        window = glfwCreateWindow(width, height, title, NULL, NULL);
        if (!window) {
            std::cerr << "Błąd tworzenia okna!" << std::endl;
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(vsyncEnabled ? 1 : 0);

        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallbackStatic);
        glfwSetMouseButtonCallback(window, mouseCallbackStatic);
        glfwSetScrollCallback(window, scrollCallbackStatic);
        glfwSetFramebufferSizeCallback(window, resizeCallbackStatic);
        glfwSetWindowCloseCallback(window, closeCallbackStatic);
        glfwSetCursorPosCallback(window, mouseMoveCallbackStatic);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_NORMALIZE);

        player = new Player(window);
        updateProjection();
        lastFrameTime = glfwGetTime();

        printControlInfo();
    }

    ~Engine() {
        shutdown();
    }

    void shutdown() {
        std::cout << "Zamykanie silnika..." << std::endl;
        if (player) delete player;
        if (window) glfwDestroyWindow(window);
        glfwTerminate();
        std::cout << "Silnik zamknięty." << std::endl;
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
        if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
        else glDisable(GL_DEPTH_TEST);
        std::cout << "Test głębokości: " << (depthTestEnabled ? "Włączony" : "Wyłączony") << std::endl;
    }

    // Funkcje dla kuli
    void increaseSphereDetail() {
        if (sphereSegments * 2 <= maxSegments) {
            sphereSegments *= 2;
            std::cout << "Zwiększono liczbę segmentów kuli: " << sphereSegments;
            std::cout << " (poligony: ~" << (sphereSegments * sphereSegments * 2) << ")" << std::endl;
        }
        else {
            std::cout << "Osiągnięto maksymalną liczbę segmentów: " << maxSegments << std::endl;
        }
    }

    void decreaseSphereDetail() {
        if (sphereSegments / 2 >= minSegments) {
            sphereSegments /= 2;
            std::cout << "Zmniejszono liczbę segmentów kuli: " << sphereSegments;
            std::cout << " (poligony: ~" << (sphereSegments * sphereSegments * 2) << ")" << std::endl;
        }
        else {
            std::cout << "Osiągnięto minimalną liczbę segmentów: " << minSegments << std::endl;
        }
    }

    void resetSphereDetail() {
        sphereSegments = baseSegments;
        std::cout << "Zresetowano liczbę segmentów kuli: " << sphereSegments;
        std::cout << " (poligony: ~" << (sphereSegments * sphereSegments * 2) << ")" << std::endl;
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
            double sleepTime = (targetFrameTime - elapsed);
            if (sleepTime > 0) {
                double start = glfwGetTime();
                while (glfwGetTime() - start < sleepTime) {}
            }
        }
    }

    // Funkcje rysowania
    void drawCube(float x, float y, float z, float size = 1.0f) {
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(size, size, size);

        // 1. Włączamy teksturowanie i wybieramy teksturę
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, myTexture);

        // 2. Ustawiamy kolor na biały (inaczej tekstura będzie zabarwiona)
        glColor3f(1.0f, 1.0f, 1.0f);

        glBegin(GL_QUADS);

        // Przód
        glNormal3f(0.0f, 0.0f, 1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);

        // Tył
        glNormal3f(0.0f, 0.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);

        // Góra
        glNormal3f(0.0f, 1.0f, 0.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, 0.5f, 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);

        // Dół
        glNormal3f(0.0f, -1.0f, 0.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, -0.5f, -0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);

        // Lewo
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, -0.5f);

        // Prawo
        glNormal3f(1.0f, 0.0f, 0.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, -0.5f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.5f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f);

        glEnd();

        // Wyłączamy teksturowanie po narysowaniu obiektu
        glDisable(GL_TEXTURE_2D);
        glPopMatrix();
    }


    void drawPyramid(float x, float y, float z, float size = 1.0f) {
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(size, size, size);

        glBegin(GL_TRIANGLES);
        // Podstawa
        glNormal3f(0.0f, -1.0f, 0.0f);
        glColor3f(1.0f, 0.5f, 0.0f);
        glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(0.5f, -0.5f, -0.5f); glVertex3f(0.5f, -0.5f, 0.5f);
        glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(0.5f, -0.5f, 0.5f); glVertex3f(-0.5f, -0.5f, 0.5f);
        // Ściany
        glNormal3f(0.0f, 0.447f, 0.894f);
        glColor3f(0.0f, 0.0f, 1.0f);
        glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.5f); glVertex3f(0.5f, -0.5f, 0.5f);
        glNormal3f(0.0f, 0.447f, -0.894f);
        glColor3f(0.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, -0.5f);
        glNormal3f(-0.894f, 0.447f, 0.0f);
        glColor3f(1.0f, 1.0f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f); glVertex3f(-0.5f, -0.5f, 0.5f);
        glNormal3f(0.894f, 0.447f, 0.0f);
        glColor3f(1.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.5f, 0.0f); glVertex3f(0.5f, -0.5f, 0.5f); glVertex3f(0.5f, -0.5f, -0.5f);
        glEnd();

        glPopMatrix();
    }

    void drawSphere(float x, float y, float z, float radius = 1.0f) {
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(radius, radius, radius);

        bool smooth = player->isSmoothShading();

        for (int i = 0; i < sphereSegments; i++) {
            float lat0 = (float)M_PI * (-0.5f + (float)i / sphereSegments);
            float lat1 = (float)M_PI * (-0.5f + (float)(i + 1) / sphereSegments);

            glBegin(GL_QUAD_STRIP);
            for (int j = 0; j <= sphereSegments; j++) {
                float lng = 2.0f * (float)M_PI * (float)j / sphereSegments;

                float x0 = cos(lat0) * cos(lng);
                float y0 = sin(lat0);
                float z0 = cos(lat0) * sin(lng);
                float x1 = cos(lat1) * cos(lng);
                float y1 = sin(lat1);
                float z1 = cos(lat1) * sin(lng);

                glNormal3f(x0, y0, z0);
                if (smooth) {
                    float r = 0.5f + 0.5f * y0;
                    float g = 0.5f + 0.5f * x0;
                    float b = 0.5f + 0.5f * z0;
                    glColor3f(r, g, b);
                }
                else {
                    glColor3f(0.8f, 0.2f, 0.8f);
                }
                glVertex3f(x0, y0, z0);

                glNormal3f(x1, y1, z1);
                if (smooth) {
                    float r = 0.5f + 0.5f * y1;
                    float g = 0.5f + 0.5f * x1;
                    float b = 0.5f + 0.5f * z1;
                    glColor3f(r, g, b);
                }
                glVertex3f(x1, y1, z1);
            }
            glEnd();
        }

        glPopMatrix();
    }
    void LoadMyTexture() {
        BitmapHandler loader;
        if (loader.Load("textura.jpg")) { // Sprawdź czy nazwa pliku się zgadza!
            glGenTextures(1, &myTexture);
            glBindTexture(GL_TEXTURE_2D, myTexture);

            // Niezbędne parametry, aby tekstura nie była czarna
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            // JPG nie ma kanału Alpha, więc wymuszamy GL_RGB
            GLenum format = (loader.GetChannels() == 4) ? GL_RGBA : GL_RGB;

            // Poprawka dla obrazów o wymiarach niebędących potęgą dwójki
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

            glTexImage2D(GL_TEXTURE_2D, 0, format,
                loader.GetWidth(), loader.GetHeight(),
                0, format, GL_UNSIGNED_BYTE, loader.GetData());

            loader.Free();
        }
        else {
            std::cerr << "Blad: Nie znaleziono pliku JPG!" << std::endl;
        }
    }

    void run() {
        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastFrameTime);
            lastFrameTime = currentTime;
            LoadMyTexture();
            player->updateStaticRotation(deltaTime);
            player->handleCameraMovement(deltaTime);
            limitFPS();
            clearScreen();

            player->applyCameraTransform();
            player->drawAxes();

            drawCube(-4.0f, 0.0f, 0.0f, 1.0f);
            drawPyramid(0.0f, 0.0f, 0.0f, 1.5f);
            drawSphere(4.0f, 0.0f, 0.0f, 1.5f);
            glDisable(GL_LIGHTING);
            glBegin(GL_LINES);
            glColor3f(0.5f, 0.5f, 0.5f);
            for (int i = -5; i <= 5; i++) {
                glVertex3f((float)i, -5.0f, 0.0f); glVertex3f((float)i, 5.0f, 0.0f);
                glVertex3f(-5.0f, (float)i, 0.0f); glVertex3f(5.0f, (float)i, 0.0f);
            }
            glEnd();

            if (player->isLightingEnabled()) {
                glEnable(GL_LIGHTING);
                glEnable(GL_LIGHT0);
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }

    void printControlInfo() {
        std::cout << "\n=== KONTROLA SILNIKA 3D ===\n";
        std::cout << "STEROWANIE KLAWIATURĄ:\n";
        std::cout << "  [ESC]     - Zamknij aplikację\n";
        std::cout << "  [1]       - Tryb statyczny kamery (domyślny)\n";
        std::cout << "  [2]       - Tryb FPS kamery (WASD + mysz)\n";
        std::cout << "  [3]       - Tryb ręczny kamery (klawisze)\n";
        std::cout << "  [SPACJA]  - Włącz/wyłącz obrót kamery (tryb statyczny)\n";
        std::cout << "  [W/S]     - Ruch po osi Y (tryb FPS)\n";
        std::cout << "  [A/D]     - Ruch po osi X (tryb FPS)\n";
        std::cout << "  [I/K]     - Ruch po osi Y (tryb ręczny)\n";
        std::cout << "  [J/L]     - Ruch po osi X (tryb ręczny)\n";
        std::cout << "  [U/O]     - Ruch po osi Z (tryb ręczny)\n";
        std::cout << "  [P]       - Przełącz rzutowanie (perspektywiczne/ortogonalne)\n";
        std::cout << "  [F]       - Przełącz pełny ekran/okno\n";
        std::cout << "  [V]       - Włącz/wyłącz VSync\n";
        std::cout << "  [D]       - Włącz/wyłącz test głębokości\n";
        std::cout << "  [C]       - Zmień losowy kolor tła\n";
        std::cout << "  [R]       - Resetuj widok\n";
        std::cout << "  [L]       - Włącz/wyłącz oświetlenie\n";
        std::cout << "  [X]       - Pokaż/ukryj osie współrzędnych\n";
        std::cout << "  [G]       - Przełącz cieniowanie (płaskie/Gouraud)\n";
        std::cout << "  [T]       - Zwiększ liczbę segmentów kuli (x2)\n";
        std::cout << "  [Y]       - Zmniejsz liczbę segmentów kuli (/2)\n";
        std::cout << "  [B]       - Resetuj liczbę segmentów kuli\n";
        std::cout << "  [H]       - Wyświetl pomoc\n";
        std::cout << "  [↑]/[↓]   - Zwiększ/zmniejsz limit FPS (+/-10)\n";
        std::cout << "\nSTEROWANIE MYSZĄ:\n";
        std::cout << "  [Lewy/Prawy/Środkowy przycisk] - Wyświetl pozycję kursora\n";
        std::cout << "  [Kółko myszy] - Ruch po osi Z (przód/tył)\n";
        std::cout << "  [Ruch myszy]  - Rozglądanie się (tryb FPS)\n";
        std::cout << "\nINFORMACJE:\n";
        std::cout << "  Okno: " << width << "x" << height << "\n";
        std::cout << "  Rzutowanie: " << (isPerspective ? "Perspektywiczne" : "Ortogonalne") << "\n";
        std::cout << "  Tryb: " << (isFullscreen ? "Pełny ekran" : "Okno") << "\n";
        std::cout << "  VSync: " << (vsyncEnabled ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "  Test głębokości: " << (depthTestEnabled ? "Włączony" : "Wyłączony") << "\n";
        std::cout << "  Segmenty kuli: " << sphereSegments << "\n";
        std::cout << "  Celowy FPS: " << targetFPS << "\n";
        player->printPlayerInfo();
        std::cout << "=============================\n" << std::endl;
    }

    // Statyczne callbacki
    static void keyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine && action == GLFW_PRESS) engine->keyCallback(key);
    }

    static void mouseCallbackStatic(GLFWwindow* window, int button, int action, int mods) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine && action == GLFW_PRESS) engine->mouseCallback(button);
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

    static void mouseMoveCallbackStatic(GLFWwindow* window, double xpos, double ypos) {
        Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
        if (engine) engine->mouseMoveCallback(xpos, ypos);
    }

private:
    void keyCallback(int key) {
        switch (key) {
        case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GLFW_TRUE); break;
        case GLFW_KEY_H: printControlInfo(); break;
        case GLFW_KEY_P: toggleProjection(); break;
        case GLFW_KEY_F: toggleFullscreen(); break;
        case GLFW_KEY_V: toggleVSync(); break;
        case GLFW_KEY_D: toggleDepthTest(); break;
        case GLFW_KEY_C:
            setClearColor((float)rand() / RAND_MAX, (float)rand() / RAND_MAX, (float)rand() / RAND_MAX, 1.0f);
            std::cout << "Zmieniono kolor tła" << std::endl;
            break;
        case GLFW_KEY_R:
            setClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            isPerspective = true;
            updateProjection();
            player->resetCamera();
            resetSphereDetail();
            std::cout << "Zresetowano widok" << std::endl;
            break;
        case GLFW_KEY_SPACE: player->toggleRotation(); break;
        case GLFW_KEY_UP: targetFPS += 10; std::cout << "Celowe FPS: " << targetFPS << std::endl; break;
        case GLFW_KEY_DOWN: if (targetFPS > 10) { targetFPS -= 10; std::cout << "Celowe FPS: " << targetFPS << std::endl; } break;
        case GLFW_KEY_1: player->setCameraMode(Player::STATIC_CAMERA); break;
        case GLFW_KEY_2: player->setCameraMode(Player::FPS_CAMERA); break;
        case GLFW_KEY_3: player->setCameraMode(Player::MANUAL_CAMERA); break;
        case GLFW_KEY_L: player->toggleLighting(); break;
        case GLFW_KEY_X: player->toggleAxes(); break;
        case GLFW_KEY_G: player->toggleShading(); break;
        case GLFW_KEY_T: increaseSphereDetail(); break;
        case GLFW_KEY_Y: decreaseSphereDetail(); break;
        case GLFW_KEY_B: resetSphereDetail(); break;
        }
    }

    void mouseCallback(int button) {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT: std::cout << "Lewy przycisk: (" << x << ", " << y << ")" << std::endl; break;
        case GLFW_MOUSE_BUTTON_RIGHT: std::cout << "Prawy przycisk: (" << x << ", " << y << ")" << std::endl; break;
        case GLFW_MOUSE_BUTTON_MIDDLE: std::cout << "Środkowy przycisk: (" << x << ", " << y << ")" << std::endl; break;
        }
    }

    void scrollCallback(double xoffset, double yoffset) {
        player->handleMouseScroll(static_cast<float>(-yoffset));
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

    void mouseMoveCallback(double xpos, double ypos) {
        player->handleMouseMove(xpos, ypos);
    }
};


int main() {
    setlocale(LC_CTYPE, "Polish");
    Engine engine(1024, 768, "3D Game Engine with Player Class");
    engine.run();
    return 0;
}