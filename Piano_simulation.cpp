#include <Windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/GLU.h>
#include <gl/GL.h>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <SDL.h>
#include <SDL_mixer.h>




using namespace std;
GLuint whiteKeyTexture;
GLuint blackKeyTexture;


map<char, bool> keyStatus;
map<char, bool>mouseStatus;
map<char, Mix_Chunk*> soundEffects;
// Définition des dimensions des touches du piano
float whiteKeyWidth = 2.4f; // Largeur des touches blanches : 2.4 cm
float whiteKeyHeight = 15.0f;  // Longueur des touches blanches : 15 cm
float whiteKeyDepth = 0.7f;
float blackKeyWidth = 1.4f;    // Largeur des touches noires : 1.4 cm
float blackKeyHeight = 10.0f; // Longueur des touches noires :10 cm
float blackKeyDepth = 0.4f;
float gap = 0.1f;// Espace entre les touches blanches
int numOctaves = 2;
bool isOrtho = true;
bool isDrop = false;
float startTime = 0.0f;
char whiteKeys[] = { 'A', 'S', 'D', 'F', 'G', 'H', 'J' ,'K','L','C','V','B','N','M' };
char blackKeys[] = { 'Q', 'W', 'E', 'R', 'T','Y', 'U', 'I', 'O', 'P' };

struct FallingRectangle {
    float x, y, z;
    float width, height;
    char associatedKey;
    bool isActive;
};
vector<FallingRectangle> rectangles;

struct KeyMapping {
    float xPosition;
    char associatedKey;
};

vector<KeyMapping> whiteKeys_position = {
    {-16.0f, 'A'}, {-13.5f, 'S'}, {-11.0f, 'D'}, {-8.5f, 'F'}, {-6.0f, 'G'},{-3.5f, 'H'}, {-1.0f, 'J'},
    {1.5f, 'K'}, {4.0f, 'L'}, {6.5f, 'C'}, {9.0f, 'V'},{11.5f, 'B'},{14.0f, 'N'},{16.5f,'M'}
};
vector<KeyMapping> blackKeys_position = {
    {-14.8f, 'Q'}, {-12.3f, 'W'}, {-7.3f, 'E'}, {-4.8f, 'R'}, {-2.3f, 'T'},
    {3.0f, 'Y'},{5.2f, 'U'},{10.2f, 'I'},{12.7f, 'O'},{15.2f, 'P'}
};

vector<pair<char, float>> loadSheetMusicFromFile(const string& filename) {
    vector<pair<char, float>> sheetMusic;

    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Can't open file" << filename << endl;
        return sheetMusic;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        char key;
        float time;
        if (iss >> key >> time) {
            sheetMusic.push_back({ key, time });
        }
    }

    return sheetMusic;
}
// Fonction pour charger une texture à partir d'un fichier image
GLuint loadTexture(const char* filePath) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Échec du chargement de la texture : " << filePath << std::endl;
        return 0;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Définir les paramètres de texture (enveloppe et filtrage)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Charger les données de l'image dans OpenGL
    if (nrChannels == 3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    else if (nrChannels == 4)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    else {
        std::cout << "Format de texture non pris en charge :" << filePath << std::endl;
        stbi_image_free(data);
        return 0;
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    return texture;
}

void initializeAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        cerr << "Échec d'initialisation de SDL : " << SDL_GetError() << endl;
        exit(1);
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "Échec d'ouverture de Mix_OpenAudio : " << Mix_GetError() << endl;
        exit(1);
    }
    Mix_AllocateChannels(100);
    // Charger les fichiers audio pour chaque touche
    soundEffects['A'] = Mix_LoadWAV("sounds/A40.wav");
    soundEffects['S'] = Mix_LoadWAV("sounds/S42.wav");
    soundEffects['D'] = Mix_LoadWAV("sounds/D44.wav");
    soundEffects['F'] = Mix_LoadWAV("sounds/F45.wav");
    soundEffects['G'] = Mix_LoadWAV("sounds/G47.wav");
    soundEffects['H'] = Mix_LoadWAV("sounds/H49.wav");
    soundEffects['J'] = Mix_LoadWAV("sounds/J51.wav");
    soundEffects['K'] = Mix_LoadWAV("sounds/K52.wav");
    soundEffects['L'] = Mix_LoadWAV("sounds/L54.wav");
    soundEffects['C'] = Mix_LoadWAV("sounds/C56.wav");
    soundEffects['V'] = Mix_LoadWAV("sounds/V57.wav");
    soundEffects['B'] = Mix_LoadWAV("sounds/B59.wav");
    soundEffects['N'] = Mix_LoadWAV("sounds/N61.wav");
    soundEffects['M'] = Mix_LoadWAV("sounds/M63.wav");

    soundEffects['Q'] = Mix_LoadWAV("sounds/Q41.wav");
    soundEffects['W'] = Mix_LoadWAV("sounds/W43.wav");
    soundEffects['E'] = Mix_LoadWAV("sounds/E46.wav");
    soundEffects['R'] = Mix_LoadWAV("sounds/R48.wav");
    soundEffects['T'] = Mix_LoadWAV("sounds/T50.wav");
    soundEffects['Y'] = Mix_LoadWAV("sounds/Y53.wav");
    soundEffects['U'] = Mix_LoadWAV("sounds/U55.wav");
    soundEffects['I'] = Mix_LoadWAV("sounds/I58.wav");
    soundEffects['O'] = Mix_LoadWAV("sounds/O60.wav");
    soundEffects['P'] = Mix_LoadWAV("sounds/P62.wav");

    // Vérifiez que tous les sons sont chargés
    for (auto& pair : soundEffects) {
        if (!pair.second) {
            cerr << "Impossible de charger le fichier audio pour la touche " << pair.first << ": " << Mix_GetError() << endl;
        }
    }
}

