// Copyright 2018 Simionescu Ana-Maria
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

typedef struct {
    int high, glove, elf;
} map;

typedef struct {
    char *name;
    int x, y, hp, stam, dmg, alive, elim;
} elf;

// Functia afiseaza cine este castigatorul, singurul elf in viata

void check_winner(FILE *out, int P, elf* E, int *win) {
    int i;
    for (i = 0; i < P; i++) {
        if (E[i].alive == 1) {
            fprintf(out, "%s has won.\n", E[i].name);
            *win = 1;
            break;
        }
    }
}

// Functia verifica daca elful se afla pe ghetar, raportand coordonatele sale
// la centrul ghetarului

int is_on_the_glacier(int R, elf* E, int id) {
    if (!(sqrt((R - E[id].x) * (R - E[id].x) +
                    (R - E[id].y) * (R - E[id].y)) <= R)) {
        return 0;
    }
    if (E[id].x < 0 || E[id].x > 2 * R) {
        return 0;
    }
    if (E[id].y < 0 || E[id].y > 2 * R) {
        return 0;
    }
    return 1;
}

// Functia afiseaza scoreboard-ul

void scoreboard(FILE *out, elf *E, int P) {
    int *aux, i, ok = 1, auxi;
    aux = calloc(P, sizeof(int));
    for (i = 0; i < P; i++) {
        aux[i] = i;
    }
    do {
        ok = 1;
        for (i = 0; i < P - 1; i++) {
            if (E[aux[i]].alive < E[aux[i + 1]].alive) {
                auxi = aux[i];
                aux[i] = aux[i + 1];
                aux[i + 1] = auxi;
                ok = 0;
            } else {
                if (E[aux[i]].elim < E[aux[i + 1]].elim &&
                        E[aux[i]].alive == E[aux[i + 1]].alive) {
                    auxi = aux[i];
                    aux[i] = aux[i + 1];
                    aux[i + 1] = auxi;
                    ok = 0;
                } else {
                    if (strcmp(E[aux[i]].name, E[aux[i + 1]].name) > 0 &&
                            E[aux[i]].elim == E[aux[i + 1]].elim &&
                            E[aux[i]].alive == E[aux[i + 1]].alive) {
                        auxi = aux[i];
                        aux[i] = aux[i + 1];
                        aux[i + 1] = auxi;
                        ok = 0;
                    }
                }
            }
        }
    } while (ok == 0);
    fprintf(out, "SCOREBOARD:\n");
    for (i = 0; i < P; i++) {
        fprintf(out, "%s\t", E[aux[i]].name);
        if (E[aux[i]].alive == 1) {
            fprintf(out, "DRY\t");
        } else {
            fprintf(out, "WET\t");
        }
        fprintf(out, "%d\n", E[aux[i]].elim);
    }
    free(aux);
}

// Functia simuleaza lupta dintre cei doi elfi, modificand hp-ul fiecaruia
// si returneaza id-ul elfului castigator

int combat(int P, FILE *out, elf* E, int id1,
                        int id2, int *pnralive, int *win) {
    int aux, ok = 1;
    if (E[id2].stam > E[id1].stam) {
        aux = id1;
        id1 = id2;
        id2 = aux;
    }
    while (ok) {
        E[id2].hp -= E[id1].dmg;
        if (E[id2].hp <= 0) {
            E[id2].alive = 0;
            *pnralive = *pnralive - 1;
            E[id1].elim++;
            ok = 0;
            fprintf(out, "%s sent %s back home.\n", E[id1].name, E[id2].name);
            E[id1].stam += E[id2].stam;
            if (*pnralive == 1) {
                    check_winner(out, P, E, win);
            }
            return id1;
        } else {
            aux = id1;
            id1 = id2;
            id2 = aux;
        }
    }
    return 0;
}

// Functia verifica daca elful poate face mutarea si daca aceasta determina
// parasirea ghetarului, dar si echiparea cu o noua pereche de manusi

