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

using namespace std;
map<char, bool> keyStatus;

// Définition des dimensions des touches du piano
float whiteKeyWidth = 2.4f; // Largeur des touches blanches : 2.4 cm
float whiteKeyHeight = 15.0f;  // Longueur des touches blanches : 15 cm
float blackKeyWidth = 1.4f;    // Largeur des touches noires : 1.4 cm
float blackKeyHeight = 10.0f; // Longueur des touches noires :10 cm
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

void drawText(const char* text, float x, float y, float z, float r, float g, float b) {
    glColor3f(r, g, b);  
    glRasterPos3f(x, y, z);  
    for (const char* c = text; *c != '\0'; ++c) {
        // Afficher le caractère courant avec la police spécifiée (Helvetica, taille 18)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);  
    }
}

void drawArc(float centerX, float centerY, float z, float radius, float startAngle, float endAngle, bool iscontour) {
    int segments = 30; 
    if (!iscontour)
        glBegin(GL_TRIANGLE_FAN);
    else
        glBegin(GL_LINE_LOOP);

    glVertex2f(centerX, centerY); 
    for (int i = startAngle; i <= endAngle; i++) {
        float theta = i * 3.14159265359f / 180.0f;
        float x = centerX + radius * cos(theta);
        float y = centerY + radius * sin(theta);
        glVertex3f(x, y, z);
    }
    glEnd();
}

void drawRoundedRectangle(float centerX, float centerY, float z, float width, float height, float radius, bool iscontour) {
    //  Dessinez le rectangle du corps
    float x = centerX - width / 2.0f;
    float y = centerY - height / 2.0f;
    if (!iscontour)
        glBegin(GL_QUADS);
    else
        glBegin(GL_LINE_LOOP);
    glVertex3f(x + radius, y, z);                   
    glVertex3f(x + width - radius, y, z);         
    glVertex3f(x + width - radius, y + height, z);  
    glVertex3f(x + radius, y + height, z);        
    glEnd();

    // Dessinez quatre coins arrondis (en utilisant des quarts de cercle)
    drawArc(x + radius, y + radius, z, radius, 180, 270, iscontour);// En bas à gauche
    drawArc(x + width - radius, y + radius, z, radius, 270, 360, iscontour);// en bas à droite
    drawArc(x + width - radius, y + height - radius, z, radius, 0, 90, iscontour);// en haut à droite
    drawArc(x + radius, y + height - radius, z, radius, 90, 180, iscontour);// en haut à gauche

    if (!iscontour)
        glBegin(GL_QUADS);
    else
        glBegin(GL_LINE_LOOP);
    glVertex3f(x, y + radius, z);                    
    glVertex3f(x + radius, y + radius, z);           
    glVertex3f(x + radius, y + height - radius, z);   
    glVertex3f(x, y + height - radius, z);       
    glEnd();

    if (!iscontour)
        glBegin(GL_QUADS);
    else
        glBegin(GL_LINE_LOOP);
    glVertex3f(x + width - radius, y + radius, z);   
    glVertex3f(x + width, y + radius, z);         
    glVertex3f(x + width, y + height - radius, z);  
    glVertex3f(x + width - radius, y + height - radius, z);        
    glEnd();
}

