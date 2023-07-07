#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include "SOIL/SOIL.h"
#include <math.h>
#include <string.h>
#include <math.h>
#define M_PI		3.14159265358979323846
#define LARGURA_DO_MUNDO 600
#define ALTURA_DO_MUNDO 600

// Ângulo de rotação do cubo
float angle = 0.0; // Ângulo de rotação do cubo
float cameraX = 0.0; // Posição da câmera no eixo X
float cameraY = 1.5; // Posição da câmera no eixo Y
float cameraZ = -5.0; // Posição da câmera no eixo Z

int lastMouseX = 0; // Última posição X do mouse
int lastMouseY = 0; // Última posição Y do mouse
float zoom = 1.0; 
float zoomSpeed = 0.1; // Velocidade de zoom da câmera
float cameraPosZ=6;
int numVertices;
int numNormais;
int numFaces;
int numTexturas;
float rotacaoHelice;
float corfog[] = { 0.27,0.51,0.71 };
float densidadeFog;

typedef struct {
    float x, y, z;
} Vertex;

typedef struct {
    float x, y, z;
} Normal;

typedef struct{
    int v1,v2,v3,v4; //vertices da face
    int vt1,vt2,vt3,vt4; //vertices da textura
    int f; //qual face
}Face;

typedef struct{
    int v1,v2,v3; //vertices da face
    int vt1,vt2,vt3; //vertices da textura
    int f; //qual face
}Face3;

typedef struct{
    float x, y;
}Textura;

/*typedef struct{
    GLfloat posicao[4];
    GLfloat iluminacao[4];

    //Para Luz do tipo holofote
    int direcao[3];
}Luz;*/
typedef struct{
    GLfloat ambiente[4];
    GLfloat difusa[4];
    GLfloat especular[4];
    GLfloat posicao[4];
    int direcao[3];
}Luz;

typedef struct{
    GLfloat ambiente[4];
    GLfloat difusa[4];
    GLfloat especular[4];
    GLfloat brilhosidade;
}Material;


Vertex* vertices;
Normal* normais;
Face* faces;
Face3* faces3;
Textura* textura;


//Esse 1 em posição define que será holofote;
Luz sol = {{1.0, 1.0, 1.0, 1}, {1.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 1.0},{1.0, 1.0, 1.0, 1.0}, {-2, 1, -2}};
Luz sala = {{0, 0, 0, 1}, {1.0, 1.0, 1.0, 1.0}, {0, 0, 0, 0},{-5.047848, 3.089242, -3.188985, 1.0}, {-2, 1, -2}};
Luz cozinha = {{1.0, 1.0, 1.0, 1}, {0, 0, 0, 1.0}, {0, 0, 0, 1.0},{-5.047848, 3.089242, 2.224503, 1.0}, {-2, 1, -2}};
Luz quarto = {{1, 0, 1, 1}, {0, 1, 0, 1.0}, {1.0, 1.0, 1.0, 1.0},{4.263586, 2.989242, -3.088985, 1.0}, {-2, 1, -2}};
Luz banheiro = {{0.5, 0.5, 0.5, 1}, {0.5, 0.5, 0.5, 1.0}, {0.5, 0.5, 0.5, 1.0},{4.717563, 2.989242, 2.324503, 1.0}, {-2, 1, -2}};

//configuracoes com q cada um reage com cada tipo de luz
Material sofa = {{0.1745, 0.01175, 0.01175, 0.55},{0.61424, 0.04136, 0.04136, 0.55},{0.727811, 0.626959, 0.626959, 0.55}, 76.8};
Material paredes = {{0.135, 0.2225, 0.1575, 0.95},{0.54, 0.89, 0.63, 0.95},{0.316228, 0.316228, 0.316228, 0.95}, 12.8};
Material cinza = {{0.25, 0.25, 0.25, 1.0},{0.4, 0.4, 0.4, 1.0},{0.774597, 0.774597, 0.774597, 1.0},76.8};
Material madeira = {{0.1, 0.1, 0.1, 1},{ 0.49, 0.22, 0.02, 1.0},{ 0, 0, 0, 1}, 0.0};
Material folhas = {{0.02, 0.175, 0.021, 0.55}, {0.075, 0.61, 0.075, 0.55}, {0.63, 0.727, 0.63, 0.55}, 128.0};
Material helice = {{0.32, 0.22, 0.02, 1.0}, {0.78, 0.56, 0.11, 1.0},{0.99, 0.94, 0.80, 1}, 27.9};
Material chaoPatio = {{0.25, 0.20, 0.20, 1.0},{1.0, 0.82, 0.82, 1.0},{0.29, 0.29, 0.29, 1.0}, 12.0};

