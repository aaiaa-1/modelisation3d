#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <GL/glut.h>
#include <sstream>  // Pour std::istringstream

#ifndef HEADER_FILE_NAME_H
#define HEADER_FILE_NAME_H

#endif

// Structures de données pour les sommets et les faces
struct Vertex {
    float x, y, z;
};

struct Face {
    std::vector<int> vertexIndices;
};

struct TextureVertex {
    float u, v;
};

struct Normal {
    float dx, dy, dz;
};

struct Line {
    std::vector<int> vertexIndices;
};

struct Point {
    int vertexIndex;
};

// Ajoutons aussi des variables globales pour stocker ces données.
std::vector<TextureVertex> textureVertices;
std::vector<Normal> normals;
std::vector<Line> lines;
std::vector<Point> points;
std::string currentGroup;
std::string currentMaterial;
std::vector<std::string> materialLibraries;

// Variables globales
std::vector<Vertex> vertices;
std::vector<Face> faces;
float zoomFactor = 1.0f;
float rotationAngleX = 0.0f;
float rotationAngleY =0.0f;
float torsionFactor = 0.0f;
float bulgeFactor = 0.0f;  // Facteur de gonflement (1.0 = pas de changement)


float waveAmplitude = 0.0f;  // Amplitude de l'ondulation
float waveFrequency = 0.0f;  // Fréquence de l'ondulation
float waveTime = 0.0f;       // Paramètre temporel pour l'animation

// Variables pour la gestion de la souris
int lastMouseX, lastMouseY;
bool isLeftButtonPressed = false;
bool isRightButtonPressed = false;

void loadOBJ(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur: Impossible d'ouvrir le fichier " << filename << std::endl;
        return;
    }

    vertices.clear();
    textureVertices.clear();
    normals.clear();
    faces.clear();
    lines.clear();
    points.clear();
    currentGroup.clear();
    currentMaterial.clear();
    materialLibraries.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "v") {  // Sommets géométriques
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        } else if (prefix == "vt") {  // Sommets de texture
            TextureVertex vt;
            iss >> vt.u >> vt.v;
            textureVertices.push_back(vt);
        } else if (prefix == "vn") {  // Normales
            Normal n;
            iss >> n.dx >> n.dy >> n.dz;
            normals.push_back(n);
        } else if (prefix == "f") {  // Faces
            Face f;
            std::string vertexInfo;
            while (iss >> vertexInfo) {
                std::istringstream vertexStream(vertexInfo);
                std::string indexStr;
                int vertexIndex = -1;
                if (std::getline(vertexStream, indexStr, '/')) {
                    vertexIndex = std::stoi(indexStr) - 1;
                }
                f.vertexIndices.push_back(vertexIndex);
            }
            faces.push_back(f);
        } else if (prefix == "l") {  // Lignes
            Line l;
            int index;
            while (iss >> index) {
                l.vertexIndices.push_back(index - 1);
            }
            lines.push_back(l);
        } else if (prefix == "p") {  // Points
            Point p;
            iss >> p.vertexIndex;
            points.push_back(p);
        } else if (prefix == "g") {  // Groupe
            iss >> currentGroup;
        } else if (prefix == "usemtl") {  // Matériau
            iss >> currentMaterial;
        } else if (prefix == "mtllib") {  // Bibliothèque de matériaux
            std::string materialLib;
            iss >> materialLib;
            materialLibraries.push_back(materialLib);
        }
    }

    file.close();
    std::cout << "Fichier OBJ chargé avec succès : "
              << vertices.size() << " sommets, "
              << textureVertices.size() << " sommets de texture, "
              << normals.size() << " normales, "
              << faces.size() << " faces.\n";
}

void saveOBJ(const std::string& filename) {

    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erreur: Impossible de sauvegarder dans le fichier " << filename << std::endl;
        return;
    }

      for (auto& v : vertices) {
        // Appliquer la torsion
        float theta = torsionFactor * v.y;
        float newX = v.x * cos(theta) - v.z * sin(theta);
        float newZ = v.x * sin(theta) + v.z * cos(theta);
        v.x = newX;
        v.z = newZ;

        // Appliquer l'ondulation
        v.y += waveAmplitude * sin(waveFrequency * v.x + waveTime);

        // Appliquer le bulge (gonfler ou contracter)
        float distance = sqrt(v.x * v.x + v.z * v.z);  // Distance par rapport à l'axe Y
        float factor = 1.0f + bulgeFactor * distance;    // Appliquer le facteur de gonflement
        v.x *= factor;
        v.z *= factor;
    }

    // Sauvegarder les bibliothèques de matériaux
    for (const auto& matLib : materialLibraries) {
        file << "mtllib " << matLib << "\n";
    }

    // Sauvegarder les sommets géométriques
    for (const auto& v : vertices) {
        file << "v " << v.x << " " << v.y << " " << v.z << "\n";
    }

    // Sauvegarder les sommets de texture
    for (const auto& vt : textureVertices) {
        file << "vt " << vt.u << " " << vt.v << "\n";
    }

    // Sauvegarder les normales
    for (const auto& n : normals) {
        file << "vn " << n.dx << " " << n.dy << " " << n.dz << "\n";
    }

    // Sauvegarder les groupes et le matériau actif
    if (!currentGroup.empty()) {
        file << "g " << currentGroup << "\n";
    }
    if (!currentMaterial.empty()) {
        file << "usemtl " << currentMaterial << "\n";
    }

    // Sauvegarder les points
    for (const auto& p : points) {
        file << "p " << (p.vertexIndex + 1) << "\n";
    }

    // Sauvegarder les lignes
    for (const auto& l : lines) {
        file << "l";
        for (int index : l.vertexIndices) {
            file << " " << (index + 1);
        }
        file << "\n";
    }

    // Sauvegarder les faces
    for (const auto& f : faces) {
        file << "f";
        for (int index : f.vertexIndices) {
            file << " " << (index + 1);
        }
        file << "\n";
    }

    file.close();
    std::cout << "Fichier OBJ sauvegardé sous : " << filename << "\n";
}