void cleanupAudio() {
    for (auto& pair : soundEffects) {
        Mix_FreeChunk(pair.second);
    }
    Mix_CloseAudio();
    SDL_Quit();
}

void drawText(const char* text, float x, float y, float z, float r, float g, float b) {
    glColor3f(r, g, b);
    glRasterPos3f(x, y, z);
    for (const char* c = text; *c != '\0'; ++c) {
        // Afficher le caractère courant avec la police spécifiée (Helvetica, taille 18)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
}

void drawArc(float centerX, float centerY, float z, float radius, float startAngle, float endAngle, bool isWireframe) {
    int segments = 30;
    if (!isWireframe)
        glBegin(GL_TRIANGLE_FAN);
    else
        glBegin(GL_LINE_LOOP);
    glVertex2f(centerX, centerY);
    for (int i = startAngle; i <= endAngle; i++) {
        float theta = i * 3.14159265359f / 180.0f;// °->rad
        float x = centerX + radius * cos(theta);
        float y = centerY + radius * sin(theta);
        glVertex3f(x, y, z);
    }
    glEnd();
}

void drawRoundedRectangle(float centerX, float centerY, float z, float width, float height, float radius, bool isWireframe) {
    //  Dessinez le rectangle du corps
    float x = centerX - width / 2.0f;
    float y = centerY - height / 2.0f;
    if (!isWireframe)
        glBegin(GL_QUADS);
    else
        glBegin(GL_LINE_LOOP);
    glVertex3f(x + radius, y, z);
    glVertex3f(x + width - radius, y, z);
    glVertex3f(x + width - radius, y + height, z);
    glVertex3f(x + radius, y + height, z);
    glEnd();

    // Dessinez quatre coins arrondis (en utilisant des quarts de cercle)
    drawArc(x + radius, y + radius, z, radius, 180, 270, isWireframe);// En bas à gauche
    drawArc(x + width - radius, y + radius, z, radius, 270, 360, isWireframe);// en bas à droite
    drawArc(x + width - radius, y + height - radius, z, radius, 0, 90, isWireframe);// en haut à droite
    drawArc(x + radius, y + height - radius, z, radius, 90, 180, isWireframe);// en haut à gauche

    if (!isWireframe)
        glBegin(GL_QUADS);
    else
        glBegin(GL_LINE_LOOP);
    glVertex3f(x, y + radius, z);
    glVertex3f(x + radius, y + radius, z);
    glVertex3f(x + radius, y + height - radius, z);
    glVertex3f(x, y + height - radius, z);
    glEnd();

    if (!isWireframe)
        glBegin(GL_QUADS);
    else
        glBegin(GL_LINE_LOOP);
    glVertex3f(x + width - radius, y + radius, z);
    glVertex3f(x + width, y + radius, z);
    glVertex3f(x + width, y + height - radius, z);
    glVertex3f(x + width - radius, y + height - radius, z);
    glEnd();
}

void drawRoundedRectangle3D(float centerX, float centerY, float centerZ, float width, float height, float depth, float radius, bool isWireframe) {
    float zTop = centerZ + depth / 2.0f;
    float zBottom = centerZ - depth / 2.0f;
    drawRoundedRectangle(centerX, centerY, zTop, width, height, radius, isWireframe);
    drawRoundedRectangle(centerX, centerY, zBottom, width, height, radius, isWireframe);

    float xLeft = centerX - width / 2.0f;
    float xRight = centerX + width / 2.0f;
    float yBottom = centerY - height / 2.0f;
    float yTop = centerY + height / 2.0f;

    // left
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xLeft, yBottom + radius, zBottom);
    glVertex3f(xLeft, yBottom + radius, zTop);
    glVertex3f(xLeft, yTop - radius, zTop);
    glVertex3f(xLeft, yTop - radius, zBottom);
    glEnd();
    //right
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xRight, yBottom + radius, zBottom);
    glVertex3f(xRight, yBottom + radius, zTop);
    glVertex3f(xRight, yTop - radius, zTop);
    glVertex3f(xRight, yTop - radius, zBottom);
    glEnd();

    //top
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xLeft + radius, yTop, zBottom);
    glVertex3f(xLeft + radius, yTop, zTop);
    glVertex3f(xRight - radius, yTop, zTop);
    glVertex3f(xRight - radius, yTop, zBottom);
    glEnd();

    //bottom
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xLeft + radius, yBottom, zBottom);
    glVertex3f(xLeft + radius, yBottom, zTop);
    glVertex3f(xRight - radius, yBottom, zTop);
    glVertex3f(xRight - radius, yBottom, zBottom);
    glEnd();

    int segments = 30;
    float cornerOffsets[4][2] = {
        {xRight - radius, yTop - radius}, //right up
        {xLeft + radius, yTop - radius},     // left up
         {xLeft + radius, yBottom + radius} ,  // left down
        {xRight - radius, yBottom + radius} // right down

    };

    for (int i = 0; i < 4; ++i) {
        float cx = cornerOffsets[i][0];
        float cy = cornerOffsets[i][1];
        float startAngle = i * 90;
        float endAngle = startAngle + 90;

        for (int j = 0; j < segments; ++j) {
            float theta1 = startAngle + j * (90.0f / segments);
            float theta2 = startAngle + (j + 1) * (90.0f / segments);

            float x1 = cx + radius * cos(theta1 * 3.14159265359f / 180.0f);
            float y1 = cy + radius * sin(theta1 * 3.14159265359f / 180.0f);
            float x2 = cx + radius * cos(theta2 * 3.14159265359f / 180.0f);
            float y2 = cy + radius * sin(theta2 * 3.14159265359f / 180.0f);

            if (!isWireframe)
                glBegin(GL_QUADS);
            else
                glBegin(GL_LINE_LOOP);

            glVertex3f(x1, y1, zBottom);
            glVertex3f(x1, y1, zTop);
            glVertex3f(x2, y2, zTop);
            glVertex3f(x2, y2, zBottom);
            glEnd();
        }
    }
}

