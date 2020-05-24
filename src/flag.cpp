#include <iostream>
#include <cstdlib>

#include <PartyKel/glm.hpp>
#include <PartyKel/WindowManager.hpp>
#include <glm/gtc/random.hpp>

#include <PartyKel/renderer/FlagRenderer3D.hpp>
#include <PartyKel/renderer/TrackballCamera.hpp>
#include <PartyKel/renderer/Renderer3D.hpp>
#include <PartyKel/renderer/Sphere.hpp>
#include <PartyKel/atb.hpp>
#include <PartyKel/Octree.hpp>

#include <vector>

static const Uint32 WINDOW_WIDTH = 900;
static const Uint32 WINDOW_HEIGHT = 700;

using namespace PartyKel;

// Calcule une force de type ressort de Hook entre deux particules de positions P1 et P2
// K est la résistance du ressort et L sa longueur à vide
inline glm::vec3 hookForce(float K, float L, const glm::vec3& P1, const glm::vec3& P2) {
    static const float epsilon = 0.0001;
    return K * (1-(L/std::max(glm::distance(P1, P2), epsilon))) * (P2 - P1);
}

inline glm::vec3 repulseForce(float dst, const glm::vec3& P1, const glm::vec3& P2) {
    glm::vec3 direction = glm::normalize(P1 - P2);
    return direction * (1 / (1 + glm::pow(dst, 2.f)));
}

// Calcule une force de type frein cinétique entre deux particules de vélocités v1 et v2
// V est le paramètre du frein
// dt est le pas temporel (delta time)
inline glm::vec3 brakeForce(float V, float dt, const glm::vec3& v1, const glm::vec3& v2) {
    return V * ((v2-v1) / dt);
}

inline glm::vec3 sphereCollisionForce(float distanceToCenter, 
                                      const glm::vec3& sphereCenter,
                                      float sphereRadius, 
                                      const glm::vec3 particlePosition, 
                                      const glm::vec3& forceParticle) {
    glm::vec3 direction = glm::normalize(particlePosition - sphereCenter);
    return direction * (1 / (1 + glm::pow(distanceToCenter, 2.f)));
}

// Structure permettant de simuler un drapeau à l'aide un système masse-ressort
struct Flag {
    int gridWidth, gridHeight; // Dimensions de la grille de points

    // Propriétés physique des points:
    std::vector<glm::vec3> positionArray;
    std::vector<glm::vec3> velocityArray;
    std::vector<float> massArray;
    std::vector<glm::vec3> forceArray;
    int nbParticles;

    // Paramètres des forces interne de simulation
    // Longueurs à vide
    glm::vec2 L0;
    float L1;
    glm::vec2 L2;

    float K0, K1, K2; // Paramètres de résistance
    float V0, V1, V2; // Paramètres de frein

    // Créé un drapeau discretisé sous la forme d'une grille contenant gridWidth * gridHeight
    // points. Chaque point a pour masse : mass / (gridWidth * gridHeight).
    // La taille du drapeau en 3D est spécifié par les paramètres width et height
    Flag(float mass, float width, float height, int gridWidth, int gridHeight):
            gridWidth(gridWidth), gridHeight(gridHeight),
            positionArray(gridWidth * gridHeight),
            velocityArray(gridWidth * gridHeight, glm::vec3(0.f)),
            massArray(gridWidth * gridHeight, mass / (gridWidth * gridHeight)),
            forceArray(gridWidth * gridHeight, glm::vec3(0.f)) {

        // glm::vec3 origin(-0.5f * width, -0.5f * height, 0.f);
        glm::vec3 origin(-0.5f * width, 0.f, 0.f);
        glm::vec3 scale(width / (gridWidth - 1), height / (gridHeight - 1), 1.f);

        nbParticles = gridWidth * gridHeight;
        for (int j = 0; j < gridHeight; ++j) {
            for (int i = 0; i < gridWidth; ++i) {
                int k = i + j * gridWidth;
                positionArray[k] = origin + glm::vec3(i, j, origin.z) * scale;
                massArray[k] = 1 - ( i / (2*(gridHeight*gridWidth)));
            }
        }

        // Les longueurs à vide sont calculés à partir de la position initiale
        // des points sur le drapeau
        L0.x = scale.x;
        L0.y = scale.y;
        L1 = glm::length(L0);
        L2 = 4.f * L0;

        // Paramètres à fixer pour avoir un système stable
        K0 = 1;
        K1 = 1;
        K2 = 1;

        V0 = 0.08;
        V1 = 0.02;
        V2 = 0.06;
    }

