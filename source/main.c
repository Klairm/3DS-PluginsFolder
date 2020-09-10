#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <3ds.h>
#include <dirent.h>
#include <unistd.h>

char pluginNames[50][50];
u64* titleIds;
u32 titleCount;
bool copyMenu = false;
Result r;

int pos = 1;

void arrowMenu(int realPos, int arrowPos){
	if(realPos == arrowPos){
		printf("->");
	}else{
		printf("  ");
	}

}

void drawMain(PrintConsole top, PrintConsole bottom){
	consoleSelect(&bottom);
	consoleClear();
	printf("<A> Select <START> Exit \n");
	consoleSelect(&top);
	consoleClear();
	arrowMenu(1,pos);printf("Create plugin folders for all TitleIDs\n");
	arrowMenu(2,pos);printf("Copy plugin to all TitlesID\n");
	
}

int getPluginCount();
void listPlugins(int pos,PrintConsole top,PrintConsole bottom);
void pluginFolders(char* titleID,PrintConsole top, PrintConsole bottom);
void copyPlugin(int pos, PrintConsole top, PrintConsole bottom);
int fileCopy(const char* filein,const char* fileout);


int main()
{
	gfxInitDefault();
	
	PrintConsole topScreen, bottomScreen;
	consoleInit(GFX_BOTTOM,&bottomScreen);
	consoleInit(GFX_TOP,&topScreen);
	
	amInit();
	
	char titleID[50];
	
	r = AM_GetTitleCount(MEDIATYPE_SD,&titleCount);
	if(R_FAILED(r)) printf("Failed: 0x%lx\n",r);
	
	
	titleIds = (u64*) calloc(titleCount, sizeof(u64));

	r = AM_GetTitleList(&titleCount,MEDIATYPE_SD,titleCount,titleIds);
	if(R_FAILED(r)) printf("Failed: 0x%lx\n",r);

	bool mainMenu = true;
	drawMain(topScreen,bottomScreen);
	
	while(aptMainLoop()){
	gspWaitForVBlank();
	hidScanInput();

	u32 kDown = hidKeysDown();

	if(kDown & KEY_DDOWN)
	{
		pos++;
		consoleClear();
		
		if(mainMenu){
			if(pos > 2) pos = 2;
	 		drawMain(topScreen,bottomScreen);
		}
		
		else if(copyMenu){
			int maxPos = getPluginCount();
			if(pos > (maxPos - 1) ) pos =  0;
			listPlugins(pos,topScreen,bottomScreen);
		}
		
		else if(!mainMenu && !copyMenu)
		{
		   pluginFolders(titleID,topScreen,bottomScreen);
		 }
	}
	
	else if (kDown & KEY_DUP)
	{
			pos--;
			consoleClear();
			if(mainMenu)
			{
				if(pos < 1) pos = 1;
				drawMain(topScreen,bottomScreen);
		  
			}
			
			else if(copyMenu)
			{
				if(pos < 0) pos = 1;
				listPlugins(pos,topScreen,bottomScreen);
			}
			
			else if(!mainMenu && !copyMenu)
			{
				pluginFolders(titleID,topScreen,bottomScreen);
			}

	}
	
	else if (kDown & KEY_A )
	{
		
		if(pos == 1 && mainMenu){
			mainMenu = false;
			copyMenu = false;
			pluginFolders(titleID,topScreen,bottomScreen);
			}
		else if(pos == 2 && mainMenu){
			mainMenu=false;
			listPlugins(pos,topScreen,bottomScreen);
			copyMenu=true;
			}
		
		else if(copyMenu){
			 copyPlugin(pos,topScreen,bottomScreen); 
		 }

	}
	else if (kDown & KEY_B)
	{
	   consoleClear();
	   mainMenu = true;
	   copyMenu = false;
	   drawMain(topScreen,bottomScreen);
	}
	
	else if (kDown & KEY_START) break;

	gfxSwapBuffers();
	gfxFlushBuffers();

	
	}
	free(titleIds);
	amExit();
	gfxExit();
	
	return 0;
	
	}

void pluginFolders(char* titleID,PrintConsole top, PrintConsole bottom){
	
	consoleSelect(&bottom);
	consoleClear();
	consoleSelect(&top);
	consoleClear();
	
	for(u32 i = 0; i < titleCount;i++)
	{

			sprintf(titleID,"sdmc:/plugin/000%llx",titleIds[i]);

		DIR* dir = opendir(titleID);
		if (dir){
				printf("Folder plugin/%llx exists, skipping...\n",titleIds[i]);
		}
		else{
				r = mkdir(titleID,0777);
				printf("Creating folder->%llx\n",titleIds[i]);
		}
		if(R_FAILED(r)) printf("Failed creating directory: 0x%lx\n",r);
		closedir(dir);
	}

	printf("Done!\n");
	consoleSelect(&bottom);
	printf("Press <B> to go back or <START> to exit\n");

}

void listPlugins(int pos, PrintConsole top, PrintConsole bottom){
   
   
   consoleSelect(&bottom);
   consoleClear();
   printf("< A > Select < B > Back\n");
   consoleSelect(&top);
   consoleClear();
   printf("== Detected plugins ==\n");
   DIR *p;
   struct dirent *pp;
   p = opendir ("sdmc:/");
   if (p != NULL)
   {

	  int i = 0;

	  while ((pp = readdir (p))!=NULL) {

			  int length = strlen(pp->d_name);

				if (strncmp(pp->d_name + length - 4, ".plg", 4) == 0) {
						strcpy(pluginNames[i],pp->d_name);

						arrowMenu(i,pos);printf("%s\n",pp->d_name);
						i++;

					}

	}
	}

		closedir(p);
		
}


void copyPlugin(int pos, PrintConsole top, PrintConsole bottom){
		consoleSelect(&bottom);
		consoleClear();
		consoleSelect(&top);
		consoleClear();
		
		char target_dir[256];
		char plugin[256];
		printf("Wait...\n");
		for(int i =0;i<titleCount;i++){
				
				sprintf(target_dir,"sdmc:/plugin/000%llx/%s",titleIds[i],pluginNames[pos]);
				sprintf(plugin,"sdmc:/%s",pluginNames[pos]);
				if(access(target_dir, F_OK) != -1)
				{
					printf("Exists, skipping... %s \n",target_dir);	
				}
				else {
					if(fileCopy(plugin,target_dir) != 0){
						printf("Failed copying the plugin.\n");
						break;
					}
					else{
						printf("Copied plugin on %s\n",target_dir);
					}
						
				}
		}
		printf("Done!\n");
		consoleSelect(&bottom);
		printf("Press <B> to go back or <START> to exit\n ");
		
}


int getPluginCount(){
//  FIXME: Find another way of getting the plugin count

   DIR *p;
   struct dirent *pp;
   int i = 0;
   p = opendir ("/");
   if (p != NULL)
	{
	  while ((pp = readdir (p))!=NULL)
	  {
		  int length = strlen(pp->d_name);
		  if (strncmp(pp->d_name + length - 4, ".plg", 4) == 0) i++;

	  }
	}
	return i;
   closedir(p);
}


int fileCopy(const char* filein, const char* fileout) {
	
		FILE* fin = fopen(filein, "r");
		int c;

		if (!fin) return 1;
		
		FILE* fout = fopen(fileout, "w+");

		if (!fout) return 2;

		while ((c = fgetc(fin)) != EOF) {
				r = fputc(c, fout);
		}
		
		fclose(fin);
		fclose(fout);
		return 0;
}
