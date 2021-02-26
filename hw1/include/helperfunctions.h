#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include<stdlib.h>
#include<stdio.h>
#include "bdd.h"

int index_counter;

int str_compare(char *str1,char *str2);

int str_len(char *str);

int str_to_int(char *str);

int simple_hash_function(int a, int b, int c, int d);

int determine_powerof2(int a);

int recursive_from_raster(unsigned char *raster,int h, int w,int curr_h,int curr_w,int curr_x,int curr_y,int min_levels,int recursion_counter);

void find_serial(int value,FILE *out);

void post_order(int *serial,BDD_NODE *node,FILE* out);

int rotate_helper(BDD_NODE *node,int level);

int map_helper(BDD_NODE *node,unsigned char (*func)(unsigned char));

int zoom_in_helper();

int zoom_out_helper();

int negate();

int threshold(int threshold);

#endif