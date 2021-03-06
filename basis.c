#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_vector.h>
#include "common.h"
#include "basis.h"
#include "overlap.h"

void* read_basis(const char * file_name)
{
// 每个原子有几个基函数需要指定说明
    FILE *f;
    char *sparate = "****";
    char *atom_orbit_type = "SP";   // SP型一行有3个参数
    char symbol[5];
    double param;
    int state = 0, i;
    int gauss_num; //  每一块gauss函数的个数
    int basis_num = 2, basis_total = 9, basis_i = 0, ib = 0;
    double tmp_alpha, tmp_coeff_1, tmp_coeff_2;     //用以临时存储从文件中读取的基函数参数信息

    // basis用于保存所有基函数
    BASIS *basis = calloc(sizeof(BASIS), basis_total);

    f = fopen(file_name, "r");
    
    while (1) {
        switch (state) {
            case 0: // initial state
                fscanf(f, "%s", symbol);
                // symbol == "****", start read atom information
                if (strcmp(symbol, sparate) == 0)   state = 1;
                //basis_i++;
                break;
            case 1: // read an atom's information
                if (fscanf(f, "%s", symbol) == EOF) // "H" read the atom symbol, initialize some parameter which i donn't know now
                    return basis;
                    //exit(EXIT_FAILURE);
                fscanf(f, "%lf", &param);     // 0

                if (strncmp(symbol, "N", 1) == 0)
                    basis_num = 5;
                else if (strncmp(symbol, "H", 1) == 0)
                    basis_num = 1;
                ib = 0;
                state = 2;  // start read basis set information
                break;
            case 2:
                if (ib >= basis_num) {
                    state = 0;
                    break;
                }else{
                    ib++;
                }

                fscanf(f, "%s", symbol);        // " "S" 3   1.00  "
                fscanf(f, "%d", &gauss_num);     // "  S "3"  1.00  " 每一块高斯函数的数目
                fscanf(f, "%lf", &param);         // "  S  3  "1.00" "    不清楚什么用途

                if (strcmp(symbol, atom_orbit_type) == 0) {
                    state = 4;  // SP 有三列数据，第一列为gaussian函数的指数，2,3列分别为2S, 2P中的组合系数
                }else{
                    state = 3;  // start read basis set data of S orbital
                }

                break;
            case 3:
                for (i = 0; i < gauss_num; i++) {
                    fscanf(f, "%lf", &tmp_alpha);
                    fscanf(f, "%lf", &tmp_coeff_1);

                    basis[basis_i].gaussian[i].alpha = tmp_alpha;
                    // 注意，此处的归一化只是针对1S轨道的 (2a/pi)^(3/4) 《量子化学》中册，P50
                    //basis[basis_i].gaussian[i].A = tmp_coeff_1 * pow(2*tmp_alpha/M_PI, (double)3.0/4);
                    basis[basis_i].gaussian[i].coeff = tmp_coeff_1;
#ifdef BEBUG_OUTPUT_BASIS_SET
                    printf("%20.10lf%20.10lf\n", basis[basis_i].gaussian[i].alpha, basis[basis_i].gaussian[i].coeff);
#endif

                }
                basis_i ++;
                ib++;
                state = 2;  //读完一组基函数信息，状态返回，读取下一组
                break;
            case 4: // 一个gaussian函数由3个参数决定的情形
                for (i = 0; i < gauss_num; i++) {
                    fscanf(f, "%lf", &tmp_alpha);
                    fscanf(f, "%lf", &tmp_coeff_1);
                    fscanf(f, "%lf", &tmp_coeff_2);
                    // 2S
                    basis[basis_i].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i].gaussian[i].coeff = tmp_coeff_1;
                    // 2P
                    // Px
                    basis[basis_i+1].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+1].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+1].gaussian[i].l = 1;
                    // Py
                    basis[basis_i+2].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+2].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+2].gaussian[i].m = 1;
                    // Pz
                    basis[basis_i+3].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+3].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+3].gaussian[i].n = 1;
                }
                basis_i += 4;
                ib += 4;
                state = 2;  //读完一组基函数信息，状态返回，读取下一组
                break;
        }
    }
    fclose(f);
    return basis;
}

// 参数 count 表示一个基函数由count个gaussian函数组成
void basis_set_output(const BASIS* b, int count, char* msg)
{
    int i;
    printf("%s\n", msg);
    for (i = 0; i < count; i++) {
        vector_output(b[i].xyz, 3, "基组坐标:");
        gto_output(b[i].gaussian, b[i].gaussCount, "基函数");
    }
}