void setProjection() {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (isOrtho) {
        glOrtho(-20, 20, -15, 15, 1.0, 200.0);
    }
    else {
        gluPerspective(60.0, 4.0 / 3.0, 1.0, 200.0);
    }
    glMatrixMode(GL_MODELVIEW);
}
// Fonction pour dessiner une touche blanche
void drawWhiteKey(float x, bool isKeyPressed, bool isMousePressed, char label) {
    glPushMatrix();
    glTranslatef(x, 0.0f, 0.0f);

    // Step 1: Draw the texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, whiteKeyTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-whiteKeyWidth / 2.0f, -whiteKeyHeight / 2.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(whiteKeyWidth / 2.0f, -whiteKeyHeight / 2.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(whiteKeyWidth / 2.0f, whiteKeyHeight / 2.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-whiteKeyWidth / 2.0f, whiteKeyHeight / 2.0f, 0.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Step 2: Draw the illumination if the key is pressed
    if (isKeyPressed || isMousePressed) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 0.7f, 0.4f, 0.5f); // Semi-transparent illumination
        glBegin(GL_QUADS);
        glVertex3f(-whiteKeyWidth / 2.0f, -whiteKeyHeight / 2.0f, 0.01f); // Slightly above the texture
        glVertex3f(whiteKeyWidth / 2.0f, -whiteKeyHeight / 2.0f, 0.01f);
        glVertex3f(whiteKeyWidth / 2.0f, whiteKeyHeight / 2.0f, 0.01f);
        glVertex3f(-whiteKeyWidth / 2.0f, whiteKeyHeight / 2.0f, 0.01f);
        glEnd();

        glDisable(GL_BLEND);
    }

    // Step 3: Draw the label on top
    glDisable(GL_DEPTH_TEST); // Disable depth test to ensure text always appears on top
    drawText(&label, -whiteKeyWidth / 4.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f); // Adjust text position & color
    glEnable(GL_DEPTH_TEST);  // Re-enable depth test

    glPopMatrix();
}