void move(FILE *in, FILE *out, int P, int *R, map** G,
                    elf* E, int *pnralive, int *win) {
    int id, a, b, aux, dira, dirb, idwin;
    char dir;
    fscanf(in, "%d ", &id);
    fscanf(in, "%c", &dir);
    while (dir != '\n') {
        if (dir == 'U') {
            E[id].x--;
            dira = 1;
            dirb = 0;
        }
        if (dir == 'D') {
            E[id].x++;
            dira = -1;
            dirb = 0;
        }
        if (dir == 'L') {
            E[id].y--;
            dira = 0;
            dirb = 1;
        }
        if (dir == 'R') {
            E[id].y++;
            dira = 0;
            dirb = -1;
        }
        a = E[id].x;
        b = E[id].y;
        if (is_on_the_glacier(*R, E, id) == 0 && E[id].alive == 1 &&
                E[id].stam >= abs(G[a][b].high - G[a + dira][b + dirb].high)) {
            fprintf(out, "%s fell off the glacier.\n", E[id].name);
            E[id].alive = 0;
            *pnralive = *pnralive - 1;
            G[a + dira][b + dirb].elf = -1;
            if (*pnralive == 1) {
                check_winner(out, P, E, win);
            }
        }
        if (E[id].alive == 1) {
            if (E[id].stam < abs(G[a][b].high - G[a + dira][b + dirb].high)) {
                E[id].x += dira;
                E[id].y += dirb;
            } else {
                E[id].stam -= abs(G[a][b].high - G[a + dira][b + dirb].high);
                if (E[id].dmg < G[a][b].glove) {
                    aux = E[id].dmg;
                    E[id].dmg = G[a][b].glove;
                    G[a][b].glove = aux;
                }
                if (G[a][b].elf != -1) {
                    idwin = combat(P, out, E, id, G[a][b].elf,
                                    pnralive, win);
                    G[a][b].elf = idwin;
                    G[a + dira][b + dirb].elf = -1;
                } else {
                    G[a][b].elf = id;
                    G[a + dira][b + dirb].elf = -1;
                }
            }
        }
        fscanf(in, "%c", &dir);
    }
}

// Functia calculeaza coordonatele, raza si damage-ul furtunii, scade hp-ul
// elfilor aflati in raza de actiune si determia elfii eliminati de aceasta

void snowstorm(FILE *in, FILE *out, int P, int *R, map** G,
                    elf* E, int *pnralive, int *win) {
    int k, x, y, r, dmg, i, j, id;
    fscanf(in, "%d", &k);
    x = k & 255; k = k >> 8;
    y = k & 255; k = k >> 8;
    r = k & 255; k = k >> 8;
    dmg = k & 255;
    for (i = x - r; i <= x + r; i++) {
        for (j = y - r + x - i; j <= y + r - x + i; j++) {
            if (i >= 0 && i < 2* *R+1 && j >= 0 && j < 2* *R+1) {
                if (G[i][j].elf != -1) {
                    id = G[i][j].elf;
                    E[id].hp -= dmg;
                    if (E[id].hp < 0) {
                        E[id].alive = 0;
                        *pnralive = *pnralive - 1;
                        G[i][j].elf = -1;
                        fprintf(out, "%s was hit by snowstorm.\n", E[id].name);
                        if (*pnralive == 1) {
                            check_winner(out, P, E, win);
                        }
                    }
                }
            }
        }
    }
}

// Functia realizeaza realocarea ghetarului si verifica elfii care au parasit
// ghetarul

map** meltdown(FILE *in, FILE *out, int P, int *R, map** G,
                    elf* E, int *pnralive, int *win) {
    int stamina, i, j, aux, ok, *list, k = -1;
    list = (int *) malloc (P * sizeof(int));
    fscanf(in, "%d", &stamina);
    for (i = 0; i < 2* *R + 1; i++) {
        if (G[i][0].elf != -1) {
            E[G[i][0].elf].alive = 0;
            *pnralive = *pnralive - 1;
            list[++k] = G[i][0].elf;
            G[i][0].elf = -1;
        }
        if (G[i][2* *R].elf != -1) {
            E[G[i][2* *R].elf].alive = 0;
            *pnralive = *pnralive - 1;
            list[++k] = G[i][2* *R].elf;
            G[i][2* *R].elf = -1;
        }
        if (G[0][i].elf != -1) {
            E[G[0][i].elf].alive = 0;
            *pnralive = *pnralive - 1;
            list[++k] = G[0][i].elf;
            G[0][i].elf = -1;
        }
        if (G[2* *R ][i].elf != -1) {
            E[G[2* *R ][i].elf].alive = 0;
            *pnralive = *pnralive - 1;
            list[++k] = G[2* *R + 1][i].elf;
            G[2* *R][i].elf = -1;
        }
    }
    for (i = 1; i < 2* *R; i++) {
        for (j = 1; j < 2* *R; j++) {
            G[i-1][j-1].high = G[i][j].high;
            G[i-1][j-1].glove = G[i][j].glove;
            G[i-1][j-1].elf = G[i][j].elf;
            if (G[i-1][j-1].elf != -1) {
                if (sqrt((*R-i)*(*R-i) + (*R-j)*(*R-j)) <= *R-1) {
                    E[G[i-1][j-1].elf].stam += stamina;
                } else {
                    E[G[i-1][j-1].elf].alive = 0;
                    *pnralive = *pnralive - 1;
                    list[++k] = G[i-1][j-1].elf;
                    G[i-1][j-1].elf = -1;
                }
            }
        }
    }
    do {
        ok = 1;
        for (i = 0; i < k; i++) {
            if (list[i] > list[i+1]) {
                aux = list[i];
                list[i] = list[i+1];
                list[i+1] = aux;
                ok = 0;
            }
        }
    } while (ok == 0);
    for (i = 0; i < k + 1; i++) {
        fprintf(out, "%s got wet because of global warming.\n",
                        E[list[i]].name);
    }
    if (*pnralive == 1) {
                        check_winner(out, P, E, win);
                    }
    for (i = 0; i < P; i++) {
        E[i].x--;
        E[i].y--;
    }
    *R = *R - 1;
    free(list);
    return G;
}