// Création d'un objet par défaut
void createDefaultObject() {
    vertices = {
        {0.0f, 0.0f, 0.0f},
        {1.0f, 0.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        {0.0f, 0.0f, 1.0f}
    };

    faces = {
        {{0, 1, 2}},
        {{0, 2, 3}},
        {{0, 1, 3}},
        {{1, 2, 3}}
    };
    std::cout << "Un objet par défaut a été créé.\n";
}

// Fonction pour dessiner une grille
void drawGrid(float size, int divisions) {
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_LINES);

    for (int i = -divisions; i <= divisions; ++i) {
        glVertex3f(-size, 0.0f, i * size / divisions);
        glVertex3f(size, 0.0f, i * size / divisions);

        glVertex3f(i * size / divisions, 0.0f, -size);
        glVertex3f(i * size / divisions, 0.0f, size);
    }

    glEnd();
}
// Fonction d'affichage
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glScalef(zoomFactor, zoomFactor, zoomFactor);
    glRotatef(rotationAngleX, 1.0f, 0.0f, 0.0f);
    glRotatef(rotationAngleY, 0.0f, 1.0f, 0.0f);

    drawGrid(4.0f, 40);

    // Appliquer la torsion
    std::vector<Vertex> transformedVertices = vertices;
    for (auto& v : transformedVertices) {
        float theta = torsionFactor * v.y;
        float newX = v.x * cos(theta) - v.z * sin(theta);
        float newZ = v.x * sin(theta) + v.z * cos(theta);
        v.x = newX;
        v.z = newZ;

        v.y += waveAmplitude * sin(waveFrequency * v.x + waveTime);

        // Appliquer le bulge (gonfler ou contracter)
        float distance = sqrt(v.x * v.x + v.z * v.z);  // Distance par rapport à l'axe Y
        float factor = 1.0f + bulgeFactor * distance;    // Appliquer le facteur de gonflement
        v.x *= factor;
        v.z *= factor;

    }

    glBegin(GL_TRIANGLES);
    for (size_t i = 0; i < faces.size(); ++i) {
        const auto& f = faces[i];
        glColor3f((i % 3) / 2.0f, ((i + 1) % 3) / 2.0f, ((i + 2) % 3) / 2.0f);

        for (int index : f.vertexIndices) {
            const auto& v = transformedVertices[index];
            glVertex3f(v.x, v.y, v.z);
        }
    }
    glEnd();

    glutSwapBuffers();
}

// Gestion des touches
void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case '+': zoomFactor *= 1.1f; break;
        case '-': zoomFactor /= 1.1f; break;
        case 'x': rotationAngleX += 5.0f; break;
        case 'X': rotationAngleX -= 5.0f; break;
        case 'y': rotationAngleY += 5.0f; break;
        case 'Y': rotationAngleY -= 5.0f; break;
        case 't': torsionFactor += 0.08f; break;
        case 'T': torsionFactor -= 0.08f; break;
        case 'w': waveFrequency += 0.08f; break;
        case 'W': waveFrequency -= 0.08f; break;   // Diminuer la fréquence
        case 'a': waveAmplitude += 0.02f; break;   // Augmenter l'amplitude
        case 'A': waveAmplitude -= 0.02f; break;   // Diminuer l'amplitude
        case 'p': waveTime += 0.1f; break;         // Avancer le temps de l'ondulation
        case 'P': waveTime -= 0.1f; break;         // Reculer le temps de l'ondulation
        case 'b': bulgeFactor += 0.1f; break;      // Augmenter le facteur de gonflement
        case 'B': bulgeFactor -= 0.1f; break;      // Diminuer le facteur de gonflement
        case 's': saveOBJ("modified.obj"); break;
        default: break;
    }
    glutPostRedisplay();
}

// Gestion des clics de la souris
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON) {
        isLeftButtonPressed = (state == GLUT_DOWN);
    } else if (button == GLUT_RIGHT_BUTTON) {
        isRightButtonPressed = (state == GLUT_DOWN);
    }
    lastMouseX = x;
    lastMouseY = y;
}

// Gestion des mouvements de la souris
void mouseMotion(int x, int y) {
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    if (isLeftButtonPressed) {
        rotationAngleX += dy * 0.5f;
        rotationAngleY += dx * 0.5f;
    }

    if (isRightButtonPressed) {
        zoomFactor += dy * 0.01f;
        if (zoomFactor < 0.1f) zoomFactor = 0.1f;
    }

    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();
}


// Main
int main(int argc, char** argv) {
    int choix;
    std::cout << "Choisissez une option:\n1. Charger un fichier OBJ\n2. Creer un objet par defaut\n";
    std::cin >> choix;

    if (choix == 1) {
        std::string filename;
        std::cout << "Entrez le nom du fichier OBJ a charger: ";
        std::cin >> filename;
        loadOBJ(filename);
    } else if (choix == 2) {
        createDefaultObject();
    } else {
        std::cerr << "Choix invalide. Fin du programme.\n";
        return 1;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Visualisation OBJ");

    glEnable(GL_DEPTH_TEST);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMotion);
    glutMainLoop();

    return 0;
}

