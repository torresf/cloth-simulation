#include "PartyKel/WindowManager.hpp"

#include <GL/glew.h>
#include <iostream>
#include <stdexcept>

namespace PartyKel {

WindowManager::WindowManager(uint32_t w, uint32_t h, const char *title) {
    setFramerate(30);

    if(-1 == SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error("Unable to initialize SDL");
    }

    if(!SDL_SetVideoMode(w, h, 32, SDL_OPENGL)) {
        throw std::runtime_error("Unable to open a window");
    }
    SDL_WM_SetCaption(title, 0);

    GLenum error = glewInit();
    if(error != GLEW_OK) {
        throw std::runtime_error("Unable to init GLEW: " + std::string((const char*) glewGetErrorString(error)));
    }
}

WindowManager::~WindowManager() {
    SDL_Quit();
}

float WindowManager::update() {
    SDL_GL_SwapBuffers();

    Uint32 currentTime = SDL_GetTicks();
    Uint32 d = currentTime - m_nStartTime;
    if(d < m_nFrameDuration) {
        SDL_Delay(m_nFrameDuration - d);
    }
    return 0.01f * (SDL_GetTicks() - m_nStartTime);
}

}
