/*Create search index and then input terms to perform search*/
#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>

#define HASHSIZE 101

typedef struct _Document Document;
typedef struct _WordList WordList;
typedef struct _Position Position;

struct _Position {			/* Hold positions of words in file */
	long pos;
	int flag;
	Position *next;
};

struct _Document {			/* Hold list of Documents */
	char *docname;
	int count;
	Position *wordposition;
	Document *next;
};

struct _WordList {			/* Hold list of words */
	char   *name;
	WordList *next;
	Document *doc;

};

WordList* wordTable[HASHSIZE];
int main(int argc, char * argv[]) 
{
	char *dirName;
	char key[256];

	void performDirectoryScan(char *dirName);
	void createSearchIndex(char *dirName, char *fileName);
	void initWordTable();
	unsigned int hash(char *word);
	WordList* lookup(char *word);
	char* getStringDuplicate(char *words);
	char* getNextWord(char *key);
	Document* getNewDocument(TCHAR *fileName);
	Document* getDocumentList(char *word);
	void insertToWordTable(char* name, TCHAR *filename);
	void insertWordPosition(Document* docName, long wordpos);
	void findWordPosition(Position* fposList, Position *tposList);
	void processDocumentList(Document *fdocList, char *word);
	void printDocumentList(Document *docList);
	void doSearch(char *key);
	void displayTable();
	void cleanup();
	dirName = (char *)malloc((strlen(argv[1]) + 1));
	strcpy(dirName, argv[1]);
	strcat(dirName, "*.*");
	initWordTable();
	performDirectoryScan(dirName);
	displayTable();
	printf("\n Enter Search term :");
	scanf("%s", key);
	doSearch(key);
	cleanup(wordTable);
	getch();
	return 0;
}

void cleanup() {			/* free hash table entries */
	int i;
	WordList *wl, *temp;
	for (i = 0; i<HASHSIZE; i++) {
		if (wordTable[i] != NULL) {
			wl = wordTable[i];
			while (wl != NULL) {
				temp = wl->next;
				free(wl);
				wl = temp;
			}
		}
	}
}

void initWordTable() {			/* Initialising the word table */
	int i;
	for (i = 0;i<HASHSIZE;i++)
		wordTable[i] = NULL;
}

unsigned int hash(char words[]) {			/* return hash value */
	unsigned int h = 0;
	int i;
	for (i = 0; words[i] != NULL; i++)
		h = words[i] + h * 31;
	return h%HASHSIZE;
}

char* getStringDuplicate(char *words) {
	int l = strlen(words) + 1;
	char *dupStr = (char*)malloc(l * sizeof(char));
	strcpy(dupStr, words);
	if (dupStr == NULL)
		return NULL;
	else
		return dupStr;
}

void performDirectoryScan(char *dirName)			/* Scanning all files from Directory */
{
	
	WIN32_FIND_DATA info;
	int i = 0;
	HANDLE h = FindFirstFile(dirName, &info);
	if (h == INVALID_HANDLE_VALUE)
	{
		return;
	}
	do
	{
		if (strcmp(info.cFileName, ".") != 0 && strcmp(info.cFileName, "..") != 0)
		{
			createSearchIndex(dirName, info.cFileName, h);
		}
	} while (FindNextFile(h, &info));

	FindClose(h);
}

void createSearchIndex(char *dirName, char *fileName, HANDLE h) {			/* Creating Indexer using words scanned */
	WordList *wl;
	FILE *inputFile;
	char ch, *words;
	int i = 0, j = 0;
	int length = 0;
	long wordpos;
	wl = (WordList *)malloc(sizeof(WordList));
	words = (char *)malloc((GetFileSize(h, fileName)) * sizeof(char));
	strcat(dirName, fileName);
	inputFile = fopen(dirName, "r");
	if (inputFile == NULL) {
		printf("File does not exist");
		return;
	}
	ch = fgetc(inputFile);
	while (ch != EOF) {
		wordpos = ftell(inputFile);
		*(words + i) = ch;
		if (ch == ' ') {
			*(words + i) = '\0';
			wordpos = (wordpos - strlen(words)) + 1;
			insertToWordTable(wordTable, &words, wordpos, fileName);
			i = 0;
			j++;
			length++;
		}
		ch = fgetc(inputFile);
	}
	insertToWordTable(&words, fileName);
	fclose(inputFile);
}

WordList* lookup(char *words) {			/* Look up routine to scan words that are hashed */
	unsigned int hi = hash(words);
	WordList *wl = wordTable[hi];
	for (; wl != NULL; wl = wl->next) {
		if (!strcmp(wl->name, words))
			return wl;
	}

	wl = (WordList*)malloc(sizeof(WordList));
	if (wl == NULL) {
		printf("\n Cannot allocate memory for wordlist \n");
		return;
	}

	wl->name = getStringDuplicate(words);
	wl->doc = NULL;
	wl->next = wordTable[hi];
	wordTable[hi] = wl;
	return wl;
}