    // Applique les forces internes sur chaque point du drapeau SAUF les points fixes
    void applyInternalForces(float dt) {
        std::vector<glm::ivec2> neighbors(4);
        for (int i = 0; i < gridWidth; ++i) {
            for (int j = 0; j < gridHeight-1; ++j) {
                int currentK = j*gridWidth + i;

                // TOPOLOGY 1
                neighbors[0] = glm::ivec2(i+1, j);
                neighbors[1] = glm::ivec2(i-1, j);
                neighbors[2] = glm::ivec2(i, j-1);
                neighbors[3] = glm::ivec2(i, j+1);

                int tmpI = 0;
                for (auto& p : neighbors) {
                    if (p.x < 0 || p.y < 0 || p.x >= gridWidth || p.y >= gridHeight)
                        continue;
                    int k = p.y * gridWidth + p.x;
                    forceArray[currentK] += hookForce(K0, tmpI < 2 ? L0.x : L0.y, positionArray[currentK], positionArray[k]);
                    forceArray[currentK] += brakeForce(V0, dt, velocityArray[currentK], velocityArray[k]);
                    ++tmpI;
                }

                // TOPOLOGY 2
                neighbors[0] = glm::ivec2(i-1, j-1);
                neighbors[1] = glm::ivec2(i+1, j-1);
                neighbors[2] = glm::ivec2(i+1, j+1);
                neighbors[3] = glm::ivec2(i-1, j+1);

                for (auto& p : neighbors) {
                    if (p.x < 0 || p.y < 0 || p.x >= gridWidth || p.y >= gridHeight)
                        continue;
                    int k = p.y * gridWidth + p.x;
                    forceArray[currentK] += hookForce(K1, L1, positionArray[currentK], positionArray[k]);
                    forceArray[currentK] += brakeForce(V1, dt, velocityArray[currentK], velocityArray[k]);
                }

                // TOPOLOGY 3
                neighbors[0] = glm::ivec2(i-2, j);
                neighbors[1] = glm::ivec2(i+2, j);
                neighbors[2] = glm::ivec2(i, j-2);
                neighbors[3] = glm::ivec2(i, j+2);

                tmpI = 0;
                for (auto& p : neighbors) {
                    if (p.x < 0 || p.y < 0 || p.x >= gridWidth || p.y >= gridHeight)
                        continue;
                    int k = p.y * gridWidth + p.x;
                    forceArray[currentK] += hookForce(K2, tmpI < 2 ? L2.x : L2.y, positionArray[currentK], positionArray[k]);
                    forceArray[currentK] += brakeForce(V2, dt, velocityArray[currentK], velocityArray[k]);
                    ++tmpI;
                }

            }
        }
    }

    void applyRepulseForces(Octree<glm::vec3>& octree, float maxDst, float multRepulse) {
        for (int i = 0; i < gridWidth; ++i) {
            for (int j = 0; j < gridHeight-1; ++j) {
                int k = j*gridWidth + i;
                auto& pos = positionArray[k];

                auto &inSameVoxel = octree.get(pos);
                assert(!inSameVoxel.empty());

                if (inSameVoxel.size() < 2)
                    continue;

                for (auto &v : inSameVoxel) {
                    float dst = glm::distance(v, pos);
                    if (dst > maxDst || pos == v)
                        continue;

                    forceArray[k] += repulseForce(dst, pos, v) * multRepulse;
                }
            }
        }
    }

    // Applique une force externe sur chaque point du drapeau SAUF les points fixes
    void applyExternalForce(const glm::vec3& F) {
        for (int i = 0; i < nbParticles; ++i) {
            // if (i % gridWidth == 0) continue;
            if (i > nbParticles - gridWidth-1) continue;
            forceArray[i] += F;
        }
    }

