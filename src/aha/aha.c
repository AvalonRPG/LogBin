/*
 The contents of this file are subject to the Mozilla Public License
 Version 1.1 (the "License"); you may not use this file except in
 compliance with the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

 Software distributed under the License is distributed on an "AS IS"
 basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 License for the specific language governing rights and limitations
 under the License.

 Alternatively, the contents of this file may be used under the terms
 of the GNU Lesser General Public license (the	"LGPL License"), in which case the
 provisions of LGPL License are applicable instead of those
 above.

 For feedback and questions about my Files and Projects please mail me,
 Alexander Matthes (Ziz) , zizsdl_at_googlemail.com
*/
#define AHA_VERSION "0.4.7.3"
#define AHA_YEAR "2015"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define     IAC     255
#define     DONT    254
#define     DO      253
#define     WONT    252
#define     WILL    251
#define     SB      250
#define     GA      249   /* Used for prompt marking */
#define     EL      248
#define     EC      247
#define     AYT     246
#define     AO      245
#define     IP      244
#define     BREAK   243
#define     DM      242
#define     NOP     241
#define     SE      240
#define     EOR     239   /* Used for prompt marking */
#define     ABORT   238
#define     SUSP    237
#define     xEOF    236

int future=0;
int future_char=0;

int getNextChar(register FILE* fp)
{
	int c;
	if (future)
	{
		future=0;
		return future_char;
	}
	if ((c = fgetc(fp)) != EOF)
		return c;
	fprintf(stderr,"Unknown Error in File Parsing!\n");
	exit(1);
}

typedef struct selem *pelem;
typedef struct selem {
	unsigned char digit[8];
	unsigned char digitcount;
	pelem next;
} telem;


pelem parseInsert(char* s)
{
	pelem firstelem=NULL;
	pelem momelem=NULL;
	unsigned char digit[8];
	unsigned char digitcount=0;
	int pos=0;
	for (pos=0;pos<1024;pos++)
	{
		if (s[pos]=='[')
			continue;
		if (s[pos]==';' || s[pos]==0)
		{
			if (digitcount==0)
			{
				digit[0]=0;
				digitcount=1;
			}

			pelem newelem=(pelem)malloc(sizeof(telem));
			for (unsigned char a=0;a<8;a++)
				newelem->digit[a]=digit[a];
			newelem->digitcount=digitcount;
			newelem->next=NULL;
			if (momelem==NULL)
				firstelem=newelem;
			else
				momelem->next=newelem;
			momelem=newelem;
			digitcount=0;
			memset(digit,0,8);
			if (s[pos]==0)
				break;
		}
		else
		if (digitcount<8)
		{
			digit[digitcount]=s[pos]-'0';
			digitcount++;
		}
	}
	return firstelem;
}

void deleteParse(pelem elem)
{
	while (elem!=NULL)
	{
		pelem temp=elem->next;
		free(elem);
		elem=temp;
	}
}