void gto_output(const GTO* g, int count, char* msg)
{
    int i;
    double alpha, coeff, norm;
    int l, m, n;

    printf("%s\n", msg);
    for (i = 0; i < count; i++)
    {
        l = (g+i)->l;
        m = (g+i)->m;
        n = (g+i)->n;
        alpha = (g+i)->alpha;
        coeff = (g+i)->coeff;
        norm = (g+i)->norm;
        printf("%d %d %d %12.8lf %12.8lf %12.8lf\n",
                l, m, n, alpha, coeff, norm);
    }
}

void atom_output(const ATOM_INFO** atom, int n)
{
    ATOM_INFO *a=NULL;
    int i;
    for (i = 0; i < n; i++) {
        a = (ATOM_INFO*)atom[i];
        printf("%s %d %d%12.7lf%12.7lf%12.7lf\n", a->symbol, a->n, 
                    a->basisCount, a->coordination->data[0], 
                    a->coordination->data[1], a->coordination->data[2]); 
        // for (j = 0; j < a->basisCount; j++)
        //     basis_set_output(a->basis + j, 3, "Basis Function:");
    }
}

// ----------------------------------------------------------------------------
// 读取基组方法的重新实现
#define Item_count              3
#define Atom_count_max          4
#define Basis_set_count_max     3

INPUT_INFO* parse_input(const char* file_name)
{
// parse the input file, translate the input file into useable data
    FILE *f;
    INPUT_INFO *input_information;
    char Item[Item_count][8] = {"$COORD", "$BASIS", "$END"};
    char BasisSetName[Basis_set_count_max][8] = {"STO-3G", "6-31G", "6-31G*"};
    char input_item[9];
    ATOM_INFO *atom; // save the coordination of atoms
    int state = 0, i, j, Atom_index = 0;
    double tmp_coord = 0.0;
    input_information = malloc(sizeof(INPUT_INFO));
    input_information->gXYZ = (gsl_vector**)malloc(sizeof(gsl_vector*));

    f = fopen(file_name, "r");

    while (1) {
        switch (state) {
            case 0: // initial state
                if (fscanf(f, "%s", input_item) == EOF) {
                    exit(EXIT_FAILURE);
                    return NULL;
                }
                for (i = 0; i < Item_count; i++) {
                    // setup the state will go according the Item value
                    if (strcmp(input_item, Item[i]) == 0) {
                        state = i + 1;
                        break;
                    }
                }
                break;
            case 1: // read the coordination information
                fscanf(f, "%s", input_item);
                // if read the item "$END", continue the other state
                if (strcmp(input_item, Item[2]) == 0) {
                    state = 0;
                    break;
                }
                // save coordination
                input_information->atomList = (ATOM_INFO**) \
                            realloc(input_information->atomList, Atom_index+1);
                input_information->gXYZ = (gsl_vector**) \
                            realloc(input_information->gXYZ, (Atom_index + 1));
                // save atom information
                atom = input_information->atomList[Atom_index] = 
                                    calloc(sizeof(ATOM_INFO), 1);
                atom->coordination = input_information->gXYZ[Atom_index] = \
                                                        gsl_vector_alloc(3);
                // read element symbol of atom
                strcpy(atom->symbol, input_item);
                // read element core electronics of atom
                fscanf(f, "%d", &atom->n);
                input_information->eCount += atom->n;   // record the total of electron
                for (i = 0; i < 3; i++) {
                    // read coordination of atom
                    fscanf(f, "%lf", &tmp_coord);
                    gsl_vector_set(atom->coordination, i,
                                                       tmp_coord * ANGS_2_BOHR);
                }

                Atom_index++;
                break;
            case 2: // read the basis set information
                fscanf(f, "%s", input_item);
                for (i = 0; i < Basis_set_count_max; i++) {
                    if (strcmp(input_item, BasisSetName[i]) == 0)
                        break;
                }

                switch (i) {
                    case 0: // STO-3G
                        // Set parameters of atom.
                        for (j = 0; j < Atom_index; j++) {
                            atom = input_information->atomList[j];
                            switch (atom->n) {
                                case 1:     
                                    // total basis count
                                    input_information->basisCount += 1;
                                    // the basis count of atom
                                    atom->basisCount = 1;
                                    break;
                                case 2:
                                    break;
                                case 3:
                                    input_information->basisCount += 5;
                                    atom->basisCount = 5;
                                    break;
                                case 7:
                                    // total basis count
                                    input_information->basisCount += 5;
                                    // the basis count of atom
                                    atom->basisCount = 5;
                                    break;
                            }
                        }
                        break;
                    case 1: // 6-31G
                        // Set parameters of atom.
                        for (j = 0; j < Atom_index; j++) {
                            atom = input_information->atomList[j];
                            switch (atom->n) {  // according the number of atom
                                case 1:     
                                    // total basis count
                                    input_information->basisCount += 2;
                                    // the basis count of atom
                                    atom->basisCount = 2;
                                    break;
                                case 2:
                                    break;
                                case 3:
                                    input_information->basisCount += 9;
                                    atom->basisCount = 9;
                                    break;
                                case 7:
                                    // total basis count
                                    input_information->basisCount += 9;
                                    // the basis count of atom
                                    atom->basisCount = 9;
                                    break;
                            }
                        }
                        break;
                    case 2: // 6-31G*
                        // Set parameters of atom.
                        for (j = 0; j < Atom_index; j++) {
                            atom = input_information->atomList[j];
                            switch (atom->n) {  // according the number of atom
                                case 1:     
                                    // total basis count
                                    input_information->basisCount += 2;
                                    // the basis count of atom
                                    atom->basisCount = 2;
                                    break;
                                case 2:
                                    break;
                                case 3:
                                    input_information->basisCount += 15;
                                    atom->basisCount = 15;
                                    break;
                                case 7:
                                    // total basis count
                                    input_information->basisCount += 15;
                                    // the basis count of atom
                                    atom->basisCount = 15;
                                    break;
                            }
                        }
                        break;
                } //end setup basis set information

                // read basis set
                // TODO:
                //      读完基函数直接退出，要修改的更友善
                input_information->basisSet = readbasis(f, 
                                                input_information->basisCount);
                input_information->atomCount = Atom_index;
                bridge(input_information->basisSet, input_information->atomList,
                                                        Atom_index);
                return input_information;
                break;
            case 3: // block end
                break;
        }
    }
    input_information->atomCount = Atom_index;
    return input_information;
}

