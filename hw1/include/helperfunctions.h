#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include<stdlib.h>
#include<stdio.h>
#include "bdd.h"

int index_counter;

int initialize_counter;

int serial_counter;

int str_compare(char *str1,char *str2);

int str_len(char *str);

int str_to_int(char *str);

int simple_hash_function(int a, int b, int c, int d);

int determine_powerof2(int a);

int recursive_from_raster(unsigned char *raster,int h, int w,int curr_h,int curr_w,int curr_x,int curr_y,int max_w,int min_levels,int recursion_counter);

void find_serial(int value,FILE *out);

void post_order(int *serial,BDD_NODE *node,FILE* out);

#endif