int acesa=1;
int acesa_sala=1;
int acesa_cozinha=1;
int acesa_quarto=1;
int acesa_banheiro=1;
float coeficienteLuz = 1; 

GLuint texturaCimento, texturatronco, texturaBancoMesa, texturaGrama, texturaRipado, texturaPorcelanato, texturaBalcao, texturaGeladeira, texturaFogao, texturaCama, texturaSofa, texturaTv, texturaChuveiro;

int listaArvore, listaBancoMesa, listaChao, listaBaseMoinho, listaPino, listaCasa;

GLuint carregaTextura(const char* arquivo){
    GLuint idTextura = SOIL_load_OGL_texture(arquivo, SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_INVERT_Y);
    if(idTextura==0){
        printf("Erro SOIL: %s \n", SOIL_last_result());
    }
    else 
        return idTextura;
}

void loadModel(char nome[]){
    FILE* file = fopen(nome,"r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        exit(1);
    }
    char line[256];
    char type[3];

    // Variáveis para armazenar os dados
    vertices = NULL;
    numVertices = 0;
    normais = NULL;
    numNormais = 0;
    faces = NULL;
    numFaces = 0;
    textura = NULL;
    numTexturas = 0;


    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%2s", type);

        if (strcmp(type, "v") == 0) {
            numVertices++;
            vertices = (Vertex*)realloc(vertices, numVertices * sizeof(Vertex));
            sscanf(line, "v %f %f %f", &vertices[numVertices - 1].x, &vertices[numVertices - 1].y, &vertices[numVertices - 1].z);
        }
        else if (strcmp(type, "vn") == 0) {
            numNormais++;
            normais = (Normal*)realloc(normais, numNormais * sizeof(Normal));
            sscanf(line, "vn %f %f %f", &normais[numNormais - 1].x, &normais[numNormais - 1].y, &normais[numNormais - 1].z);
        }
        else if(strcmp(type, "f") == 0){
            numFaces++;
            faces = (Face*)realloc(faces,numFaces * sizeof(Face));
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d", &faces[numFaces - 1].v1, &faces[numFaces - 1].vt1, &faces[numFaces - 1].f, &faces[numFaces - 1].v2, &faces[numFaces - 1].vt2, &faces[numFaces - 1].f, &faces[numFaces - 1].v3, &faces[numFaces - 1].vt3, &faces[numFaces - 1].f, &faces[numFaces - 1].v4, &faces[numFaces - 1].vt4, &faces[numFaces - 1].f);
        }
        else if(strcmp(type, "vt") == 0){
            numTexturas++;
            textura = (Textura*)realloc(textura, numTexturas * sizeof(Textura));
            sscanf(line, "vt %f %f", &textura[numTexturas-1].x, &textura[numTexturas-1].y);
        }
    }

    fclose(file);
}

