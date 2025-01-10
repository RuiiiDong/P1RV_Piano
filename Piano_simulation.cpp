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
GLuint texture;

int currentSkin = 0;
map<char, bool> keyStatus;
map<char, bool>mouseStatus;
map<char, Mix_Chunk*> soundEffects;
//variables liés au recording
std::vector<Uint8> audioBuffer; // Buffer to store audio data
bool isRecording = false;      // Recording state


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
map<char, tuple<float, float, float>> keyColors = {
    {'A', {0.96f, 0.26f, 0.21f}},
    {'S', {0.30f, 0.69f, 0.31f}},
    {'D', {0.13f, 0.59f, 0.95f}},
    {'F', {1.00f, 0.92f, 0.23f}},
    {'G', {0.61f, 0.15f, 0.69f}},
    {'H', {0.00f, 0.73f, 0.83f}},
    {'J', {0.85f, 0.33f, 0.10f}},
    {'K', {0.40f, 0.76f, 0.64f}},
    {'L', {0.25f, 0.32f, 0.71f}},
    {'C', {1.00f, 0.55f, 0.00f}},
    {'V', {0.64f, 0.16f, 0.16f}},
    {'B', {0.12f, 0.73f, 0.56f}},
    {'N', {0.46f, 0.53f, 0.60f}},
    {'M', {0.91f, 0.76f, 0.65f}},
    {'Q', {0.89f, 0.22f, 0.21f}},
    {'W', {0.80f, 0.60f, 0.20f}},
    {'E', {0.18f, 0.80f, 0.44f}},
    {'R', {0.25f, 0.88f, 0.82f}},
    {'T', {0.40f, 0.22f, 0.71f}},
    {'Y', {0.71f, 0.53f, 0.35f}},
    {'U', {0.56f, 0.64f, 0.85f}},
    {'I', {0.95f, 0.77f, 0.06f}},
    {'O', {0.74f, 0.20f, 0.64f}},
    {'P', {0.99f, 0.68f, 0.13f}}
};



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
//fonctions liée au son/recording
void audioRecordingCallback(void* udata, Uint8* stream, int len) {
    if (isRecording) {
        audioBuffer.insert(audioBuffer.end(), stream, stream + len);
    }
}



void initializeAudio() {
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        cerr << "SDL Initialization Failed: " << SDL_GetError() << endl;
        exit(1);
    }
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        cerr << "Mix_OpenAudio Failed: " << Mix_GetError() << endl;
        exit(1);
    }
    Mix_AllocateChannels(100);
    Mix_SetPostMix(audioRecordingCallback, nullptr);
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
void startRecording() {
    if (isRecording) return;
    audioBuffer.clear();
    isRecording = true;
    cout << "Recording started!" << endl;
}