// Functia citeste comenzile si apeleaza functiile corespunzatoare

map** read_comm(FILE *in, FILE *out, int P,
                        int *R, map** G, elf* E, int *pnralive, int *win) {
    char *comm;
    comm = (char *) calloc(20 , sizeof(char));
    fscanf(in, "%s", comm);
    if (strcmp(comm, "MOVE") == 0) {
        move(in, out, P, R, G, E, pnralive, win);
    }
    if (strcmp(comm, "SNOWSTORM") == 0) {
        snowstorm(in, out, P, R, G, E, pnralive, win);
    }
    if (strcmp(comm, "MELTDOWN") == 0) {
        G = meltdown(in, out, P, R, G, E, pnralive, win);
    }
    if (strcmp(comm, "PRINT_SCOREBOARD") == 0) {
        scoreboard(out, E, P);
    }
    free(comm);
    return G;
}

int main() {
    int P, R, i, *pR, nralive, *pnralive, Rinit, x, y, j;

    FILE *in = fopen("snowfight.in", "r");
    FILE *out = fopen("snowfight.out", "w");
    fscanf(in, "%d%d", &R, &P);
    pR = &R;
    nralive = P;
    pnralive = &nralive;
    Rinit = R;
    // Alocarea hartii si ghetarului
    map **G;
    G = (map **) malloc((2 * R + 1) * sizeof(map));
    for (i = 0; i < 2*R + 1; i++) {
        G[i] = (map *) malloc((2 * R + 1) * sizeof(map));
    }
     for (i = 0; i < 2*R + 1; i++) {
        for (j = 0; j < 2*R + 1; j ++) {
            fscanf(in, "%d%d", &G[i][j].high, &G[i][j].glove);
            G[i][j].elf = -1;
        }
    }
    int win, *pwin;
    win = 0;
    pwin = &win;
    // Alocare vectorului pentru elfi
    elf *E;
    E = (elf *) malloc(P * sizeof(elf));
    for (i = 0; i < P; i++) {
        E[i].name = (char *) malloc(20 * sizeof(char));
        E[i].alive = 1;
        E[i].elim = 0;
        fscanf(in, "%s", E[i].name);
        fscanf(in, "%d%d%d%d", &E[i].x, &E[i].y, &E[i].hp, &E[i].stam);
        x = E[i].x;
        y = E[i].y;
        G[x][y].elf = i;
        if (!(sqrt((R-x)*(R-x) + (R-y)*(R-y)) <= R)) {
            fprintf(out, "%s has missed the glacier.\n", E[i].name);
            E[i].alive = 0;
            G[x][y].elf = -1;
            *pnralive = *pnralive - 1;
            if (*pnralive == 1 && i == P-1) {
                check_winner(out, P, E, pwin);
            }
        }
        if (*pnralive == 1 && i == P-1) {
            check_winner(out, P, E, pwin);
        }
        E[i].dmg = G[E[i].x][E[i].y].glove;
        G[E[i].x][E[i].y].glove = 0;
    }
    while (win == 0 && !feof(in)) {
        G = read_comm(in, out, P, pR, G, E, pnralive, pwin);
    }
    for (i = 0; i < 2*Rinit + 1; i++) {
        free(G[i]);
    }
    free(G);
    for (i = 0; i < P; i++) {
        free(E[i].name);
    }
    free(E);

    fclose(in);
    fclose(out);
    return 0;
}