// Fonction pour dessiner une touche noire
void drawBlackKey(float x, bool isKeyPressed, bool isMousePressed, char label) {
    glPushMatrix();
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 0.5f);

    // Step 1: Draw the texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, blackKeyTexture);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-blackKeyWidth / 2.0f, -blackKeyHeight / 2.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(blackKeyWidth / 2.0f, -blackKeyHeight / 2.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(blackKeyWidth / 2.0f, blackKeyHeight / 2.0f, 0.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-blackKeyWidth / 2.0f, blackKeyHeight / 2.0f, 0.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Step 2: Draw the illumination if the key is pressed
    if (isKeyPressed || isMousePressed) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 0.7f, 0.4f, 0.5f); // Semi-transparent illumination
        glBegin(GL_QUADS);
        glVertex3f(-blackKeyWidth / 2.0f, -blackKeyHeight / 2.0f, 0.01f); // Slightly above the texture
        glVertex3f(blackKeyWidth / 2.0f, -blackKeyHeight / 2.0f, 0.01f);
        glVertex3f(blackKeyWidth / 2.0f, blackKeyHeight / 2.0f, 0.01f);
        glVertex3f(-blackKeyWidth / 2.0f, blackKeyHeight / 2.0f, 0.01f);
        glEnd();

        glDisable(GL_BLEND);
    }

    // Step 3: Draw the label on top
    glDisable(GL_DEPTH_TEST); // Disable depth test to ensure text always appears on top
    drawText(&label, -blackKeyWidth / 4.0f, 0.0f, 0.02f, 1.0f, 1.0f, 1.0f); // Adjust text position & color
    glEnable(GL_DEPTH_TEST);  // Re-enable depth test

    glPopMatrix();
}





//void draw_1_octave() {
//    float whiteKeyX = -7.2f;
//    float blackKeyOffsets[] = { 0.5f, 1.5f, 3.5f, 4.5f, 5.5f }; 
//    float totalWhiteKeyWidthWithGap = whiteKeyWidth + gap; 
//
//    for (int i = 0; i < 7; ++i) {
//        drawWhiteKey(whiteKeyX + i *( whiteKeyWidth+gap), keyStatus[whiteKeys[i]],mouseStatus[whiteKeys[i]],whiteKeys[i]);
//        
//    }
//    for (int i = 0; i < 5; ++i) {
//        float blackKeyX = whiteKeyX + (blackKeyOffsets[i] * totalWhiteKeyWidthWithGap);
//        drawBlackKey(blackKeyX,  keyStatus[blackKeys[i]], mouseStatus[blackKeys[i]],blackKeys[i]);
//    }
//}
// Fonction pour dessiner un clavier complet (plusieurs octaves)
void drawPiano(float startX, int numOctaves) {
    float whiteKeyX = startX;  // Position de départ de la première octave
    float blackKeyOffsets[] = { 0.5f, 1.5f, 3.5f, 4.5f, 5.5f };
    float totalWhiteKeyWidthWithGap = whiteKeyWidth + gap;

    for (int octave = 0; octave < numOctaves; ++octave) {
        // Dessiner les touches blanches pour l'octave actuelle
        for (int i = 0; i < 7; ++i) {
            drawWhiteKey(whiteKeyX + i * totalWhiteKeyWidthWithGap, keyStatus[whiteKeys[i + octave * 7]],
                mouseStatus[whiteKeys[i + octave * 7]], whiteKeys[i + octave * 7]);
        }

        // Dessiner les touches noires pour l'octave actuelle
        for (int i = 0; i < 5; ++i) {
            float blackKeyX = whiteKeyX + (blackKeyOffsets[i] * totalWhiteKeyWidthWithGap);
            drawBlackKey(blackKeyX, keyStatus[blackKeys[i + octave * 5]], mouseStatus[blackKeys[i + octave * 5]], blackKeys[i + octave * 5]);
        }

        // Passer à la prochaine octave
        whiteKeyX += 7 * totalWhiteKeyWidthWithGap;
    }
}

