#include "fbxutil.h"


bool isWithin(char v, char lo, char hi){return v>=lo && v<=hi;}
bool	BeginsSymbol(char c) {
    return	isWithin(c,'a','z') || isWithin(c,'A','Z') || c=='_';
}
bool	IsAlphaNumeric(char c)
{
    return	isWithin(c,'0','9') || BeginsSymbol(c);
}
bool	IsWhitespace(char c)
{
    return	(c==' ' || c=='\t' || c=='\n' || c==0xa);
}
bool IsSeparator(char c)
{
    return	c==',' || c==';';
}

void	ReadLine(FILE* fp, char* line, int maxlen)
{
    fgets(line,maxlen,fp);
}

void	SkipLine(std::ifstream& src) {src.ignore(1024,'\n');}


bool	ReadQuotedString(FILE* fp, char* txt, int maxlen) {
    char c=fgetc(fp);
    while (IsWhitespace(c )) c=fgetc(fp);
    if (!c)
        return	false;
    ASSERT(c=='\"')
    c=fgetc(fp);
    char*	dst=txt;
    do {
        ASSERT((dst-txt)<maxlen-1)
        *dst++=c;
        c=fgetc(fp);
    } while (c!='\"');
    *dst++=0;
    return	true;
}

int ReadInt(FILE* fp)
{
    char c;
    c=fgetc(fp);
    while (IsWhitespace( c) || IsSeparator(c)) {
        c=fgetc(fp);
    }
    bool	neg=false;
    int	value=0;
    if (c=='-') {neg=true;c=fgetc(fp);}
    while (isWithin(c,'0','9')){
        value*=10;
        value+=c-'0';
        c=fgetc(fp);
    }
    return	neg?-value:value;
}

float ReadFloat2(FILE* fp)
{
    char c;
    c=fgetc(fp);
    while (IsWhitespace( c ) || IsSeparator(c)) {
        c=fgetc(fp);
    }
    bool	neg=false;
    float	value=0.f;
    if (c=='-') {neg=true;c=fgetc(fp);}

    while (isWithin(c,'0','9')) {
        value*=10.f;
        value+=(float)(c-'0');
        c=fgetc(fp);
    }

    if (c=='E') {
        // hello debugger
        ASSERT(0 && "read exponent not handled yet");
    }
    if (c=='.')
    {
        c=fgetc(fp);
        float place=1.f;
        while (isWithin(c,'0','9')) {
            place*=1/10.f;
            value+=place*(float)(c-'0');
            c=fgetc(fp);
        }
    }

    return	neg?-value:value;
}
float	ReadFloat(FILE* fp) {
    float f;
    int	r=fscanf(fp,"%f",&f);
    ASSERT(r==1 && "read float failed");
    return	f;
}

bool
ReadToken(FILE* fp, char* token, int maxlen)
{
    char c;

    c=fgetc(fp);
    while (IsWhitespace( c ) && c) {
        c=fgetc(fp);
    }
    if (!c)
        return	false;
    char*	dst = token;
    while (dst < token+maxlen && IsAlphaNumeric( c) && c)
    {
        *dst++=c;
        c=fgetc(fp);
    }
    return	false;
}

// assume we're inside block
void	ExitBlock(FILE* fp)
{
    int	count=1;
    char c;
    do
    {
        c=fgetc(fp);
        if (c=='{') count++;
        else if (c=='}') {
            count--;
        }
    } while (c && count>0 && c>=0);
    return;
}
bool	EnterBlock(
        FILE* fp,
        char* blockname,
        int maxBlockName,
        char* insname,
        int maxInsName)
{

    char c;
    c=fgetc(fp);
    while (IsWhitespace(c ) || IsSeparator(c ))
    {
        c=fgetc(fp);
    }
    if (!c)
        return false;
    char* dst;
    dst = blockname;
    while (IsAlphaNumeric(c ) && (dst<blockname+maxBlockName-1)) {
        *dst++ = c;
        c=fgetc(fp);
    }
    *dst++=0;

    c=fgetc(fp);
    while (IsWhitespace( c ))
    {	c=fgetc(fp);
    }
     if (!c)
        return false;
    dst=insname;
    while (IsAlphaNumeric(c ) && (dst<insname+maxInsName-1) )
    {
        *dst++ = c;
        c=fgetc(fp);
    }
    *dst++=0;
    while (IsWhitespace(c ) && c) {
        c=fgetc(fp);
    }
    ASSERT(c=='{' && "Expected brace open for template");
    return true;
}

bool	SkipComma(std::ifstream& src)
{
    char c; src>>c; if (c==',') return true; else src.unget(); return false;
}

void	ExitBlock(std::ifstream& src)
{
    int	count=0;
    char c;

    do {
        c=src.get();
        if (c=='{')
        {
            count++;
        } else
        if (c=='}') {
            count--;
        }
    } while (count>=0 && !src.eof());
};

void	SkipWhitespace(std::ifstream& src) {
    do {
        char c = src.peek();
        //		printf("peek=%c\n",c);
        if (c==' ' || c=='\t' || c=='\n' || c==0xa || c==0xd) {
            src.get(c);
            continue;
        } else
            break;
    }
    while (!src.eof());
}

void    SkipWhitespaceAndSemicolonComments(std::ifstream& src)
{
    do {
        SkipWhitespace(src);
        char c = src.peek();
        if (c==';') {
            SkipLine(src);
        }
        else break;
    }
    while (!src.eof());
}

bool	EnterBlock(std::ifstream& src)
{
    char c;
    SkipWhitespace(src);
    do { if (src.peek()=='}') return false; c=src.get(); } while (c!='{' && !src.eof());
    if (c=='{') return true;
    else return false;
}

void	ExitBlock2(std::ifstream& src)
{
    int	count=0;
    char c;

    do {
        c=src.get();
        if (c=='{')
        {
            count++;
        } else
        if (c=='}') {
            count--;
        }
    } while (count>=0 && !src.eof());
};

void	SkipBlock(std::ifstream& src)
{
    if (EnterBlock(src)) { ExitBlock(src); };
};


#ifdef TEST_FBXUTIL
int	main(int argc, const char* argv)
{
    FILE*	fp = fopen("testascii.txt","rb");
    char block[512],ins[512],str[512];
    ReadString(fp,str,512);
    printf("read string: \"%s\"\n", str);
    printf("read 4 floats: %.3f %.3f %.3f %.3f\n", ReadFloat(fp), ReadFloat(fp), ReadFloat(fp), ReadFloat(fp));
    EnterBlock(fp,block,sizeof(block),ins,sizeof(ins));
    printf("block: %s %s\n",block,ins);
    ExitBlock(fp);
    fclose(fp);

    float	f0,f1,f2,f3;
    char c;
    std::string	str2;
    ifstream	src;
    src.open("testascii.txt");
    src >> str2;
    src >> f0 >> c >> f1 >>c>> f2 >>c>> f3;
    printf("ifstream test: %s %f %f %f %f\n",(char*) &str2[0],f0, f1,f2,f3);
    src.close();
}
#endif
