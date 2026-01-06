#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace std;

// Estructuras básicas para el cargador OBJ
struct Vertice { float x, y, z; };
struct Cara { int v1, v2, v3; };

struct Modelo {
    vector<Vertice> vertices;
    vector<Cara> caras;
    GLuint textureID;
};

// Variables para las piezas del minicargador
Modelo cabina, brazoDer, brazoIzq, cuchara;

GLuint cargarTextura(const char* ruta) {
    GLuint id;
    int ancho, alto, canales;

    // 1. Invertir la imagen al cargar (importante para modelos de Blender)
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(ruta, &ancho, &alto, &canales, 0);

    if (data) {
        glGenTextures(1, &id);
        glBindTexture(GL_TEXTURE_2D, id);

        // 2. Configuración tradicional de envoltura y filtrado
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLenum formato = (canales == 4) ? GL_RGBA : GL_RGB;

        // 3. CARGA TRADICIONAL (Usa GLU para generar mipmaps automáticamente)
        gluBuild2DMipmaps(GL_TEXTURE_2D, canales, ancho, alto, formato, GL_UNSIGNED_BYTE, data);

        std::cout << "EXITO: Textura cargada [" << ruta << "]" << std::endl;
    }
    else {
        std::cout << "ERROR: No se encontro la textura en: " << ruta << std::endl;
        return 0;
    }

    stbi_image_free(data);
    return id;
}

// --- CARGADOR OBJ BÁSICO ---
Modelo cargarOBJ(string ruta) {
    Modelo mod;
    ifstream archivo(ruta);
    string linea;
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        string prefijo;
        ss >> prefijo;
        if (prefijo == "v") {
            Vertice v;
            ss >> v.x >> v.y >> v.z;
            mod.vertices.push_back(v);
        }
        else if (prefijo == "f") {
            Cara c;
            string v1, v2, v3;
            ss >> v1 >> v2 >> v3;
            // Extraer solo el índice del vértice (antes del '/')
            c.v1 = stoi(v1.substr(0, v1.find("/"))) - 1;
            c.v2 = stoi(v2.substr(0, v2.find("/"))) - 1;
            c.v3 = stoi(v3.substr(0, v3.find("/"))) - 1;
            mod.caras.push_back(c);
        }
    }
    return mod;
}
void dibujarEjes() {
    glDisable(GL_LIGHTING); // Desactivamos luces para ver los colores puros de las lineas
    glLineWidth(3.0f);      // Lineas gruesas para que se vean bien

    glBegin(GL_LINES);
    // Eje X - Rojo
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(50.0f, 0.0f, 0.0f);

    // Eje Y - Verde
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 50.0f, 0.0f);

    // Eje Z - Azul
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 50.0f);
    glEnd();

    glEnable(GL_LIGHTING); // Reactivamos luces para los modelos
    glLineWidth(1.0f);
}
void dibujarModelo(Modelo m, float r, float g, float b) {
    glDisable(GL_LIGHTING); // Desactivamos luces para ver el color puro
    glColor3f(r, g, b);     // Aplicamos el color manual

    glBegin(GL_TRIANGLES);
    for (const auto& cara : m.caras) {
        // Validacion de seguridad para evitar crashes
        if (cara.v1 < m.vertices.size() && cara.v2 < m.vertices.size() && cara.v3 < m.vertices.size()) {
            glVertex3f(m.vertices[cara.v1].x, m.vertices[cara.v1].y, m.vertices[cara.v1].z);
            glVertex3f(m.vertices[cara.v2].x, m.vertices[cara.v2].y, m.vertices[cara.v2].z);
            glVertex3f(m.vertices[cara.v3].x, m.vertices[cara.v3].y, m.vertices[cara.v3].z);
        }
    }
    glEnd();
    glEnable(GL_LIGHTING);
}

void inicializar() {
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    // Cargar las piezas que modelaste en Blender
    // Asegúrate de que las rutas coincidan con tu carpeta modelos/
    cabina = cargarOBJ("modelos/minicargador_cabina.obj");
    brazoDer = cargarOBJ("modelos/minicargador_brazo_derecho.obj");
    brazoIzq = cargarOBJ("modelos/minicargador_brazo_izquierdo.obj");
    cuchara = cargarOBJ("modelos/minicargador_cuchara.obj");

    // Textura provisional (puedes poner un .jpg en la carpeta)
    // cabina.textureID = cargarTextura("texturas/metal.jpg");
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    // Camara (Ajustada para ver un area grande)
    gluLookAt(60.0, 40.0, 100.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    dibujarEjes();

    // --- DIBUJAR PIEZAS DEL MINICARGADOR ---
    glPushMatrix();
    // TRUCO DE ESCALA: Si no lo ves, aumenta este numero (ej. 10.0 o 50.0)
    glScalef(200.0f, 200.0f, 200.0f);

    // Dibujamos cada pieza con los colores que tenias en Blender (manual)
    dibujarModelo(cabina, 0.9f, 0.7f, 0.0f);      // Amarillo Maquinaria
    dibujarModelo(brazoDer, 0.2f, 0.2f, 0.2f);    // Gris Oscuro
    dibujarModelo(brazoIzq, 0.2f, 0.2f, 0.2f);    // Gris Oscuro
    dibujarModelo(cuchara, 0.4f, 0.4f, 0.4f);     // Gris Metalico

    // Agrega aqui las demas piezas que subiste (llantas, pedales, etc.)
    glPopMatrix();

    glutSwapBuffers();
}
void redimensionar(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (float)w / h, 0.1, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1600, 900);
    glutCreateWindow("Simulacion de Maquinaria Pesada");

    inicializar();

    glutDisplayFunc(display);
    glutReshapeFunc(redimensionar);
    glutIdleFunc(display);

    glutMainLoop();
    return 0;
}