void loadModel3(char nome[]){
    FILE* file = fopen(nome,"r");
    if (file == NULL) {
        printf("Erro ao abrir o arquivo.\n");
        exit(1);
    }
    char line[256];
    char type[3];

    // Variáveis para armazenar os dados
    vertices = NULL;
    numVertices = 0;
    normais = NULL;
    numNormais = 0;
    faces3 = NULL;
    numFaces = 0;
    textura = NULL;
    numTexturas = 0;


    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%2s", type);

        if (strcmp(type, "v") == 0) {
            numVertices++;
            vertices = (Vertex*)realloc(vertices, numVertices * sizeof(Vertex));
            sscanf(line, "v %f %f %f", &vertices[numVertices - 1].x, &vertices[numVertices - 1].y, &vertices[numVertices - 1].z);
        }
        else if (strcmp(type, "vn") == 0) {
            numNormais++;
            normais = (Normal*)realloc(normais, numNormais * sizeof(Normal));
            sscanf(line, "vn %f %f %f", &normais[numNormais - 1].x, &normais[numNormais - 1].y, &normais[numNormais - 1].z);
        }
        else if(strcmp(type, "f") == 0){
            numFaces++;
            faces3 = (Face3*)realloc(faces3,numFaces * sizeof(Face3));
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &faces3[numFaces - 1].v1, &faces3[numFaces - 1].vt1, &faces3[numFaces - 1].f, &faces3[numFaces - 1].v2, &faces3[numFaces - 1].vt2, &faces3[numFaces - 1].f, &faces3[numFaces - 1].v3, &faces3[numFaces - 1].vt3, &faces3[numFaces - 1].f);
        }
        else if(strcmp(type, "vt") == 0){
            numTexturas++;
            textura = (Textura*)realloc(textura, numTexturas * sizeof(Textura));
            sscanf(line, "vt %f %f", &textura[numTexturas-1].x, &textura[numTexturas-1].y);
        }
    }

    fclose(file);
}

void criaListaArvore(){
    
    listaArvore = glGenLists(1);
    glNewList(listaArvore, GL_COMPILE);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, madeira.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, madeira.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, madeira.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, madeira.brilhosidade);
   
        loadModel3("modelos/troncos.obj");
        glColor3f(0.68,0.33,0.09);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturatronco);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, folhas.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, folhas.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, folhas.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, folhas.brilhosidade);
        loadModel3("modelos/folhas.obj");
        glColor3f(0,0.39,0);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);

    glEndList();
}

void criaListaBancoMesa(){
    /*GLfloat matSpecular[] ={1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialf(GL_FRONT, GL_SHININESS, 128 );
    */

    listaBancoMesa = glGenLists(1);
    glNewList(listaBancoMesa, GL_COMPILE);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, madeira.ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, madeira.difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, madeira.especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, madeira.brilhosidade);
    
   
        loadModel3("modelos/banco.obj");
        glColor3f(0.87,0.72,0.52);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaBancoMesa);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);
    glEndList();
}

void criaListaChao(){
    listaChao = glGenLists(1);
    glNewList(listaChao, GL_COMPILE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, chaoPatio.ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, chaoPatio.difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, chaoPatio.especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, chaoPatio.brilhosidade);
        
        loadModel3("modelos/chao2.obj");
        glColor3f(0.7,0.7,0.7);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaGrama);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);

        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);
    glEndList();
}

void criaListaBaseMoinho(){
    
    listaBaseMoinho = glGenLists(1);
    glNewList(listaBaseMoinho, GL_COMPILE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, madeira.ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, madeira.difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, madeira.especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, madeira.brilhosidade);

        loadModel3("modelos/basemoinho.obj");
        glColor3f(1,0.87,0.67);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaBancoMesa);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);
    glEndList();
}

void criaListaPino(){
    listaPino = glGenLists(1);
    glNewList(listaPino, GL_COMPILE);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, madeira.ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, madeira.difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, madeira.especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, madeira.brilhosidade);
        loadModel3("modelos/pinomoinho.obj");
        glColor3f(0.6,0.6,0.6);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);
    glEndList();
}