void insertWordPosition(Document* docName, long wordpos) {			/* Returns new position of given word */
	Position *curpos, *newpos;
	newpos = (Position *)malloc(sizeof(Position));
	newpos->pos = wordpos;
	newpos->flag = 0;
	newpos->next = NULL;
	curpos = docName->wordposition;
	while (curpos->next != NULL) {
		curpos = curpos->next;
	}
	curpos->next = newpos;

}

Document* getNewDocument(char *fileName) {			/* returns a new document */
	char *str;
	Document *newdoc;
	newdoc = (Document *)malloc(sizeof(Document));
	newdoc->count = 0;
	newdoc->docname = (char *)malloc(sizeof(char));
	strcpy(newdoc->docname, fileName);
	newdoc->count++;
	newdoc->wordposition = NULL;
	newdoc->next = NULL;
	return newdoc;
}

void insertToWordTable(char **words, long wordpos, char *fileName) {			/* Insert the term into hashtable based on hashvalue */
	unsigned int hi;
	WordList* wl;
	char *str;
	Position *temppos, *newpos;
	Document *tempdoc, *newdoc, *prevdoc, *curdoc;
	wl = lookup(*words, wordTable);
	curdoc = wl->doc;
	prevdoc = curdoc;
	temppos = curdoc->wordposition;
	while (curdoc != NULL) {
		if (wcscmp(curdoc->docname, fileName) == NULL) {
			curdoc->count++;
			insertWordPosition(curdoc, wordpos);
			return;
		}
		prevdoc = curdoc;
		curdoc = curdoc->next;
	}
	newdoc = getNewDocument(fileName);
	if (wl->doc == NULL) {
		wl->doc = newdoc;
		return;
	}
	prevdoc->next = newdoc;
	insertWordPosition(newdoc, wordpos);
	return;

}

void displayTable() {			/* Display Contents of the table */
	int i;
	WordList *wl;
	Document *tempdoc;
	printf("\nWords In the Hash Table are:\n");

	for (i = 0; i<HASHSIZE; i++) {

		wl = wordTable[i];
		for (; wl != NULL; wl = wl->next) {
			printf("\n \nWord :%s \n", wl->name);
			for (tempdoc = wl->doc; tempdoc != NULL; tempdoc = tempdoc->next) {
				printf("\nDocument name is: %s", tempdoc->docname);
				printf("\n Count : %d", tempdoc->count);
			}
		}
	}
}

char* getNextWord(char *key) {			/* get all the words from input key */
	char *words;
	static int i = 0;
	int j;
	words = (char *)malloc(sizeof(char));
	for (j = 0; *(key + i) != NULL; i++, j++) {
		*(words + j) = *(key + i);
		if (*(key + i) == ' ') {
			*(words + j) = '\0';
			i++;
			return words;
		}
	}
	*(words + j) = '\0';
	return words;
}

Document* getDocumentList(char *word) {			/* get document list for the given word */
	WordList *wl;
	wl = lookup(word, wordTable);
	if (wl != NULL)
		return wl->doc;
}

void findWordPosition(Position* fposList, Position *tposList) {			/* comparisons of positions for successful search */
	Position *firstList, *secondList;
	long pos;
	for (firstList = fposList; firstList != NULL; firstList = firstList->next) {
		secondList = tposList;
		pos = firstList->pos;
		for (; firstList->pos <= secondList->pos; secondList = secondList->next) {
			pos++;
			if (pos == secondList->pos) {
				firstList->flag = 1;
			}
		}
	}
}

void processDocumentList(Document *fdocList, char *word) {			/* process documents for position of the given input terms */
	Document *ndocList, *tdocList;
	Position *nposList, *tposList;
	long *keyposList;
	tdocList = fdocList;
	ndocList = getDocumentList(word, wordTable);
	for (; ndocList->next != NULL;ndocList = ndocList->next) {
		if (wcscmp(tdocList->docname, ndocList->docname) == 0) {
			tposList = tdocList->wordposition;
			nposList = ndocList->wordposition;
			findWordPosition(tposList, nposList);

		}
	}
}

void printDocumentList(Document *docList) {			/* print all the successful documents */
	Document *tempdoc;
	Position *posList;
	tempdoc = docList;
	for (; tempdoc != NULL; tempdoc = tempdoc->next) {
		posList = docList->wordposition;
		for (; posList != NULL; posList = posList->next) {
			if (posList->flag == 1)
				printf("\n\n%S  Pos : %d\n\n", docList->docname, posList->pos);
		}
	}
}

void doSearch(char *key) {			/* perform search based on key term(s) given */

	Document *fdocList, *tempdocList;
	int   i, j = 0, k = 0, wcount = 0, doccount = 0, pcount = 0;
	char *word;
	word = key;
	while (!*word) {
		if (*word == ' ')
			wcount++;
	}
	wcount++;
	word = getNextWord(key);
	fdocList = getDocumentList(word, wordTable);
	tempdocList = fdocList;
	if (tempdocList == NULL) {
		printf("No such term in any document\n");
		return;
	}
	for (; tempdocList->next != NULL; tempdocList = tempdocList->next) {
		for (i = 1; i<wcount; i++) {
			word = getNextWord(key);
			processDocumentList(tempdocList, word, wordTable);
		}
	}
	printDocumentList(fdocList);
}
