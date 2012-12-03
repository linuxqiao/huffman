/*
 * =====================================================================================
 *
 *       Filename:  huffman.c
 *
 *    Description:  huffman 树的实现
 *
 *        Version:  1.0
 *        Created:  2012年11月12日 08时30分15秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "huffman.h"

//#define HUFFMAN_DEBUG

HTNode HuffmanTree[BUFF_SIZE+1];
int weight[ELEM_SIZE+1];

char *huffman_code[ELEM_SIZE+1];
char str_code_cover[ELEM_SIZE+1][50];

int main(int argc, char *argv[])
{
	clock_t start, finish;

	start = clock();

	if (argc < 3)
	{
		PrintUseageInfo(argv[0]+2);
	}
	else
	{
		if (strcmp("-c", argv[1]) == 0)
		{
			CompressFiles(argv[2], argv[3]);
			finish = clock();
			printf("程序运行时间：%.4f 秒\n", (double)(finish - start) / CLOCKS_PER_SEC);

		}
		else if (strcmp("-d", argv[1]) == 0)
		{
			DecompressFiles(argv[2], argv[3]);
			finish = clock();
			printf("程序运行时间：%.4f 秒\n", (double)(finish - start) / CLOCKS_PER_SEC);
		}
		else if (strcmp("-r", argv[1]) == 0)
		{
			PrintHzFileInfo(argv[2]);
		}
		else
		{
			PrintUseageInfo(argv[0]+2);
		}
	}

	return EXIT_SUCCESS;
}

void CompressFiles(char *input_file, char *output_file)
{
	int 		i, ch;
	int 		input_file_size, output_file_size;
	FILE 		*fp_input, *fp_tmp, *fp_output;
	char 		tmp_file[PATH_MAX];
	char 		code_cover[9];
	HzFileInfo	HzInfo;

	memset(tmp_file, 0, (sizeof(char)*PATH_MAX));
	strcpy(tmp_file, input_file);
	strcat(tmp_file, ".tmp");

	strcpy(HzInfo.file_name, input_file);
	
	if ((fp_input = fopen(input_file, "rb")) == NULL ) {
		fprintf(stderr, "Cannot open file %s !\n", input_file);
		exit(0);
	}
	
	while ((ch = fgetc(fp_input)) != EOF)
	{
		weight[ch+1]++;
	}

	input_file_size = ftell(fp_input);
	HzInfo.prev_file_size = input_file_size;
	
	CrtHuffmanTree(HuffmanTree, BUFF_SIZE, weight, ELEM_SIZE);

	// 这个编码估计最长不超过50（函数执行中会有assert检测）
	CrtHuffmanCode(HuffmanTree, ELEM_SIZE, 50);
	
	#ifdef HUFFMAN_DEBUG
	int sum = 0, total = 0;
	for (i = 1; i < ELEM_SIZE + 1; i++)
	{
		if (weight[i] != 0)
		{
			printf("%03d : %d\n", i-1, weight[i]);
			sum++;
			total += weight[i];
		}
	}
	printf("Weight Size: %d Total is %d\n", sum, total);

	printf("Huffman Tree:\n");
	for (i = 1; i < BUFF_SIZE + 1; i++)
	{
		if (HuffmanTree[i].weight != 0)
		{
			printf("%06d %06d %06d %06d\n", 
			HuffmanTree[i].weight, HuffmanTree[i].parent,
			HuffmanTree[i].LChild, HuffmanTree[i].RChild);
		}
	}
	printf("Huffman Code:\n");
	sum = 0;
	for (i = 1; i < ELEM_SIZE + 1; i++)
	{
		if (strlen(huffman_code[i]) != 0)
		{
			printf("%03d : %s\n", i-1, huffman_code[i]);
			sum++;
		}
	}
	printf("Code Size: %d\n", sum);
	#endif
 
	if ((fp_tmp = fopen(tmp_file, "wb+")) == NULL ) {
		fprintf(stderr, "Cannot open file %s !\n", tmp_file);
		exit(0);
	}

	fseek(fp_input, 0, SEEK_SET);
	while ((ch = fgetc(fp_input)) != EOF)
	{
		fprintf(fp_tmp, "%s", huffman_code[ch+1]);
	}

	fclose(fp_input);

	fseek(fp_tmp, 0, SEEK_SET);

	if ((fp_output = fopen(output_file, "wb")) == NULL ) {
		fprintf(stderr, "Cannot open file %s !\n", output_file);
		exit(0);
	}

	// 先行占位
	fwrite(&HzInfo, sizeof(HzInfo), 1, fp_output);
	
	// 向压缩文件输出字典信息
	fwrite(HuffmanTree, sizeof(HuffmanTree), 1, fp_output);


	code_cover[8] = '\0';
	while ((i = fread(code_cover, sizeof(char), 8, fp_tmp)) == 8)
	{
		code_cover[8] = '\0';
		ch = StrToNum(code_cover, 8);
		fputc(ch, fp_output);
	}
	if ( i > 0)
	{
		HzInfo.add_zero_num = i;
		while (i != 8)
		{
			code_cover[i++] = '0';
		}
		code_cover[8] = '\0';
		ch = StrToNum(code_cover, i);
		fputc(ch, fp_output);
	}

	output_file_size = ftell(fp_output);

	// 输出压缩文件头信息
	fseek(fp_output, 0, SEEK_SET);
	fwrite(&HzInfo, sizeof(HzInfo), 1, fp_output);
	
	fclose(fp_output);

	printf("压缩后的文件 %s 已生成。 ", output_file);
	printf("压缩率：%.2f %%\n", (double)output_file_size / input_file_size * 100);

	fclose(fp_tmp);
	remove(tmp_file);
	
	FreeHuffmanCode(ELEM_SIZE);
}

void DecompressFiles(char *input_file, char *output_file)
{
	int 		i, ch, old_ch, tag = 0, huffman_root;
	FILE 		*fp_input, *fp_tmp, *fp_output;
	char 		tmp_file[PATH_MAX];
	char 		code_str[9];
	HzFileInfo 	HzInfo;

	memset(tmp_file, 0, (sizeof(char)*PATH_MAX));
	strcpy(tmp_file, input_file);
	strcat(tmp_file, ".tmp");
	
	if ((fp_input = fopen(input_file, "rb")) == NULL ) {
		fprintf(stderr, "Cannot open file %s !\n", input_file);
		exit(0);
	}

	// 读取文件头信息
	fread(&HzInfo, sizeof(HzInfo), 1, fp_input);

	// 读取字典信息
	fread(HuffmanTree, sizeof(HuffmanTree), 1, fp_input);

	for (i = 1; i < BUFF_SIZE + 1; i++)
	{
		if (HuffmanTree[i].parent == 0 && HuffmanTree[i].weight != 0)
		{
			huffman_root = i;
		}
	}

	#ifdef HUFFMAN_DEBUG
	for (i = 1; i < BUFF_SIZE + 1; i++)
	{
		if (HuffmanTree[i].weight != 0)
		{
			printf("%03d : %06d %06d %06d %06d\n", i,
					HuffmanTree[i].weight, HuffmanTree[i].parent,
					HuffmanTree[i].LChild, HuffmanTree[i].RChild);
		}
	}
	#endif
 
	if ((fp_tmp = fopen(tmp_file, "wb+")) == NULL ) {
		fprintf(stderr, "Cannot open file %s !\n", tmp_file);
		exit(0);
	}

	old_ch = fgetc(fp_input);
	while ((ch = fgetc(fp_input)) != EOF)
	{
		NumToStr(old_ch, code_str);
		code_str[8] = '\0';
		fprintf(fp_tmp, "%s", code_str);
		old_ch = ch;
	}

	NumToStr(old_ch, code_str);
	code_str[HzInfo.add_zero_num] = '\0';
	fprintf(fp_tmp, "%s", code_str);

	fclose(fp_input);

	if ((fp_output = fopen(output_file, "wb")) == NULL ) {
		fprintf(stderr, "Cannot open file %s !\n", output_file);
		exit(0);
	}
	
	fseek(fp_tmp, 0, SEEK_SET);

	while (TRUE)
	{
		i = huffman_root;
		while (HuffmanTree[i].LChild != 0 && (ch = fgetc(fp_tmp)) != EOF)
		{
			tag++;
			if (ch == '1')
			{
				i = HuffmanTree[i].RChild;
			}
			else if (ch == '0')
			{
				i = HuffmanTree[i].LChild;
			}
			else
			{
				fprintf(stderr, "压缩文件中出现了不可识别的符号，程序终止！\n");
				exit(0);
			}
		}
		if (ch == EOF)
			break;
		fputc(i-1, fp_output);
	}

	fclose(fp_output);

	printf("解压缩后的文件 %s 已生成。\n", output_file);

	fclose(fp_tmp);
	remove(tmp_file);
}

void PrintHzFileInfo(char *input_file)
{
	FILE 		*fp_input;
	HzFileInfo 	HzInfo;

	if ((fp_input = fopen(input_file, "rb")) == NULL ) {
		fprintf(stderr, "Cannot open file %s !\n", input_file);
		exit(0);
	}

	// 读取文件头信息
	fread(&HzInfo, sizeof(HzInfo), 1, fp_input);

	printf("在压缩文件 %s 中：\n", input_file);
	printf("\t被压缩的原文件名：%s\n", HzInfo.file_name);
	printf("\t被压缩的文件大小: %u 字节\n", HzInfo.prev_file_size);

	fclose(fp_input);
}

void CrtHuffmanTree(HTNode *HuffmanTree, int tree_size, int *weight, int size)
{
	int i, elem_size = 0, node1 = 1, node2 = 2;

	for (i = 1; i < size + 1; i++)
	{
		HuffmanTree[i].weight = weight[i];
		if(weight[i] != 0)
		{
			elem_size++;
		}
	}

	for (i = size + 1; i < size + elem_size; i++)
	{
		SelectTwoMinWeight(HuffmanTree, i-1, &node1, &node2);
		HuffmanTree[i].weight = HuffmanTree[node1].weight + HuffmanTree[node2].weight;
		HuffmanTree[node1].parent = i;
		HuffmanTree[node2].parent = i;
		HuffmanTree[i].LChild = node1;
		HuffmanTree[i].RChild = node2;
	}
}

void SelectTwoMinWeight(HTNode *HuffmanTree, int size, int *node1, int *node2)
{
	int i;
	int weight1 = INT_MAX, weight2 = INT_MAX;
	
	for(i = 1; i < size + 1; i++)
	{
		if (HuffmanTree[i].weight != 0 && HuffmanTree[i].weight < weight2 && HuffmanTree[i].parent == 0 )
		{
			if (HuffmanTree[i].weight < weight1)
			{
				*node2 = *node1;
				*node1 = i;
				weight2 = weight1;
				weight1 = HuffmanTree[i].weight;
			}
			else
			{
				*node2 = i;
				weight2 = HuffmanTree[i].weight;
			}
		}
	}
}

void CrtHuffmanCode(HTNode *HuffmanTree, int elem_size, int max_code_size)
{
	char  	*strtmp;
	int 	i, p_tag, start, code_tmp;

	strtmp = (char *)malloc(max_code_size*sizeof(char));
	strtmp[max_code_size-1] = '\0';

	for (i = 1; i < elem_size + 1; i++)
	{
		start = max_code_size - 1;
		code_tmp = i;
		p_tag = HuffmanTree[i].parent;
		while (p_tag != 0)
		{
			start--;
			if (code_tmp == HuffmanTree[p_tag].LChild)
			{
				strtmp[start] = '0';
			}
			else
			{
				strtmp[start] = '1';
			}
			code_tmp = p_tag;
			p_tag = HuffmanTree[p_tag].parent;
		}
		assert(max_code_size-start > 0);
		huffman_code[i] = (char *)malloc((max_code_size-start)*sizeof(char));
		strcpy(huffman_code[i], strtmp + start);
	}
	free(strtmp);	
}

void FreeHuffmanCode(int size)
{
	int i;
	
	for (i = 1; i < size + 1; i++)
	{
		if (huffman_code[i] != NULL)
		{
			free(huffman_code[i]);
		}
	}
}

inline int StrToNum(char *str, int size)
{
	int sum = 0, i;

	for (i = 0; i < size; i++)
	{
		sum = sum * 2 + (*(char *)str - '0');
		str++;
	}

	return sum;
}

inline void NumToStr(int num, char *str)
{ 
	int i = 7, tmp;

	memset(str, '0', 8);
	for (tmp = num; tmp != 0; tmp /= 2) 
	{ 
		str[i--]= tmp % 2 + '0';
	}
}

inline void PrintUseageInfo(char *str)
{
	printf("\n错误：命令格式错误！\n\n用法：%s -[参数] 输入文件 输出文件\n", str);
	puts("\n程序说明：一个使用 Huffman 算法进行文件压缩的小程序，目前仅支持单文件压缩。\n");
	puts("参数：\n\t-c : 压缩文件\n\t-d : 解压缩文件\n\n\t-r : 查看压缩文件信息\n");
}