void criaListaCasa(){
    listaCasa = glGenLists(1);
    glNewList(listaCasa, GL_COMPILE);
        loadModel3("modelos/chaocasa.obj");
        glColor3f(0.94,1,0.94);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaPorcelanato);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);


        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, paredes.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, paredes.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, paredes.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, paredes.brilhosidade);
        loadModel3("modelos/paredecasa.obj");
        glColor3f(0.59,0.98,0.59);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaRipado);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);


        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, paredes.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, paredes.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, paredes.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, paredes.brilhosidade);
        loadModel3("modelos/tetocasa.obj");
        glColor3f(0.59,0.98,0.59);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);


        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, paredes.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, paredes.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, paredes.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, paredes.brilhosidade);
        loadModel3("modelos/paredeinternacasa.obj");
        glColor3f(0.4,0.4,0.4);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);

        loadModel3("modelos/balcaocasa.obj");
        //glColor3f(0,0,0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaBalcao);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);


        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, cinza.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, cinza.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cinza.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cinza.brilhosidade);
        loadModel3("modelos/geladeiracasa.obj");
        glColor3f(0.75,0.75,0.75);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaGeladeira);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);


        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, cinza.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, cinza.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cinza.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cinza.brilhosidade);
        loadModel3("modelos/fogaocasa.obj");
        glColor3f(0.75,0.75,0.75);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaFogao);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);

        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, sofa.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, sofa.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sofa.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, sofa.brilhosidade);
        loadModel3("modelos/camacasa.obj");
        //glColor3f(0.75,0.75,0.75);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaCama);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);

        
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, sofa.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, sofa.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, sofa.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, sofa.brilhosidade);
        loadModel3("modelos/sofacasa.obj");
        //glColor3f(0.75,0.75,0.75);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaSofa);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);


        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, cinza.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, cinza.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cinza.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cinza.brilhosidade);
        loadModel3("modelos/tvcasa.obj");
        glColor3f(0.21,0.21,0.21);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);


        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, cinza.ambiente);
        glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, cinza.difusa);
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, cinza.especular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, cinza.brilhosidade);
        loadModel3("modelos/chuveirocasa.obj");
        //glColor3f(0.75,0.75,0.75);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, texturaChuveiro);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        glDisable(GL_TEXTURE_2D);
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        free(vertices);
        free(normais);
        free(faces);
        free(faces3);
        free(textura);
        
    glEndList();
}

void desenhaHelice(){
    //loadModel3("modelos/helicemoinho.obj");
    
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, helice.ambiente);
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, helice.difusa);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, helice.especular);
    glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, helice.brilhosidade);

    glPushMatrix();
        glTranslatef(18.9,5.80,-8.6);
        glRotatef(rotacaoHelice,-0.7888, -0.0089, 0.6146);
            glColor3f(0.5,0.5,0.5);
            glBegin(GL_TRIANGLES);
                for(int i=0; i<numFaces; i++){
                    glTexCoord2f(textura[faces3[i].vt1-1].x, textura[faces3[i].vt1-1].y);
                    glVertex3f(vertices[faces3[i].v1 -1].x, vertices[faces3[i].v1 -1].y, vertices[faces3[i].v1 -1].z);
                    glTexCoord2f(textura[faces3[i].vt2-1].x, textura[faces3[i].vt2-1].y);
                    glVertex3f(vertices[faces3[i].v2 -1].x, vertices[faces3[i].v2 -1].y, vertices[faces3[i].v2 -1].z);
                    glTexCoord2f(textura[faces3[i].vt3-1].x, textura[faces3[i].vt3-1].y);
                    glVertex3f(vertices[faces3[i].v3 -1].x, vertices[faces3[i].v3 -1].y, vertices[faces3[i].v3 -1].z);
                }
            glEnd();
        for(int i=0; i<numNormais;i++){
            glNormal3f(normais[i].x, normais[i].y, normais[i].z);
        }
        //free(vertices);
        //free(normais);
        //free(faces);
        //free(faces3);
        //free(textura);
    glPopMatrix();
}