int main(int argc,char* args[])
{
	char* filename=NULL;
	register FILE *fp = stdin;
	int colorshema=0; //0:normal, 1:black, 2:pink
	int iso=-1; //utf8
	char stylesheet=0;
	char htop_fix=0;
	char line_break=0;
	char parse_times=0;
	char word_wrap=0;
	char no_header=0;
	char times[500000];
	//Searching Parameters
	for (int p = 1;p<argc;p++)
	{
		if ((strcmp(args[p],(char*)"--help")==0) || (strcmp(args[p],(char*)"-h")==0) || (strcmp(args[p],(char*)"-?")==0))
		{
			printf("\033[1;31mAnsi Html Adapter\033[0m Version "AHA_VERSION"\n");
			printf("\033[1maha\033[0m takes SGR-colored Input and prints W3C conform HTML-Code\n");
			printf("use: \033[1maha\033[0m <\033[4moptions\033[0m> [\033[4m-f file\033[0m]\n");
			printf("     \033[1maha\033[0m (\033[4m--help\033[0m|\033[4m-h\033[0m|\033[4m-?\033[0m)\n");
			printf("\033[1maha\033[0m reads the Input from a file or stdin and writes HTML-Code to stdout\n");
			printf("\033[4moptions\033[0m: --black,      -b: \033[1;30m\033[1;47mBlack\033[0m Background and \033[1;37mWhite\033[0m \"standard color\"\n");
			printf("         --pink,       -p: \033[1;35mPink\033[0m Background\n");
			printf("         --stylesheet, -s: Use a stylesheet instead of inline styles\n");
			printf("         --iso X,    -i X: Uses ISO 8859-X instead of utf-8. X must be 1..16\n");
			printf("         --times,  -t: Renders times \n");
			printf("         --line-fix,   -l: Uses a fix for inputs using control sequences to\n");
			printf("                           change the cursor position like htop. It's a hot fix,\n");
			printf("                           it may not work with any program like htop. Example:\n");
			printf("                           \033[1mecho\033[0m q | \033[1mhtop\033[0m | \033[1maha\033[0m -l > htop.htm\n");
			printf("         --word-wrap,  -w: Wrap long lines in the html file. This works with\n");
			printf("                           CSS3 supporting browsers as well as many older ones.\n");
			printf("         --no-header,  -n: Don't include header into generated HTML,\n");
			printf("                           useful for inclusion in full HTML files.\n");
			printf("Example: \033[1maha\033[0m --help | \033[1maha\033[0m --black > aha-help.htm\n");
			printf("         Writes this help text to the file aha-help.htm\n\n");
			printf("Copyleft \033[1;32mAlexander Matthes\033[0m aka \033[4mZiz\033[0m "AHA_YEAR"\n");
			printf("         \033[5;36mzizsdl@googlemail.com\033[0m\n");
			printf("         \033[5;36mhttp://ziz.delphigl.com/tool_aha.php\033[0m\n");
			printf("This application is subject to the \033[1;34mMPL\033[0m or \033[1;34mLGPL\033[0m.\n");
			return 0;
		}
		else
		if ((strcmp(args[p],(char*)"--version")==0) || (strcmp(args[p],(char*)"-v")==0))
		{
			printf("\033[1;31mAnsi Html Adapter\033[0m Version "AHA_VERSION"\n");
			return 0;
		}
		else
		if ((strcmp(args[p],"--line-fix")==0) || (strcmp(args[p],"-l")==0))
		{
			htop_fix=1;
		}
		else
		if ((strcmp(args[p],"--no-header")==0) || (strcmp(args[p],"-n")==0))
		{
			no_header=1;
		}
		else
		if ((strcmp(args[p],"--word-wrap")==0) || (strcmp(args[p],"-w")==0))
			word_wrap=1;
		else
		if ((strcmp(args[p],"--times")==0) || (strcmp(args[p],"-t")==0))
			parse_times=1;
		else
		if ((strcmp(args[p],"--black")==0) || (strcmp(args[p],"-b")==0))
			colorshema=1;
		else
		if ((strcmp(args[p],"--pink")==0) || (strcmp(args[p],"-p")==0))
			colorshema=2;
		else
		if ((strcmp(args[p],"--stylesheet")==0) || (strcmp(args[p],"-s")==0))
			stylesheet=1;
		else
		if ((strcmp(args[p],"--iso")==0) || (strcmp(args[p],"-i")==0))
		{
			if (p+1>=argc)
			{
				fprintf(stderr,"No ISO code given!\n");
				return 0;
			}
			iso = atoi(args[p+1]);
			if (iso<1 || iso>16)
			{
				fprintf(stderr,"not a valid ISO code: ISO 8859-%s\n",args[p+1]);
				return 0;
			}
			p++;
		}
		else
		if (strcmp(args[p],"-f")==0)
		{
			if (p+1>=argc)
			{
				fprintf(stderr,"no file to read given after \"-f\"!\n");
				return 0;
			}
			fp = fopen(args[p+1],"r");
			if (fp==NULL)
			{
				fprintf(stderr,"file \"%s\" not found!\n",args[p+1]);
				return 0;
			}
			p++;
			filename=args[p];
		}
		else
		{
			fprintf(stderr,"Unknown parameter \"%s\"\n",args[p]);
			return 0;
		}
	}

	if (parse_times == 1)
		printf("<pre id='full'>\n");

	//Begin of Conversion
	unsigned int c;
	int fc = -1; //Standard Foreground Color //IRC-Color+8
	int bc = -1; //Standard Background Color //IRC-Color+8
	int ul = 0; //Not underlined
	int bo = 0; //Not bold
	int bl = 0; //No Blinking
	int ofc,obc,oul,obo,obl; //old values
	int line=0;
	int momline=0;
	int newline=-1;
	int temp;
	int nulc = -1;
	int timex = -1; // elapsed  time - offset
	int timey = -1; // datalen
	int timeline = 0;
	while ((c=fgetc(fp)) != EOF)
	{
		if (c==0 && nulc != 2) {
			// 00XY
			if (nulc == -1) nulc = 1;
			else if (nulc == 1) nulc = 2;
			continue;
		}
		if (nulc == 2) {
			if (timex == -1) timex = c;
			else if (timey == -1) timey = c;

			//printf("\ntime(%d)\n", c);
			char str[4];

			if (timex != -1 && timey != -1)  {
				sprintf(str, "{line: %d, time: %d, length: %d},", timeline, timex, timey);
				if (parse_times == 1)
					sprintf(times, "%s", strcat(times, str));

				timex=timey=nulc = -1;
			}
			continue;
		}
		if (c=='\033')
		{
			//Saving old values
			ofc=fc;
			obc=bc;
			oul=ul;
			obo=bo;
			obl=bl;
			//Searching the end (a letter) and safe the insert:
			c='0';
			char buffer[1024];
			int counter=0;
			while ((c<'A') || ((c>'Z') && (c<'a')) || (c>'z'))
			{
				c=getNextChar(fp);
				buffer[counter]=c;
				if (c=='>') //end of htop
					break;
				counter++;
				if (counter>1022)
					break;
			}
			buffer[counter-1]=0;
			pelem elem;
			switch (c)
			{
				case 'm':
					//printf("\n%s\n",buffer); //DEBUG
					elem=parseInsert(buffer);
					pelem momelem=elem;
					while (momelem!=NULL)
					{
						//jump over zeros
						int mompos=0;
						while (mompos<momelem->digitcount && momelem->digit[mompos]==0)
							mompos++;
						if (mompos==momelem->digitcount) //only zeros => delete all
						{
							bo=0;ul=0;bl=0;fc=-1;bc=-1;
						}
						else
						{
							switch (momelem->digit[mompos])
							{
								case 1: bo=1; break;
								case 2: if (mompos+1<momelem->digitcount)
												switch (momelem->digit[mompos+1])
												{
													case 1: //Reset blink and bold
														bo=0;
														bl=0;
														break;
													case 4: //Reset underline
														ul=0;
														break;
														case 7: //Reset Inverted
														temp = bc;
														if (fc == -1 || fc == 9)
														{
															if (colorshema!=1)
																bc = 0;
															else
																bc = 7;
														}
														else
															bc = fc;
														if (temp == -1 || temp == 9)
														{
															if (colorshema!=1)
																fc = 7;
															else
																fc = 0;
														}
														else
															fc = temp;
														break;
												}
												break;
						case 3: if (mompos+1<momelem->digitcount)
											fc=momelem->digit[mompos+1];
										break;
						case 4: if (mompos+1==momelem->digitcount)
											ul=1;
										else
											bc=momelem->digit[mompos+1];
										break;
						case 5: bl=1; break;
						case 7: //TODO: Inverse
										temp = bc;
										if (fc == -1 || fc == 9)
										{
											if (colorshema!=1)
												bc = 0;
											else
												bc = 7;
										}
										else
											bc = fc;
										if (temp == -1 || temp == 9)
										{
											if (colorshema!=1)
												fc = 7;
											else
												fc = 0;
										}
										else
											fc = temp;
										break;
							}
						}
						momelem=momelem->next;
					}
					deleteParse(elem);
				break;
				case 'H':
					if (htop_fix) //a little dirty ...
					{
						elem=parseInsert(buffer);
						pelem second=elem->next;
						if (second==NULL)
							second=elem;
						newline=second->digit[0]-1;
						if (second->digitcount>1)
							newline=(newline+1)*10+second->digit[1]-1;
						if (second->digitcount>2)
							newline=(newline+1)*10+second->digit[2]-1;
						deleteParse(elem);
						if (newline<line)
							line_break=1;
					}
				break;
			}
			if (htop_fix)
				if (line_break)
				{
					for (;line<80;line++)
						printf(" ");
				}
			//Checking the differences
			if ((fc!=ofc) || (bc!=obc) || (ul!=oul) || (bo!=obo) || (bl!=obl)) //ANY Change
			{
				if ((ofc!=-1) || (obc!=-1) || (oul!=0) || (obo!=0) || (obl!=0))
					printf("</span>");
				if ((fc!=-1) || (bc!=-1) || (ul!=0) || (bo!=0) || (bl!=0))
				{
					printf("<span class=\"");
					switch (fc)
					{
						case	0: printf("black ");
										 break; //Black
						case	1: printf("red ");
										 break; //Red
						case	2: printf("green ");
										 break; //Green
						case	3: printf("yellow ");
										 break; //Yellow
						case	4: printf("blue ");
										 break; //Blue
						case	5: printf("purple ");
										 break; //Purple
						case	6: printf("cyan ");
										 break; //Cyan
						case	7: printf("white ");
										 break; //White
						case	9: printf("reset ");
										 break; //Reset
					}
					switch (bc)
					{
						case	0: printf("bg-black ");
										 break; //Black
						case	1: printf("bg-red ");
										 break; //Red
						case	2: printf("bg-green ");
										 break; //Green
						case	3: printf("bg-yellow ");
										 break; //Yellow
						case	4: printf("bg-blue ");
										 break; //Blue
						case	5: printf("bg-purple ");
										 break; //Purple
						case	6: printf("bg-cyan ");
										 break; //Cyan
						case	7: printf("bg-white ");
										 break; //White
						case	9: printf("bg-reset ");
										 break; //Reset
					}
					if (ul) printf("underline ");
					if (bo) printf("bold ");
					if (bl) printf("blink ");
					printf("\">");
				}
			}
		}
		// printf("%c|%d\n", c, c);
		// if (c == 13) printf("\n");
		// continue;
		else
		if (c==13 && htop_fix)
		{
			for (;line<80;line++)
				printf(" ");
			line=0;
			momline++;
			printf("\n");
		}
		else if (c!=8)
		{
			line++;
			if (line_break)
			{
				printf("\n");
				line=0;
				line_break=0;
				momline++;
			}
			if (newline>=0)
			{
				while (newline>line)
				{
					printf(" ");
					line++;
				}
				newline=-1;
			}
			if (c >= xEOF && c <= IAC) {
			 if (c==GA){
					printf("\n");
					line=0;
					line_break=0;
					momline++;
					timeline++;
					sprintf(times, "%s{line: %d},", times, timeline);
					continue;
				}
			}
			else {
				switch (c)
				{
					case '&':	printf("&amp;"); break;
					case '\"': printf("&quot;"); break;
					case '<':	printf("&lt;"); break;
					case '>':	printf("&gt;"); break;
					case '\n':
						momline++;
						timeline++;
						line=0;
						sprintf(times, "%s{line: %d},", times, timeline);
					default: printf("%c",c);
				}
			}
			if (iso>0) //only at ISOS
				if ((c & 128)==128) //first bit set => there must be followbytes
				{
					int bits=2;
					if ((c & 32)==32)
						bits++;
					if ((c & 16)==16)
						bits++;
					for (int meow=1;meow<bits;meow++)
						printf("%c",getNextChar(fp));

				}
		}
	}

	//Footer
	if ((fc!=-1) || (bc!=-1) || (ul!=0) || (bo!=0) || (bl!=0))
		printf("</span>\n");

	if (parse_times) {
		printf("</pre>\n");
		if (times)
			printf("<script>var times = [%s];</script>", times);
	}

	if (fp!=stdin)
		fclose(fp);
	return 0;
}