void keyboardUp(unsigned char key, int x, int y) {
    key = toupper(key);
    keyStatus[key] = false;
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        GLdouble modelview[16];
        GLdouble projection[16];
        GLint viewport[4];
        GLdouble worldX, worldY, worldZ;

        glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
        glGetDoublev(GL_PROJECTION_MATRIX, projection);
        glGetIntegerv(GL_VIEWPORT, viewport);
        //viewport[3]  height of  window
        gluUnProject(x, viewport[3] - y, 0, modelview, projection, viewport, &worldX, &worldY, &worldZ);
        if (!isOrtho) {
            worldX *= 24;
            worldY = worldY * (24.8) + 24;
        }
        //cout << "Mouse: (" << x << ", " << y << "\n";
        //cout << "World Coordinates: (" << worldX << ", " << worldY << ", " << worldZ << ")\n";

        float whiteKeyX = -16.0f;
        float blackKeyOffsets[] = { 0.5f, 1.5f, 3.5f, 4.5f, 5.5f };
        float totalWhiteKeyWidthWithGap = whiteKeyWidth + gap;
        for (int octave = 0; octave < numOctaves; octave++) {
            for (int i = 0; i < 5; ++i) {
                float blackKeyX = whiteKeyX + blackKeyOffsets[i] * totalWhiteKeyWidthWithGap - blackKeyWidth / 2.0f;
                float blackKeyYmax = whiteKeyHeight / 2.0f;
                float blackKeyYmin = whiteKeyHeight / 2.0f - blackKeyHeight;
                if (worldX >= blackKeyX && worldX <= blackKeyX + blackKeyWidth &&
                    worldY >= blackKeyYmin && worldY <= blackKeyYmax) {
                    char key = blackKeys[i + octave * 5];
                    if (mouseStatus[key] == false && soundEffects[key]) {
                        Mix_PlayChannel(-1, soundEffects[key], 0);
                    }
                    mouseStatus[key] = true;
                    if (isDrop) {
                        for (auto& rect : rectangles) {
                            if (rect.associatedKey == key && rect.isActive) {
                                rect.isActive = false;
                                break;
                            }
                        }
                    }
                    glutPostRedisplay();
                    return;
                }
            }
            for (int i = 0; i < 7; ++i) {
                float keyStartX = whiteKeyX + i * (whiteKeyWidth + gap) - whiteKeyWidth / 2.0f;
                float keyEndX = keyStartX + whiteKeyWidth;
                float keyStartY = -whiteKeyHeight / 2.0f;
                float keyEndY = whiteKeyHeight / 2.0f;
                if (worldX >= keyStartX && worldX <= keyEndX &&
                    worldY >= keyStartY && worldY <= keyEndY) {
                    char key = whiteKeys[i + octave * 7];
                    if (mouseStatus[key] == false && soundEffects[key]) {
                        Mix_PlayChannel(-1, soundEffects[key], 0);
                    }
                    mouseStatus[key] = true;
                    if (isDrop) {
                        for (auto& rect : rectangles) {
                            if (rect.associatedKey == key && rect.isActive) {
                                rect.isActive = false;
                                break;
                            }
                        }
                    }
                    glutPostRedisplay();
                    return;
                }
            }
            whiteKeyX += 7 * totalWhiteKeyWidthWithGap;
        }
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        for (auto& status : mouseStatus) {
            status.second = false;
        }
        glutPostRedisplay();
    }
}

void spawnRectangle(bool isWhiteKey, char associatedKey) {
    FallingRectangle rect;
    if (isWhiteKey) {
        // Find the corresponding position according to the key
        auto it = find_if(whiteKeys_position.begin(), whiteKeys_position.end(),
            [associatedKey](const KeyMapping& km) { return km.associatedKey == associatedKey; });
        if (it != whiteKeys_position.end()) {
            rect.x = it->xPosition;
            rect.associatedKey = associatedKey;
        }
    }
    else {
        auto it = find_if(blackKeys_position.begin(), blackKeys_position.end(),
            [associatedKey](const KeyMapping& km) { return km.associatedKey == associatedKey; });
        if (it != blackKeys_position.end()) {
            rect.x = it->xPosition;
            rect.associatedKey = associatedKey;
        }
    }
    rect.y = 20.0f;
    rect.z = 2.0f;
    rect.width = isWhiteKey ? 2.4f : 1.4f;
    rect.height = 1.5f;
    rect.isActive = true;

    rectangles.push_back(rect);
}