void desenhaCasa(){
    loadModel("modelos/casa.obj");
    glColor3f(0.5,0.5,0.5);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texturaCimento);
        glBegin(GL_QUADS);
            for(int i=0; i<numFaces; i++){
                glTexCoord2f(textura[faces[i].vt1-1].x, textura[faces[i].vt1-1].y);
                glVertex3f(vertices[faces[i].v1 -1].x, vertices[faces[i].v1 -1].y, vertices[faces[i].v1 -1].z);
                glTexCoord2f(textura[faces[i].vt2-1].x, textura[faces[i].vt2-1].y);
                glVertex3f(vertices[faces[i].v2 -1].x, vertices[faces[i].v2 -1].y, vertices[faces[i].v2 -1].z);
                glTexCoord2f(textura[faces[i].vt3-1].x, textura[faces[i].vt3-1].y);
                glVertex3f(vertices[faces[i].v3 -1].x, vertices[faces[i].v3 -1].y, vertices[faces[i].v3 -1].z);
                glTexCoord2f(textura[faces[i].vt4-1].x, textura[faces[i].vt4-1].y);
                glVertex3f(vertices[faces[i].v4 -1].x, vertices[faces[i].v4 -1].y, vertices[faces[i].v4 -1].z);
            }
        glEnd();
    glDisable(GL_TEXTURE_2D);
    for(int i=0; i<numNormais;i++){
        glNormal3f(normais[i].x, normais[i].y, normais[i].z);
    }
    free(vertices);
    free(normais);
    free(faces);
    free(textura);
}

void desenhaCena() {

    glLoadIdentity();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, 1.0, 0.1, 100.0);  // Definir a projeção perspectiva

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Aplique as transformações da câmera
    gluLookAt(cameraX, cameraY, cameraPosZ, cameraX, cameraY, cameraZ, 0.0, 1.5, 0.0); 
    glTranslatef(cameraX, cameraY, cameraPosZ); // Translação para o ponto de rotação
    glRotatef(angle, 0.0, 1.5, 0.0); // Realiza a rotação em torno do eixo Y
    glTranslatef(-cameraX, -cameraY, -cameraPosZ); /// Translação de volta para a posição original

    glFogi(GL_FOG_MODE, GL_EXP);        // Linear, exp. ou exp²
    glFogfv(GL_FOG_COLOR, corfog);         // Cor
    glFogf(GL_FOG_DENSITY, densidadeFog);      // Densidade
    glHint(GL_FOG_HINT, GL_DONT_CARE);  // Não aplicar se não puder
    glFogf(GL_FOG_START, 1.0f);         // Profundidade inicial
    glFogf(GL_FOG_END, 5.0f);           // Profundidade final
    
    
    glCallList(listaChao);
    glCallList(listaCasa);
    glCallList(listaBaseMoinho);
    glCallList(listaPino);
    desenhaHelice();
    
    
    glCallList(listaArvore);
    glCallList(listaBancoMesa);


    glutSwapBuffers();
}


void rotHelice(int time){
    if(rotacaoHelice<=360){
        rotacaoHelice++;
    }else{
        rotacaoHelice=0;
    }
    glutPostRedisplay();
    glutTimerFunc(time, rotHelice, time);
}

