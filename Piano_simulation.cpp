#include <Windows.h>
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include <GL/gl.h>
#include <GLUT.h>
#include<map>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include<SDL.h>
#include <SDL_mixer.h>

using namespace std;
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
char whiteKeys[] = { 'A', 'S', 'D', 'F', 'G', 'H', 'J' };
char blackKeys[] = { 'W', 'E', 'T', 'Y', 'U' };
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
        cerr<< "Échec d'ouverture de Mix_OpenAudio : " << Mix_GetError() <<endl;
        exit(1);
    }
    Mix_AllocateChannels(100);
    // Charger les fichiers audio pour chaque touche
    soundEffects['A'] = Mix_LoadWAV("sounds/A.wav");
    soundEffects['S'] = Mix_LoadWAV("sounds/S.wav");
    soundEffects['D'] = Mix_LoadWAV("sounds/D.wav");
    soundEffects['F'] = Mix_LoadWAV("sounds/F.wav");
    soundEffects['G'] = Mix_LoadWAV("sounds/G.wav");
    soundEffects['H'] = Mix_LoadWAV("sounds/H.wav");
    soundEffects['J'] = Mix_LoadWAV("sounds/J.wav");

    soundEffects['W'] = Mix_LoadWAV("sounds/W.wav");
    soundEffects['E'] = Mix_LoadWAV("sounds/E.wav");
    soundEffects['T'] = Mix_LoadWAV("sounds/T.wav");
    soundEffects['Y'] = Mix_LoadWAV("sounds/Y.wav");
    soundEffects['U'] = Mix_LoadWAV("sounds/U.wav");

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
    glVertex2f(centerX,centerY);
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

    float xLeft = centerX - width / 2.0f ;
    float xRight = centerX + width / 2.0f ;
    float yBottom = centerY - height / 2.0f ;
    float yTop = centerY + height / 2.0f;


    // left
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xLeft, yBottom+radius, zBottom);
    glVertex3f(xLeft, yBottom+radius, zTop);
    glVertex3f(xLeft, yTop-radius, zTop);
    glVertex3f(xLeft, yTop-radius, zBottom);
    glEnd();