// Fonction pour dessiner une touche blanche
void drawWhiteKey(float x,bool isPressed, char label) {
    // Charger la texture de la touche blanche (seulement une fois)
    static GLuint texture = loadTexture("F:\\INFORV\\P1RV\\Piano_simulation\\Piano_simulation\\2.png");
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
    glVertex3f(-whiteKeyWidth / 2.0f-0.5f, -whiteKeyHeight / 2.0f-0.5f, -0.01f); 
    glTexCoord2f(texCoordWidth, 0.0f); 
    glVertex3f(whiteKeyWidth / 2.0f+0.5f, -whiteKeyHeight / 2.0f-0.5f, -0.01f); 
    glTexCoord2f(texCoordWidth, texCoordHeight); 
    glVertex3f(whiteKeyWidth / 2.0f+0.5f, whiteKeyHeight / 2.0f+2.5f, -0.01f); 
    glTexCoord2f(0.0f, texCoordHeight); 
    glVertex3f(-whiteKeyWidth / 2.0f-0.5f, whiteKeyHeight / 2.0f+2.5f, -0.01f); 
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Dessiner une ombre sous la touche avec un léger décalage
    glColor4f(0.05f, 0.05f, 0.05f, 0.4f);  // Couleur gris  pour l'ombre
    glPushMatrix();
    glTranslatef(0.13f, -0.13f, -0.13f); // Décalage de l'ombre
    drawRoundedRectangle(0.0, 0.0, 0.0f, whiteKeyWidth, whiteKeyHeight, 0.2f, 0);
    glPopMatrix();

    // Changer la couleur de la touche en fonction de son état (appuyée ou non)
    if (isPressed) {
        glColor3f(1.0f, 0.7f, 0.4f);
    }
    else {
        glColor3f(1.0f, 1.0f, 1.0f);
    }
    drawRoundedRectangle(0.0, 0.0, 1.0f, whiteKeyWidth, whiteKeyHeight, 0.2f, 0);
    glScalef(whiteKeyWidth, whiteKeyHeight, 1.0f);
    glTranslatef(0.0f, 0.0f, -3.0f);
    glutSolidCube(1.0);// Dessiner un cube solide représentant la touche
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
void drawBlackKey(float x, bool isPressed, char label) {
    glPushMatrix();
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 0.1f);
    // Dessiner l'ombre de la touche noire
    glColor4f(0.1f, 0.1f, 0.1f, 0.4f);
    glTranslatef(0.05f, -0.1f, -0.05f); //Décalage de l'ombre
    drawRoundedRectangle(0.0, 0.0, 0.7f, blackKeyWidth, blackKeyHeight, 0.2f, 0);
    glPopMatrix();

    // Définir la couleur de la touche noire en fonction de son état (appuyée ou non)
    glPushMatrix();
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 1.0f);

    if (isPressed) {
        glColor3f(1.0f, 0.7f, 0.4f);
    }
    else {
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
    }
    drawRoundedRectangle(0.0, 0.0, 1.0f, blackKeyWidth, blackKeyHeight, 0.15f, 0);
    glScalef(blackKeyWidth, blackKeyHeight, 1.0f);
    glutSolidCube(1.0);
    glPopMatrix();

    // Dessiner le contour de la touche noire pour créer un effet visuel
    glPushMatrix();
    // Déplacer légèrement pour que le contour soit devant la touche
    glTranslatef(x, whiteKeyHeight / 2.0f - blackKeyHeight / 2.0f, 0.5f);
    glColor3f(0.4f, 0.4f, 0.4f);
    glLineWidth(2.0f);  // Définir l'épaisseur des lignes pour le contour
    float scale = 1.005f;
    glScalef(scale, scale, scale);
    drawRoundedRectangle(0.0, 0.0, 1.0f, blackKeyWidth, blackKeyHeight, 0.2f, 1);

    glPopMatrix();
    drawText(string(1, label).c_str(), x - 0.15f, -0.8f, 2.5f, 1.0f, 1.0f, 1.0f);
}

// Fonction pour dessiner l'ensemble du piano
void drawPiano() {
    float whiteKeyX = -7.2f;
    float blackKeyOffsets[] = { 0.5f, 1.5f, 3.5f, 4.5f, 5.5f }; 
    float totalWhiteKeyWidthWithGap = whiteKeyWidth + gap; 

    for (int i = 0; i < 7; ++i) {
        drawWhiteKey(whiteKeyX + i *( whiteKeyWidth+gap), keyStatus[whiteKeys[i]],whiteKeys[i]);
        
    }
    for (int i = 0; i < 5; ++i) {
        float blackKeyX = whiteKeyX + (blackKeyOffsets[i] * totalWhiteKeyWidthWithGap);
        drawBlackKey(blackKeyX,  keyStatus[blackKeys[i]],blackKeys[i]);
    }
}

// Fonction pour afficher la scène dans la fenêtre
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    drawPiano();

    glutSwapBuffers();
}

void keyboardDown(unsigned char key, int x, int y) {
    key = toupper(key);
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
                keyStatus[blackKeys[i]] = true;
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
                keyStatus[whiteKeys[i]] = true;
                glutPostRedisplay();
                return;
            }
        }
    }
    else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
        for (auto& status : keyStatus) {
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
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboardDown);
    glutKeyboardUpFunc(keyboardUp);
    glutMouseFunc(mouse);

    glutMainLoop();
    return 0;
}