void inicializa(){
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_FOG);     // Liga GL_FOG

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 1.0);
    glClearColor(0.27,0.51,0.71,1);
    numVertices=0;
    numNormais=0;
    rotacaoHelice=0;
    densidadeFog=0.015f;
    texturaCimento = carregaTextura("imagens/cimento.png");
    texturatronco = carregaTextura("imagens/tronco.png");
    texturaBancoMesa = carregaTextura("imagens/madeira.png");
    texturaGrama = carregaTextura("imagens/cascalho.png");
    texturaPorcelanato = carregaTextura("imagens/porcelanato.jpg");
    texturaRipado = carregaTextura("imagens/ripado.jpg");
    texturaBalcao = carregaTextura("imagens/granito.jpg");
    texturaGeladeira = carregaTextura("imagens/geladeira.png");
    texturaFogao = carregaTextura("imagens/fog.png");
    texturaCama = carregaTextura("imagens/cama.png");
    texturaSofa = carregaTextura("imagens/sofa.jpg");
    texturaChuveiro = carregaTextura("imagens/chuveiro.png");
    
    criaListaArvore();
    criaListaBancoMesa();
    criaListaChao();
    criaListaBaseMoinho();
    criaListaPino();
    criaListaCasa();
    loadModel3("modelos/helicemoinho.obj");

    glLightfv(GL_LIGHT0, GL_POSITION, sol.posicao);
    glLightfv(GL_LIGHT0, GL_AMBIENT, sol.ambiente);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, sol.difusa);
    glLightfv(GL_LIGHT0, GL_SPECULAR, sol.especular);
    //glLightiv(GL_LIGHT0, GL_SPOT_DIRECTION, sol.direcao);
    glLightf ( GL_LIGHT0, GL_SPOT_EXPONENT , 0.0);
    glLightf ( GL_LIGHT0, GL_SPOT_CUTOFF, 70.0);

    glLightfv(GL_LIGHT1, GL_POSITION, sala.posicao);
    glLightfv(GL_LIGHT1, GL_AMBIENT, sala.ambiente);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, sala.difusa);
    glLightfv(GL_LIGHT1, GL_SPECULAR, sala.especular);
    //glLightiv(GL_LIGHT1, GL_SPOT_DIRECTION, sala.direcao);
    glLightf ( GL_LIGHT1, GL_SPOT_EXPONENT , 0.0);
    glLightf ( GL_LIGHT1, GL_SPOT_CUTOFF, 70.0);

    glLightfv(GL_LIGHT2, GL_POSITION, cozinha.posicao);
    glLightfv(GL_LIGHT2, GL_AMBIENT, cozinha.ambiente);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, cozinha.difusa);
    glLightfv(GL_LIGHT2, GL_SPECULAR, cozinha.especular);
    //glLightiv(GL_LIGHT2, GL_SPOT_DIRECTION, cozinha.direcao);
    glLightf ( GL_LIGHT2, GL_SPOT_EXPONENT , 0.0);
    glLightf ( GL_LIGHT2, GL_SPOT_CUTOFF, 70.0);
    
    glLightfv(GL_LIGHT3, GL_POSITION, quarto.posicao);
    glLightfv(GL_LIGHT3, GL_AMBIENT, quarto.ambiente);
    glLightfv(GL_LIGHT3, GL_DIFFUSE, quarto.difusa);
    glLightfv(GL_LIGHT3, GL_SPECULAR, quarto.especular);
    //glLightiv(GL_LIGHT3, GL_SPOT_DIRECTION, quarto.direcao);
    glLightf ( GL_LIGHT3, GL_SPOT_EXPONENT , 0.0);
    glLightf ( GL_LIGHT3, GL_SPOT_CUTOFF, 70.0);
    
    glLightfv(GL_LIGHT4, GL_POSITION, banheiro.posicao);
    glLightfv(GL_LIGHT4, GL_AMBIENT, banheiro.ambiente);
    glLightfv(GL_LIGHT4, GL_DIFFUSE, banheiro.difusa);
    glLightfv(GL_LIGHT4, GL_SPECULAR, banheiro.especular);
    //glLightiv(GL_LIGHT4, GL_SPOT_DIRECTION, banheiro.direcao);
    glLightf ( GL_LIGHT4, GL_SPOT_EXPONENT , 0.0);
    glLightf ( GL_LIGHT4, GL_SPOT_CUTOFF, 70.0);

}