//void timerFunc(int value) {
//    if (rand() % 10 < 7) { 
//        spawnRectangle(true);
//    }
//    else { 
//        spawnRectangle(false);
//    }
//    random_device rd; 
//    mt19937 gen(rd()); 
//    uniform_int_distribution<int> dis(500, 1500); 
//    int nextInterval = dis(gen);
//    glutTimerFunc(nextInterval, timerFunc, 0);
//}

void checkRectangleHit(char key) {
    for (auto& rect : rectangles) {
        if (rect.associatedKey == key && rect.isActive) {
            rect.isActive = false;
            break;
        }
    }
}

vector<pair<char, float>> sheetMusic;
void keyboardDown(unsigned char key, int x, int y) {
    switch (key) {
    case '1':
        sheetMusic = loadSheetMusicFromFile("Notes/ode_to_joy.txt");
        isDrop = !isDrop;
        startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        rectangles.clear();
        cout << "Current track:《Ode to Joy》 isDrop:" << isDrop << endl;
        break;

    case '2':
        sheetMusic = loadSheetMusicFromFile("Notes/twinkle_twinkle.txt");
        isDrop = !isDrop;
        startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        rectangles.clear();
        cout << "Current track:《Twinkle twinkle》 isDrop:" << isDrop << endl;
        break;

    case '3':
        sheetMusic = loadSheetMusicFromFile("Notes/happy_birthday.txt");
        isDrop = !isDrop;
        startTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
        rectangles.clear();
        cout << "Current track:《Happy birthday》 isDrop:" << isDrop << endl;
        break;
    case' ':
        isOrtho = !isOrtho;
        setProjection();
        glutPostRedisplay();
        break;
    default:
        key = toupper(key);
        if (keyStatus[key] == false && soundEffects[key]) {
            Mix_PlayChannel(-1, soundEffects[key], 0);// -1 signifie que le premier canal libre est sélectionné pour la lecture
        }
        keyStatus[key] = true;
        glutPostRedisplay();
        break;
    }
    checkRectangleHit(key);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    if (isOrtho) {
        gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    }
    else {
        gluLookAt(0.0, -1.0, 25.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    }

    drawPiano(-16.0f, numOctaves);
    if (isDrop) {
        for (auto& rect : rectangles) {
            if (rect.isActive) {
                glColor3f(0.6f, 0.8f, 0.9f);
                glBegin(GL_QUADS);
                glVertex3f(rect.x - rect.width / 2, rect.y, rect.z);
                glVertex3f(rect.x + rect.width / 2, rect.y, rect.z);
                glVertex3f(rect.x + rect.width / 2, rect.y + rect.height, rect.z);
                glVertex3f(rect.x - rect.width / 2, rect.y + rect.height, rect.z);
                glEnd();
            }
        }
    }
    glutSwapBuffers();
}

void update(int value) {
    if (isDrop) {
        float currentTime = (glutGet(GLUT_ELAPSED_TIME) / 1000.0f) - startTime;

        // Iterate over the score to generate rectangles
        while (!sheetMusic.empty() && sheetMusic.front().second <= currentTime) {
            char key = sheetMusic.front().first;
            bool isWhiteKey = find(begin(whiteKeys), end(whiteKeys), key) != end(whiteKeys);
            spawnRectangle(isWhiteKey, key);
            sheetMusic.erase(sheetMusic.begin());
        }

        for (auto& rect : rectangles) {
            if (rect.isActive) {
                rect.y -= 0.05f; // Rectangular descent speed
                if (rect.y < -15.0f) {
                    rect.isActive = false; // Rectangle disappears when out of range
                }
            }
        }

        glutPostRedisplay();
    }

    glutTimerFunc(16, update, 0); // Updated every 16ms
}
// Fonction d'initialisation
void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    setProjection();

    // Load textures
    whiteKeyTexture = loadTexture("white.png");
    if (whiteKeyTexture == 0) {
        std::cerr << "Failed to load white.jpg texture!" << std::endl;
    }

    blackKeyTexture = loadTexture("black.jpg");
    if (blackKeyTexture == 0) {
        std::cerr << "Failed to load black.jpg texture!" << std::endl;
    }
}

int main(int argc, char** argv) {
    for (char key : { 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'C', 'V', 'B', 'N', 'M', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P'}) {
        keyStatus[key] = false;
        mouseStatus[key] = false;
    }

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Piano simulation");
    glewInit();
    init();
    initializeAudio();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);
    glutTimerFunc(16, update, 0);
    //glutTimerFunc(1000, timerFunc, 0); 

    glutMainLoop();
    cleanupAudio();
    return 0;
}