//right
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xRight, yBottom + radius, zBottom);
    glVertex3f(xRight, yBottom + radius, zTop);
    glVertex3f(xRight, yTop-radius, zTop);
    glVertex3f(xRight, yTop-radius, zBottom);
    glEnd();

    //top
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xLeft +radius, yTop, zBottom);
    glVertex3f(xLeft + radius, yTop, zTop);
    glVertex3f(xRight - radius, yTop, zTop);
    glVertex3f(xRight - radius, yTop, zBottom);
    glEnd();
     

     //bottom
    glBegin(isWireframe ? GL_LINE_LOOP : GL_QUADS);
    glVertex3f(xLeft+radius, yBottom, zBottom);
    glVertex3f(xLeft+radius, yBottom, zTop);
    glVertex3f(xRight-radius, yBottom, zTop);
    glVertex3f(xRight-radius, yBottom, zBottom);
    glEnd();

    int segments = 30; 
    float cornerOffsets[4][2] = {
        {xRight-radius, yTop-radius}, //right up
        {xLeft+radius, yTop-radius},     // left up
         {xLeft+radius, yBottom+radius} ,  // left down
        {xRight-radius, yBottom+radius} // right down
       
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

// Fonction pour dessiner une touche blanche
void drawWhiteKey(float x,bool iskeyPressed, bool ismousepressed,char label) {
    // Charger la texture de la touche blanche (seulement une fois)
    static GLuint texture = loadTexture("2.png");
    // Définir les coordonnées de texture (tout l'image est utilisée ici)
    float texCoordWidth = 1.0f;  
    float texCoordHeight =1.0f;  

    glPushMatrix();
    glTranslatef(x, 0.0f, 0.0f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glColor3f(1.0f, 1.0f, 1.0f); 

    // Dessiner la forme de la touche avec la texture
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); 
    glVertex3f(-whiteKeyWidth / 2.0f-0.5f, -whiteKeyHeight / 2.0f-0.5f, -0.8f); 
    glTexCoord2f(texCoordWidth, 0.0f); 
    glVertex3f(whiteKeyWidth / 2.0f+0.5f, -whiteKeyHeight / 2.0f-0.5f, -0.8f); 
    glTexCoord2f(texCoordWidth, texCoordHeight); 
    glVertex3f(whiteKeyWidth / 2.0f+0.5f, whiteKeyHeight / 2.0f+2.5f, -0.8f); 
    glTexCoord2f(0.0f, texCoordHeight); 
    glVertex3f(-whiteKeyWidth / 2.0f-0.5f, whiteKeyHeight / 2.0f+2.5f, -0.8f); 
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Dessiner une ombre sous la touche avec un léger décalage
    glColor4f(0.05f, 0.05f, 0.05f, 0.4f);  // Couleur gris  pour l'ombre
    glPushMatrix();
    glTranslatef(0.13f, -0.1f, -0.13f); // Décalage de l'ombre
    drawRoundedRectangle(0.0, 0.0, -0.5f, whiteKeyWidth, whiteKeyHeight, 0.2f, 0);
    glPopMatrix();

    // Changer la couleur de la touche en fonction de son état (appuyée ou non)
    if (iskeyPressed||ismousepressed) {
        glColor3f(1.0f, 0.7f, 0.4f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    drawRoundedRectangle3D(0.0, 0.0, 0.0f, whiteKeyWidth, whiteKeyHeight,whiteKeyDepth, 0.2f, 0);
    glPopMatrix();

    // Dessiner le contour de la touche avec une couleur grise et une épaisseur de ligne plus grande
    glPushMatrix();
    glTranslatef(x, 0.0f, 0.0f);
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(3.0f);  // Définir l'épaisseur des lignes pour le bord
    drawRoundedRectangle(0.0, 0.0, 0.0f, whiteKeyWidth, whiteKeyHeight, 0.2f, 1);

    glPopMatrix();

    drawText(string(1, label).c_str(), x - 0.2f, -5.0f, 1.5f, 0.0f, 0.0f, 0.0f);
}

// Fonction pour dessiner une touche noire
void drawBlackKey(float x, bool iskeyPressed, bool ismousepressed,char label) {
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

    if (iskeyPressed||ismousepressed) {
        glColor3f(1.0f, 0.7f, 0.4f);
    }
    else {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    }
    drawRoundedRectangle3D(0.0, 0.0,0.8f, blackKeyWidth, blackKeyHeight,blackKeyDepth, 0.15f, 0);
    glPopMatrix();

    // Dessiner le contour de la touche noire pour créer un effet visuel
    glPushMatrix();
    // Déplacer légèrement pour que le contour soit devant la touche
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 0.5f);
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(2.0f);  // Définir l'épaisseur des lignes pour le contour
    float scale = 1.005f;
    glScalef(scale, scale, scale);
    drawRoundedRectangle(0.0, 0.0, 0.5f, blackKeyWidth, blackKeyHeight, 0.2f, 1);

    glPopMatrix();
    drawText(string(1, label).c_str(), x - 0.15f, -0.8f, 2.0f, 1.0f, 1.0f, 1.0f);
}

// Fonction pour dessiner l'ensemble du piano
void drawPiano() {
    float whiteKeyX = -7.2f;
    float blackKeyOffsets[] = { 0.5f, 1.5f, 3.5f, 4.5f, 5.5f }; 
    float totalWhiteKeyWidthWithGap = whiteKeyWidth + gap; 

    for (int i = 0; i < 7; ++i) {
        drawWhiteKey(whiteKeyX + i *( whiteKeyWidth+gap), keyStatus[whiteKeys[i]],mouseStatus[whiteKeys[i]],whiteKeys[i]);
        
    }
    for (int i = 0; i < 5; ++i) {
        float blackKeyX = whiteKeyX + (blackKeyOffsets[i] * totalWhiteKeyWidthWithGap);
        drawBlackKey(blackKeyX,  keyStatus[blackKeys[i]], mouseStatus[blackKeys[i]],blackKeys[i]);
    }
}

// Fonction pour afficher la scène dans la fenêtre
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0,0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    drawPiano();
   //drawRoundedRectangle3D(0, 0, 0, blackKeyWidth, blackKeyHeight, 2.0, 0.2,0);
    //drawRoundedRectangle(0, 0, 0, blackKeyWidth, blackKeyHeight, 0.2, 0);
    glutSwapBuffers();
}

void keyboardDown(unsigned char key, int x, int y) {
    key = toupper(key);
    if (keyStatus[key]==false&&soundEffects[key]) {
        Mix_PlayChannel(-1, soundEffects[key], 0);   // -1 signifie que le premier canal libre est sélectionné pour la lecture
    } 
    keyStatus[key] = true;
    glutPostRedisplay();
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

        float whiteKeyX = -7.2f;
        float blackKeyOffsets[] = { 0.5f, 1.5f, 3.5f, 4.5f, 5.5f };
        float totalWhiteKeyWidthWithGap = whiteKeyWidth + gap;
        for (int i = 0; i < 5; ++i) {
            float blackKeyX = whiteKeyX + blackKeyOffsets[i] * totalWhiteKeyWidthWithGap - blackKeyWidth / 2.0f;
            float blackKeyYmax = whiteKeyHeight / 2.0f;
            float blackKeyYmin = whiteKeyHeight / 2.0f - blackKeyHeight;
            if (worldX >= blackKeyX && worldX <= blackKeyX + blackKeyWidth &&
                worldY >= blackKeyYmin && worldY <= blackKeyYmax) {
                char key = blackKeys[i];
                if (mouseStatus[key] ==false&& soundEffects[key]) {
                    Mix_PlayChannel(-1, soundEffects[key], 0);
                }
                mouseStatus[key] = true;
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
                char key = whiteKeys[i];
                if (mouseStatus[key] ==false && soundEffects[key]) {
                    Mix_PlayChannel(-1, soundEffects[key], 0);
                }
                mouseStatus[key] = true;
                glutPostRedisplay();
                return;
            }
        }
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        for (auto& status : mouseStatus) {
            status.second = false;
        }
        glutPostRedisplay();
    }
}

// Fonction d'initialisation
void init() {
    glEnable(GL_DEPTH_TEST);  
    glEnable(GL_BLEND); 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
    glMatrixMode(GL_PROJECTION);
   
    // Définir les limites du système de coordonnées en projection orthographique
    glOrtho(-20, 20, -15, 15, 1.0, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    for (char key : {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'W', 'E', 'T', 'Y', 'U'}) {
        keyStatus[key] = false;
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

    glutMainLoop();
    cleanupAudio();
    return 0;
}

