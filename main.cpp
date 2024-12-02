#include <SDL.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <iostream>
#include <GLFW/glfw3.h>



// Dimensions de la fenêtre
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

int main(int argc, char* argv[]) {
    // Initialisation de SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "Erreur d'initialisation de SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Initialisation de SDL_Mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Erreur d'initialisation de SDL_Mixer: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Chargement des sons
    Mix_Chunk* soundA = Mix_LoadWAV("assets/sounds/note_a.mp3");
    Mix_Chunk* soundB = Mix_LoadWAV("assets/sounds/note_b.mp3");
    if (!soundA || !soundB) {
        std::cerr << "Erreur de chargement des sons: " << Mix_GetError() << std::endl;
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Création de la fenêtre
    SDL_Window* window = SDL_CreateWindow("Piano SDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Erreur de création de la fenêtre: " << SDL_GetError() << std::endl;
        Mix_FreeChunk(soundA);
        Mix_FreeChunk(soundB);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Création du renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Erreur de création du renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        Mix_FreeChunk(soundA);
        Mix_FreeChunk(soundB);
        Mix_CloseAudio();
        SDL_Quit();
        return -1;
    }

    // Boucle principale
    bool running = true;
    SDL_Event event;

    // États des touches
    bool keyAPressed = false;
    bool keyBPressed = false;

    bool keyAReleased = true; // Track release state for A key
    bool keyBReleased = true; // Track release state for B key

    while (running) {
        // Gestion des événements
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_a && keyAReleased) {
                    keyAPressed = true;  // Mark key as pressed
                    keyAReleased = false; // Prevent re-triggering until released
                    Mix_PlayChannel(-1, soundA, 0); // Jouer le son A
                }
                else if (event.key.keysym.sym == SDLK_b && keyBReleased) {
                    keyBPressed = true;  // Mark key as pressed
                    keyBReleased = false; // Prevent re-triggering until released
                    Mix_PlayChannel(-1, soundB, 0); // Jouer le son B
                }
            }
            else if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_a) {
                    keyAPressed = false;  // Mark key as released
                    keyAReleased = true;
                }
                else if (event.key.keysym.sym == SDLK_b) {
                    keyBPressed = false;  // Mark key as released
                    keyBReleased = true;
                }
            }
        }

        // Couleur d'arrière-plan
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Dessiner la touche A
        SDL_Rect pianoKeyA = { 200, 200, 150, 400 }; // Position et taille de la touche A
        if (keyAPressed) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Rouge si pressée
        }
        else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir sinon
        }
        SDL_RenderFillRect(renderer, &pianoKeyA);

        // Dessiner la touche B
        SDL_Rect pianoKeyB = { 400, 200, 150, 400 }; // Position et taille de la touche B
        if (keyBPressed) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // Bleu si pressée
        }
        else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Noir sinon
        }
        SDL_RenderFillRect(renderer, &pianoKeyB);

        // Mise à jour de l'affichage
        SDL_RenderPresent(renderer);
    }

    // Nettoyage
    Mix_FreeChunk(soundA);
    Mix_FreeChunk(soundB);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