void teclado(unsigned char key, int x, int y) {
    float cameraSpeed = 0.5; // Velocidade de movimento da câmera

   
    switch (key) {
     case 'a': // Movimento para a esquerda
            cameraX -= cameraSpeed;
            cameraPosZ-=cameraSpeed;
            break;
        case 'd': // Movimento para a direita
            cameraX += cameraSpeed;
            cameraPosZ+=cameraSpeed;
            break;
        case 'w':
            // Movimento para frente na direção da câmera
            cameraX += cameraSpeed * sin(angle * M_PI / 180.0);
            cameraZ -= cameraSpeed * cos(angle * M_PI / 180.0);
            cameraPosZ -= cameraSpeed * cos(angle * M_PI / 180.0);
            break;
        case 's':
            // Movimento para trás na direção oposta à câmera
            cameraX -= cameraSpeed * sin(angle * M_PI / 180.0);
            cameraZ += cameraSpeed * cos(angle * M_PI / 180.0);
            cameraPosZ += cameraSpeed * cos(angle * M_PI / 180.0);
            break;
        case 'g': // Zoom in
            zoom += zoomSpeed;
            break;
        case 'h': // Zoom out
            zoom -= zoomSpeed;
            if (zoom < 0.1) // Limite mínimo de zoom
                zoom = 0.1;
            break;
        case'o':
            if(densidadeFog<0.3f){
            densidadeFog+=0.001f;
            }
            break;
        case'p':
            if(densidadeFog>0){
            densidadeFog-=0.001f;
            }
            break;
        case 27:
            exit(0);
            free(vertices);
            free(normais);
            free(faces);
            free(faces3);
            free(textura);
        break;
        case '1':
            if(acesa_sala){
                acesa_sala = 0;
                glDisable(GL_LIGHT1);
            }else{
                acesa_sala = 1;
                glEnable(GL_LIGHT1);
            }
        break;
        case '2':
            if(acesa_cozinha){
                acesa_cozinha = 0;
                glDisable(GL_LIGHT2);
            }else{
                acesa_cozinha = 1;
                glEnable(GL_LIGHT2);
            }
        break;
        case '3':
            if(acesa_quarto){
                acesa_quarto = 0;
                glDisable(GL_LIGHT3);
            }else{
                acesa_quarto = 1;
                glEnable(GL_LIGHT3);
            }
        break;
        case '4':
            if(acesa_banheiro){
                acesa_banheiro = 0;
                glDisable(GL_LIGHT4);
            }else{
                acesa_banheiro = 1;
                glEnable(GL_LIGHT4);
            }
        break;
        case '5':
            if(acesa){
                acesa = 0;
                glDisable(GL_LIGHT0);
            }else{
                acesa = 1;
                glEnable(GL_LIGHT0);
            }
        break;
        case '+':
            if(acesa==1 && coeficienteLuz<1){
                coeficienteLuz+=0.1f;
                /*for(int i=0; i<3;i++){
                    sol.ambiente[i]=coeficienteLuz;
                }
                for(int i=0; i<3;i++){
                    sol.difusa[i]=coeficienteLuz;
                }
                for(int i=0; i<3;i++){
                    sol.direcao[i]=coeficienteLuz;
                }
                for(int i=0; i<3;i++){
                    sol.especular[i]=coeficienteLuz;
                } */  
            }
        break;
        case '-':
            if( coeficienteLuz>0){
                coeficienteLuz = coeficienteLuz - 0.1;
                /*for(int i=0; i<3;i++){
                    sol.ambiente[i]=coeficienteLuz;
                }
                for(int i=0; i<3;i++){
                    sol.difusa[i]=coeficienteLuz;
                }
                for(int i=0; i<3;i++){
                    sol.direcao[i]=coeficienteLuz;
                }
                for(int i=0; i<3;i++){
                    sol.especular[i]=coeficienteLuz;
                } */ 
            }
        break;
    }

    glutPostRedisplay();
}


void atualizaCena(int value) {
   for(int i=0; i<3;i++){
        sol.ambiente[i]=coeficienteLuz;
    }
    for(int i=0; i<3;i++){
        sol.difusa[i]=coeficienteLuz;
    }
    for(int i=0; i<3;i++){
        sol.direcao[i]=coeficienteLuz;
    }
    for(int i=0; i<3;i++){
        sol.especular[i]=coeficienteLuz;
    }
}
void mouse(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        lastMouseX = x;
        lastMouseY = y;
    }
}
void mouseMovimento(int x, int y) {
    float mouseSpeed = 0.1; // Velocidade de rotação da câmera

    int deltaX = x - lastMouseX;
    int deltaY = y - lastMouseY;

    angle -= deltaX * mouseSpeed;
    angle = fmod(angle, 360.0); // Utiliza a função fmod() para manter o ângulo dentro do intervalo de 0 a 360 graus

    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();
}

void redimensionada(int width, int height) {
    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    float aspectRatio = (float)width / height;
    gluPerspective(45.0, aspectRatio, 0.1, 100.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Cubo");
    glutDisplayFunc(desenhaCena);
    glutReshapeFunc(redimensionada);
    glutIdleFunc(atualizaCena);
    glutKeyboardFunc(teclado);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMovimento);
    glutTimerFunc(0,rotHelice,16);
    inicializa();
    glutMainLoop();
    return 0;
}