    void applySphereCollision(const SphereHandler& sphereHandler, float multiplier, float radiusDelta) {
        for (int i = 0; i < nbParticles; ++i) {
            // if (i % gridWidth == 0) continue;
            if (i > nbParticles - gridWidth-1) continue;

            for (size_t j = 0; j < sphereHandler.positions.size(); ++j) {
                float dist = glm::distance(sphereHandler.positions[j], positionArray[i]);
                if (dist < sphereHandler.radius[j] + radiusDelta) {
                    forceArray[i] += sphereCollisionForce(dist, sphereHandler.positions[j], sphereHandler.radius[j], positionArray[i], forceArray[i]) * multiplier;
                }
            }
        }
    }

    // Met à jour la vitesse et la position de chaque point du drapeau
    // en utilisant un schema de type Leapfrog
    void update(float dt) {
        for (int i = 0; i < nbParticles ; ++i) {
            velocityArray[i] += dt * (forceArray[i]/massArray[i]);
            positionArray[i] += dt * velocityArray[i];
            forceArray[i] = glm::vec3(0);
        }
    }
};

int main() {

    SphereHandler sphereHandler;
    sphereHandler.colors = {glm::vec3(1, 0, 0), glm::vec3(1, 1, 0), glm::vec3(0, 1, 0)};
    sphereHandler.positions = {glm::vec3(0, -3, 2), glm::vec3(1.5, -3.5, 0.7), glm::vec3(3, -2, -1.5)};
    sphereHandler.radius = {2, 1.5, .8};
    float sphereCollisionMultiplier = 1.5;
    float radiusDelta = 0.15;
    glm::ivec2 flagGrid = glm::ivec2(70, 30);
    glm::ivec2 flagSize = glm::ivec2(8, 3);
    float flagMass = 1.f;

    WindowManager wm(WINDOW_WIDTH, WINDOW_HEIGHT, "Flag Simulation");
    wm.setFramerate(60);

    // Initialisation de AntTweakBar (pour la GUI)
    TwInit(TW_OPENGL, NULL);
    TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

    Flag flag(flagMass, flagSize.x, flagSize.y, flagGrid.x, flagGrid.y); // Création d'un drapeau
    glm::vec3 G(0.f, -0.05f, 0.f); // Gravité

    float maxDstRepulseForce    = 0.17;
    float multRepulseForce      = 0.1;
    bool activeSpheres          = true;
    bool activeAutoCollisions   = true;
    bool wireframe              = false;

    FlagRenderer3D renderer(flag.gridWidth, flag.gridHeight);
    Octree<glm::vec3> octree(7, glm::vec3(0,-10,0), glm::vec3(50.f));

    glm::mat4 projection = glm::perspective(70.f, float(WINDOW_WIDTH) / WINDOW_HEIGHT, 0.1f, 100.f);

    renderer.setProjMatrix(projection);
    TwBar* gui = TwNewBar("Parametres");

    float randomMoveScale = 0.01f;
    float windVelocity = 0.025f;
    float newWindVelocity = windVelocity;

    atb::addVarRW(gui, ATB_VAR(flag.K0), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.K1), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.K2), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.V0), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.V1), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(flag.V2), "step=0.01");

    atb::addVarRW(gui, ATB_VAR(sphereCollisionMultiplier), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(sphereHandler.radius[0]), "label='Sphere radius' step=0.01");
    atb::addVarRW(gui, ATB_VAR(sphereHandler.positions[0].x), "label='Sphere x pos' step=0.03");
    atb::addVarRW(gui, ATB_VAR(radiusDelta), "step=0.01");

    atb::addVarRW(gui, ATB_VAR(maxDstRepulseForce), "step=0.01");
    atb::addVarRW(gui, ATB_VAR(multRepulseForce), "step=0.01");

    atb::addVarRW(gui, ATB_VAR(newWindVelocity), "label='Wind velocity' step=0.02");

    atb::addVarRW(gui, ATB_VAR(activeAutoCollisions));
    atb::addVarRW(gui, ATB_VAR(activeSpheres));
    atb::addVarRW(gui, ATB_VAR(wireframe));

    atb::addButton(gui, "Reset", [&]() {
        Flag tmp(flagMass, flagSize.x, flagSize.y, flagGrid.x, flagGrid.y); // Création d'un drapeau
        tmp.K0 = flag.K0;
        tmp.K1 = flag.K1;
        tmp.K2 = flag.K2;
        tmp.V0 = flag.V0;
        tmp.V1 = flag.V1;
        tmp.V2 = flag.V2;
        flag = tmp;
    });

    TrackballCamera camera;
    camera.moveFront(12);
    int mouseLastX, mouseLastY;

    // Delta Time : temps s'écoulant entre chaque frame
    float dt = 0.f;

    Renderer3D renderer3D;
    renderer3D.setProjMatrix(projection);

    float spherePosZ = sphereHandler.positions[0].z;
    float spherePosY = sphereHandler.positions[0].y;
    float moveStep = 1.f;

    bool done = false;
    while(!done) {
        wm.startMainLoop();

        // Rendu
        renderer.clear();
        renderer.setViewMatrix(camera.getViewMatrix());
        renderer3D.setViewMatrix(camera.getViewMatrix());
        renderer.drawGrid(flag.positionArray.data(), wireframe);

        if (activeSpheres)
            renderer3D.drawParticles(sphereHandler.positions.size(), sphereHandler.positions.data(), sphereHandler.radius.data(), sphereHandler.colors.data(), 1);

        // Simulation
        if (dt > 0.f) {
            flag.applyExternalForce(G); // Applique la gravité
            flag.applyExternalForce(glm::sphericalRand(windVelocity)); // Applique un "vent" de direction aléatoire et de force 0.25 Newtons
            flag.applyInternalForces(dt); // Applique les forces internes

            if (activeSpheres)
                flag.applySphereCollision(sphereHandler, sphereCollisionMultiplier, radiusDelta);

            for (auto& pos : flag.positionArray)
                octree.add(pos, pos);
            
            if (activeAutoCollisions)
                flag.applyRepulseForces(octree, maxDstRepulseForce, multRepulseForce);

            for (auto& pos : flag.positionArray)
                octree.remove(pos, pos);

            flag.update(dt); // Mise à jour du système à partir des forces appliquées
        }

        // GUI Display
        TwDraw();

        // Gestion des evenements
        SDL_Event e;
        while(wm.pollEvent(e)) {
            int handled = TwEventSDL(&e, SDL_MAJOR_VERSION, SDL_MINOR_VERSION);

            switch(e.type) {
                case SDL_QUIT:
                    done = true;
                    break;
                case SDL_KEYDOWN:
                    if (e.key.keysym.sym == SDLK_SPACE) {
                        wireframe = !wireframe;
                    }
                    if (e.key.keysym.sym == SDLK_ESCAPE) {
                        done = true;
                        break;
                    }
                    if (e.key.keysym.sym == SDLK_UP) {
                        spherePosY += moveStep;
                        break;
                    }
                    if (e.key.keysym.sym == SDLK_DOWN) {
                        spherePosY -= moveStep;
                        break;
                    }
                    if (e.key.keysym.sym == SDLK_RIGHT) {
                        spherePosZ -= moveStep;
                        break;
                    }
                    if (e.key.keysym.sym == SDLK_LEFT) {
                        spherePosZ += moveStep;
                        break;
                    }
                    if (e.key.keysym.sym == SDLK_KP_PLUS) {
                        newWindVelocity += .01f;
                        break;
                    }
                    if (e.key.keysym.sym == SDLK_KP_MINUS) {
                        newWindVelocity -= .01f;
                        break;
                    }
                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_WHEELUP) {
                        camera.moveFront(-0.4f);
                    } else if (e.button.button == SDL_BUTTON_WHEELDOWN) {
                        camera.moveFront(0.4f);
                    }
                    else if (e.button.button == SDL_BUTTON_LEFT) {
                        mouseLastX = e.button.x;
                        mouseLastY = e.button.y;
                    }
                default:
                    break;
            }
        }

        int mouseX, mouseY;
        if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            float dX = mouseX - mouseLastX, dY = mouseY - mouseLastY;
            camera.rotateLeft(glm::radians(dX));
            camera.rotateUp(glm::radians(dY));
            mouseLastX = mouseX;
            mouseLastY = mouseY;
        }

        sphereHandler.positions[0].z = glm::mix(sphereHandler.positions[0].z, spherePosZ, .08);
        sphereHandler.positions[0].y = glm::mix(sphereHandler.positions[0].y, spherePosY, .08);
        windVelocity = glm::mix(windVelocity, newWindVelocity, .08);

        // Mise à jour de la fenêtre
        dt = wm.update();
    }

    return EXIT_SUCCESS;
}