void stopRecording(const std::string& filename) {
    if (!isRecording) return;

    isRecording = false;
    cout << "Recording stopped! Saving to " << filename << endl;

    std::ofstream wavFile(filename, std::ios::binary);
    if (!wavFile.is_open()) {
        cerr << "Error opening file for recording!" << endl;
        return;
    }

    // Write WAV header
    Uint32 dataSize = audioBuffer.size();
    wavFile.write("RIFF", 4);
    Uint32 fileSize = 36 + dataSize;
    wavFile.write(reinterpret_cast<const char*>(&fileSize), 4);
    wavFile.write("WAVE", 4);
    wavFile.write("fmt ", 4);
    Uint32 subChunk1Size = 16;
    wavFile.write(reinterpret_cast<const char*>(&subChunk1Size), 4);
    Uint16 audioFormat = 1;
    wavFile.write(reinterpret_cast<const char*>(&audioFormat), 2);
    Uint16 numChannels = 2; // Stereo
    wavFile.write(reinterpret_cast<const char*>(&numChannels), 2);
    Uint32 sampleRate = 44100;
    wavFile.write(reinterpret_cast<const char*>(&sampleRate), 4);
    Uint32 byteRate = sampleRate * numChannels * 2;
    wavFile.write(reinterpret_cast<const char*>(&byteRate), 4);
    Uint16 blockAlign = numChannels * 2;
    wavFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
    Uint16 bitsPerSample = 16;
    wavFile.write(reinterpret_cast<const char*>(&bitsPerSample), 2);
    wavFile.write("data", 4);
    wavFile.write(reinterpret_cast<const char*>(&dataSize), 4);

    // Write audio data
    wavFile.write(reinterpret_cast<const char*>(audioBuffer.data()), dataSize);
    wavFile.close();
    cout << "Recording saved!" << endl;
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
// Fonction pour dessiner une touche blanche avec le skin 1
void drawWhiteKey1(float x, bool iskeyPressed, bool ismousepressed, char label) {
    // Charger la texture de la touche blanche (seulement une fois)

    // Définir les coordonnées de texture (tout l'image est utilisée ici)
    float texCoordWidth = 1.0f;
    float texCoordHeight = 1.0f;

    glPushMatrix();
    glTranslatef(x, 0.0f, 0.0f);

    /*
    // Activer et lier la texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Dessiner la forme de la touche avec la texture
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-whiteKeyWidth / 2.0f - 0.5f, -whiteKeyHeight / 2.0f - 0.5f, -0.8f);
    glTexCoord2f(texCoordWidth, 0.0f);
    glVertex3f(whiteKeyWidth / 2.0f + 0.5f, -whiteKeyHeight / 2.0f - 0.5f, -0.8f);
    glTexCoord2f(texCoordWidth, texCoordHeight);
    glVertex3f(whiteKeyWidth / 2.0f + 0.5f, whiteKeyHeight / 2.0f + 2.5f, -0.8f);
    glTexCoord2f(0.0f, texCoordHeight);
    glVertex3f(-whiteKeyWidth / 2.0f - 0.5f, whiteKeyHeight / 2.0f + 2.5f, -0.8f);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    */

    // Dessiner une ombre sous la touche avec un léger décalage
    glColor4f(0.05f, 0.05f, 0.05f, 0.4f);  // Couleur gris pour l'ombre
    glPushMatrix();
    glTranslatef(0.13f, -0.1f, -0.13f); // Décalage de l'ombre
    drawRoundedRectangle(0.0, 0.0, -0.5f, whiteKeyWidth, whiteKeyHeight, 0.2f, 0);
    glPopMatrix();

    // Changer la couleur de la touche en fonction de son état (appuyée ou non)
    if (iskeyPressed || ismousepressed) {
        glColor3f(1.0f, 0.7f, 0.4f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    drawRoundedRectangle3D(0.0, 0.0, 0.0f, whiteKeyWidth, whiteKeyHeight, whiteKeyDepth, 0.2f, 0);
    glPopMatrix();

    // Dessiner le contour de la touche avec une couleur grise et une épaisseur de ligne plus grande
    glPushMatrix();
    glTranslatef(x, 0.0f, 0.0f);
    glColor3f(0.5f, 0.5f, 0.5f);
    glLineWidth(3.0f);  // Définir l'épaisseur des lignes pour le bord
    if (isOrtho) {
        drawRoundedRectangle(0.0, 0.0, 0.0f, whiteKeyWidth, whiteKeyHeight, 0.2f, 1);
    }
    else {
        drawRoundedRectangle(0.0, 0.0, 0.2f, whiteKeyWidth, whiteKeyHeight, 0.2f, 1);
    }

    glPopMatrix();

    // Dessiner le texte sur la touche
    drawText(string(1, label).c_str(), x - 0.2f, -5.0f, 1.5f, 0.0f, 0.0f, 0.0f);
}

// Fonction pour dessiner une touche blanche avec le skin 2
void drawWhiteKey2(float x, bool isKeyPressed, bool isMousePressed, char label) {
    glPushMatrix(); // Sauvegarder la matrice actuelle
    glTranslatef(x, 0.0f, 0.0f); // Positionner la touche

    // Étape 1 : Dessiner la texture
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

    // Étape 2 : Dessiner l'illumination si la touche est pressée
    if (isKeyPressed || isMousePressed) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glColor4f(1.0f, 0.7f, 0.4f, 0.5f); // Couleur semi-transparente
        glBegin(GL_QUADS);
        glVertex3f(-whiteKeyWidth / 2.0f, -whiteKeyHeight / 2.0f, 0.01f); // Légèrement au-dessus de la texture
        glVertex3f(whiteKeyWidth / 2.0f, -whiteKeyHeight / 2.0f, 0.01f);
        glVertex3f(whiteKeyWidth / 2.0f, whiteKeyHeight / 2.0f, 0.01f);
        glVertex3f(-whiteKeyWidth / 2.0f, whiteKeyHeight / 2.0f, 0.01f);
        glEnd();

        glDisable(GL_BLEND);
    }

    // Étape 3 : Dessiner le texte sur le bouton (en bas, centré)
    glDisable(GL_DEPTH_TEST); // Désactiver le test de profondeur pour que le texte soit toujours visible
    drawText(&label, -whiteKeyWidth / 4.0f, -whiteKeyHeight / 2.0f + 0.5f, 0.02f, 0.0f, 0.0f, 0.0f); // Position et couleur
    glEnable(GL_DEPTH_TEST); // Réactiver le test de profondeur

    glPopMatrix(); // Restaurer la matrice actuelle
}


// Fonction pour dessiner une touche noire avec le skin 1
void drawBlackKey1(float x, bool iskeyPressed, bool ismousepressed, char label) {
    glPushMatrix();
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 0.0f);
    // Dessiner l'ombre de la touche noire
    glColor4f(0.1f, 0.1f, 0.1f, 0.4f);
    glTranslatef(0.05f, -0.1f, -0.05f); //Décalage de l'ombre
    drawRoundedRectangle(0.0, 0.0, 0.5f, blackKeyWidth, blackKeyHeight, 0.2f, 0);
    glPopMatrix();

    // Définir la couleur de la touche noire en fonction de son état (appuyée ou non)
    glPushMatrix();
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 0.5f);

    if (iskeyPressed || ismousepressed) {
        glColor3f(1.0f, 0.7f, 0.4f);
    }
    else {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    }
    drawRoundedRectangle3D(0.0, 0.0, 0.4f, blackKeyWidth, blackKeyHeight, blackKeyDepth, 0.15f, 0);
    glPopMatrix();

    // Dessiner le contour de la touche noire pour créer un effet visuel
    glPushMatrix();
    // Déplacer légèrement pour que le contour soit devant la touche
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 0.5f);
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(2.0f);  // Définir l'épaisseur des lignes pour le contour
    float scale = 1.005f;
    glScalef(scale, scale, scale);
    drawRoundedRectangle(0.0, 0.0, 0.2f, blackKeyWidth, blackKeyHeight, 0.2f, 1);

    glPopMatrix();
    drawText(string(1, label).c_str(), x - 0.15f, -0.8f, 2.0f, 1.0f, 1.0f, 1.0f);
}
// Fonction pour dessiner une touche noire avec le skin 2
void drawBlackKey2(float x, bool isKeyPressed, bool isMousePressed, char label) {
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
    float whiteKeyX = startX;
    float blackKeyOffsets[] = { 0.5f, 1.5f, 3.5f, 4.5f, 5.5f };
    float totalWhiteKeyWidthWithGap = whiteKeyWidth + gap;

    for (int octave = 0; octave < numOctaves; ++octave) {
        // Dessiner les touches blanches pour l'octave actuelle
        for (int i = 0; i < 7; ++i) {
            if (currentSkin == 0) {
                drawWhiteKey1(whiteKeyX + i * totalWhiteKeyWidthWithGap, keyStatus[whiteKeys[i + octave * 7]],
                    mouseStatus[whiteKeys[i + octave * 7]], whiteKeys[i + octave * 7]);
            }
            else {
                drawWhiteKey2(whiteKeyX + i * totalWhiteKeyWidthWithGap, keyStatus[whiteKeys[i + octave * 7]],
                    mouseStatus[whiteKeys[i + octave * 7]], whiteKeys[i + octave * 7]);
            }
        }

        // Dessiner les touches noires pour l'octave actuelle
        for (int i = 0; i < 5; ++i) {
            float blackKeyX = whiteKeyX + (blackKeyOffsets[i] * totalWhiteKeyWidthWithGap);
            if (currentSkin == 0) {
                drawBlackKey1(blackKeyX, keyStatus[blackKeys[i + octave * 5]],
                    mouseStatus[blackKeys[i + octave * 5]], blackKeys[i + octave * 5]);
            }
            else {
                drawBlackKey2(blackKeyX, keyStatus[blackKeys[i + octave * 5]],
                    mouseStatus[blackKeys[i + octave * 5]], blackKeys[i + octave * 5]);
            }
        }

        // Passer à la prochaine octave
        whiteKeyX += 7 * totalWhiteKeyWidthWithGap;
    }
}

