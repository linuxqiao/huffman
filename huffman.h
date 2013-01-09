/*
 * =====================================================================================
 *
 *       Filename:  huffman.h
 *
 *    Description:  huffman 树的定义和一些常量声明
 *
 *        Version:  1.0
 *        Created:  2012年11月12日 08时11分38秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Hurley (LiuHuan), liuhuan1992@gmail.com
 *        Company:  Class 1107 of Computer Science and Technology
 *
 * =====================================================================================
 */

#ifndef HUFFMAN_H_
#define HUFFMAN_H_

#define ELEM_SIZE  (256) 					// 叶子节点的最大值
#define BUFF_SIZE  (2 * ELEM_SIZE - 1) 		// 所有节点的最大值

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

typedef struct tagHTNode
{
	int 	weight; 	// 节点权值
	int 	parent; 	// 双亲的下标
	int 	LChild; 	// 左孩子节点的下标
	int 	RChild; 	// 右孩子节点的下标
}HTNode;

typedef struct tagHzFileInfo
{
	char 			file_name[PATH_MAX];
	unsigned int 	prev_file_size;
	unsigned int 	add_zero_num;
}HzFileInfo;

// Huffman 树
extern HTNode HuffmanTree[BUFF_SIZE+1];

// 根据给定的概率分布，构造权值数组（按照字母序，数组0号元素不用）
extern int weight[ELEM_SIZE+1];

// 根据构造的Huffman树，建立每一个字母的Huffman编码
extern char *huffman_code[ELEM_SIZE+1];

// Huffman 树构造函数
void CrtHuffmanTree(HTNode *HuffmanTree, int tree_size, int *weight, int size);

// Huffman 树中选择两个权值最小的节点
void SelectTwoMinWeight(HTNode *HuffmanTree, int size, int *node1, int *node2);

// Huffman 编码构造函数
void CrtHuffmanCode(HTNode *HuffmanTree, int tree_size, int max_code_size);

// Huffman 编码内存释放函数
void FreeHuffmanCode(int code);

// 压缩文件函数
void CompressFiles(char *input_file, char *output_file);

// 解压缩文件函数
void DecompressFiles(char *input_file, char *output_file);

// 显示压缩文件信息函数
void PrintHzFileInfo(char *input_file);

// 二进制字符串转换十进制数字
int StrToNum(char *str, int size);

// 十进制数字转换二进制字符串
void NumToStr(int num, char *str);

// 打印使用方法
void PrintUseageInfo(char *str);

#endif // HUFFMAN_H_