// link the basis set and coordination
void bridge(BASIS* b, ATOM_INFO** atomList, int atomCount)
{
    int i, j, inc = 0;

    for (i = 0; i < atomCount; i++)
    {
        for (j = 0; j < atomList[i]->basisCount; j++)
            b[inc + j].xyz = atomList[i]->coordination;
        inc += atomList[i]->basisCount;
    }
}

#define ORBITAL_TYPE_COUNT  3

// 是函数read_basis针对新的数据结构的升级版
BASIS* readbasis(FILE * f, int basisCount)
{
// read the part contain basis set in the input file
// 每个原子有几个基函数需要指定说明
    char *sparate = "****";
    char orbitalType[ORBITAL_TYPE_COUNT][3] = {"S", "SP", "D"};   // SP型一行有3个参数
    char symbol[5];
    int gauss_num; //gauss_num    每一块gauss函数的个数
    int basis_i = 0, state = 0, i;
    //用以临时存储从文件中读取的基函数参数信息
    double param, tmp_alpha, tmp_coeff_1, tmp_coeff_2;

    // basis用于保存所有基函数
    BASIS *basis = calloc(sizeof(BASIS), basisCount);

    while (1) {
        switch (state) {
            case 0: // initial state
                fscanf(f, "%s", symbol);
                // symbol == "****", start read atom information
                if (strcmp(symbol, sparate) == 0)   state = 1;
                break;
            case 1: 
            // read an atom's information
                fscanf(f, "%s", symbol); // read the atom symbol
                if (strcmp(symbol, "$END") == 0) {
                    return basis;
                    //return 0;
                }
                fscanf(f, "%lf", &param);     // 0
                // read the basis set count of every atom
                state = 2;  // start read basis set information
                break;
            case 2: // read orbital type
                // S   3   1.00
                fscanf(f, "%s", symbol);
                // if it's **** goto state 1
                if (strcmp(symbol, sparate) == 0) {
                    state = 1;
                    break;
                }
                fscanf(f, "%d", &gauss_num);
                fscanf(f, "%lf", &param);

                for (i = 0; i < ORBITAL_TYPE_COUNT; i++) {
                    if (strcmp(symbol, orbitalType[i]) == 0)
                        break;
                }

                switch (i) {
                    case 0: // S
                        // save the gaussian function count of basis
                        basis[basis_i].gaussCount += gauss_num;
                        basis[basis_i].gaussian = calloc(sizeof(GTO), gauss_num);
                        state = 3;  // start read basis set data of S orbital
                        break;
                    case 1: // SP 
                        // S
                        basis[basis_i].gaussCount += gauss_num;
                        basis[basis_i].gaussian = calloc(sizeof(GTO), gauss_num);
                        // P
                        basis[basis_i+1].gaussCount += gauss_num;
                        basis[basis_i+1].gaussian = calloc(sizeof(GTO), gauss_num);
                        basis[basis_i+2].gaussCount += gauss_num;
                        basis[basis_i+2].gaussian = calloc(sizeof(GTO), gauss_num);
                        basis[basis_i+3].gaussCount += gauss_num;
                        basis[basis_i+3].gaussian = calloc(sizeof(GTO), gauss_num);
                        state = 4;
                        break;
                    case 2: // D
                        for (i = 0; i < 6; i++) {
                            basis[basis_i+i].gaussCount += gauss_num;
                            basis[basis_i+i].gaussian = calloc(sizeof(GTO), gauss_num);
                        }
                        state = 5;
                        break;
                    }
                break;
            case 3: // 当前为读取S
                for (i = 0; i < gauss_num; i++) {
                    fscanf(f, "%lf", &tmp_alpha);
                    fscanf(f, "%lf", &tmp_coeff_1);

                    basis[basis_i].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i].gaussian[i].coeff = tmp_coeff_1;
                    basis[basis_i].gaussian[i].norm =
                                normalize_coeff(&basis[basis_i].gaussian[i]);
//#define DEBUG_OUTPUT_BASIS_SET
#ifdef DEBUG_OUTPUT_BASIS_SET
                    printf("%d %d %d %10.6lf%10.6lf%10.6lf\n",
                            basis[basis_i].gaussian[i].l,
                            basis[basis_i].gaussian[i].m,
                            basis[basis_i].gaussian[i].n,
                            basis[basis_i].gaussian[i].alpha,
                            basis[basis_i].gaussian[i].coeff,
                            basis[basis_i].gaussian[i].norm);
#endif

                }
                basis_i++;
                state = 2;  //读完一组基函数信息，状态返回，读取下一组
                break;
            case 4: // 读取SP标签：S与P轨道
                for (i = 0; i < gauss_num; i++) {
                    fscanf(f, "%lf", &tmp_alpha);
                    fscanf(f, "%lf", &tmp_coeff_1);
                    fscanf(f, "%lf", &tmp_coeff_2);
                    // 2S
                    basis[basis_i].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i].gaussian[i].coeff = tmp_coeff_1;
                    basis[basis_i].gaussian[i].norm = 
                                   normalize_coeff(&basis[basis_i].gaussian[i]);

                    basis[basis_i+1].gaussCount = basis[basis_i+2].gaussCount \
                    = basis[basis_i+3].gaussCount = basis[basis_i].gaussCount;
                    // 2P
                    // Px
                    basis[basis_i+1].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+1].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+1].gaussian[i].l = 1;
                    basis[basis_i+1].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+1].gaussian[i]);
                    // Py
                    basis[basis_i+2].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+2].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+2].gaussian[i].m = 1;
                    basis[basis_i+2].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+2].gaussian[i]);
                    // Pz
                    basis[basis_i+3].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+3].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+3].gaussian[i].n = 1;
                    basis[basis_i+3].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+3].gaussian[i]);
                }
                basis_i += 4;
                state = 2;  //读完一组基函数信息，状态返回，读取下一组
                break;
            case 5: // read D basis set information
                fscanf(f, "%lf", &tmp_alpha);
                fscanf(f, "%lf", &tmp_coeff_1);
                for (i = 0; i < gauss_num; i++) {
                    // d_{x^{2}}
                    basis[basis_i].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i].gaussian[i].l = 2;
                    basis[basis_i].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i].gaussian[i]);

                    // d_{y^{2}}
                    basis[basis_i+1].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+1].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+1].gaussian[i].m = 2;
                    basis[basis_i+1].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+1].gaussian[i]);
                    // d_{z^{2}}
                    basis[basis_i+2].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+2].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+2].gaussian[i].n = 2;
                    basis[basis_i+2].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+2].gaussian[i]);
                    // d_{xy}
                    basis[basis_i+3].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+3].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+3].gaussian[i].l = 1;
                    basis[basis_i+3].gaussian[i].m = 1;
                    basis[basis_i+3].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+3].gaussian[i]);
                    // d_{xz}
                    basis[basis_i+4].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+4].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+4].gaussian[i].l = 1;
                    basis[basis_i+4].gaussian[i].n = 1;
                    basis[basis_i+4].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+4].gaussian[i]);
                    // d_{yz}
                    basis[basis_i+5].gaussian[i].alpha = tmp_alpha;
                    basis[basis_i+5].gaussian[i].coeff = tmp_coeff_2;
                    basis[basis_i+5].gaussian[i].m = 1;
                    basis[basis_i+5].gaussian[i].n = 1;
                    basis[basis_i+5].gaussian[i].norm = 
                                normalize_coeff(&basis[basis_i+5].gaussian[i]);
                }
                basis_i += 6;
                state = 2;  //读完一组基函数信息，状态返回，读取下一组
                break;
        }
    }
    return basis;
}

double normalize_coeff(const GTO *g)
{
    double alpha = g->alpha;
    double l = g->l;
    double m = g->m;
    double n = g->n;

    return pow(2 * alpha / M_PI, 0.75) * sqrt(pow(4*alpha, l + m + n) / \
        (factorial_2(2*l-1) * factorial_2(2*m-1) * factorial_2(2*n-1)));
}

int gtoIsNeg(const GTO* g)
{
    if (g->l < 0 || g->m < 0 || g->n < 0)
        return 1;
    else
        return 0;
}