void drawGrid(const vector<KeyMapping>& whiteKeysPosition, float startY, float endY) {
    glColor4f(0.0f, 0.0f, 0.0f, 0.3f); // Semi-transparent black

    // Draw vertical lines between white keys (merge successive lines)
    for (size_t i = 0; i < whiteKeysPosition.size() - 1; ++i) {
        // Calculate the middle point between two adjacent keys
        float middleX = (whiteKeysPosition[i].xPosition + whiteKeysPosition[i + 1].xPosition) / 2.0f;

        // Draw a single line at the middle point
        glBegin(GL_LINES);
        glVertex3f(middleX, startY, 0.0f);
        glVertex3f(middleX, endY, 0.0f);
        glEnd();
    }

    // Draw the first and last vertical lines to frame the grid
    float firstX = whiteKeysPosition.front().xPosition - whiteKeyWidth / 2.0f;
    float lastX = whiteKeysPosition.back().xPosition + whiteKeyWidth / 2.0f;

    glBegin(GL_LINES);
    glVertex3f(firstX, startY, 0.0f);
    glVertex3f(firstX, endY, 0.0f);
    glEnd();

    glBegin(GL_LINES);
    glVertex3f(lastX, startY, 0.0f);
    glVertex3f(lastX, endY, 0.0f);
    glEnd();
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
void drawFallingRectangle(const FallingRectangle& rect) {
    glPushMatrix();
    glTranslatef(rect.x, rect.y, rect.z);

    // Définir une nouvelle couleur harmonieuse en fonction de la touche
    auto color = keyColors[rect.associatedKey];
    float r = get<0>(color);
    float g = get<1>(color);
    float b = get<2>(color);

    // Appliquer une légère variation pour les rendre plus esthétiques
    glColor3f(r * 0.9f + 0.1f, g * 0.9f + 0.1f, b * 0.9f + 0.1f);

    // Dessiner le rectangle arrondi
    drawRoundedRectangle3D(
        0.0f,
        0.0f,
        0.0f,
        rect.width,
        rect.height,
        0.2f,
        0.2f,
        false
    );

    // Dessiner l'étiquette de la touche sur le rectangle
    glColor3f(0.0f, 0.0f, 0.0f); // Texte noir
    drawText(string(1, rect.associatedKey).c_str(), 0.0f, -rect.height / 4.0f, rect.z + 0.2f, 0.0f, 0.0f, 0.0f);

    glPopMatrix();
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
void resetKeyStates() {
    for (auto& status : keyStatus) {
        status.second = false;
    }
    for (auto& status : mouseStatus) {
        status.second = false;
    }
}

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
    case '7':

        if (currentSkin == 0) { currentSkin = 1; }
        else { currentSkin = 0; }
        glutPostRedisplay();

        break;
    case '8': // Start recording
        startRecording();
        break;
    case '9': // Stop recording
        stopRecording("output.wav");
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
//guide pour utilisateur
void drawUserGuide() {
    // Define colors for ON/OFF states
    std::string recordingStatus = isRecording ?
        "On (Press '9' to stop)" :
        "Off (Press '8' to start)";

    // Define the tutorial name
    std::string tutorialStatus = isDrop ?
        "Tutorial: ACTIVE (Press '1', '2', or '3' to change)" :
        "Tutorial: NONE (Press '1', '2', or '3' to activate)";

    // Define skin type
    std::string skinStatus = currentSkin == 0 ?
        "Skin: CLASSIC (Press '7' to toggle)" :
        "Skin: MODERN (Press '7' to toggle)";

    // Define projection type
    std::string projectionStatus = isOrtho ?
        "Projection: ORTHOGRAPHIC (Press SPACE to toggle)" :
        "Projection: PERSPECTIVE (Press SPACE to toggle)";

    // Combine all guide messages
    std::string guideText =
        "Recording: " + recordingStatus + " / " +
        tutorialStatus + " / " +
        skinStatus + " / " +
        projectionStatus;

    // Adjust the starting position to push it left
    float startX = -19.0f; // Shifted further to the left
    float startY = -14.0f;

    // Draw the user guide text
    drawText(guideText.c_str(), startX, startY, 0.0f, 1.0f, 1.0f, 1.0f);
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
    //draw Piano
    drawPiano(-16.0f, numOctaves);
    //guide utilisateur
    drawUserGuide();

    if (isDrop) { drawGrid(whiteKeys_position, 15.0f, -15.0f); }
    if (isDrop) {
        for (auto& rect : rectangles) {
            if (rect.isActive) {
                drawFallingRectangle(rect);
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
    texture = loadTexture("2.png");
    if (texture == 0) {
        std::cerr << "Failed to load 2.png texture!" << std::endl;
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
