#include "fbxutil.h"

//bool isWithin(char v, char lo, char hi){return v>=lo && v<=hi;}

bool FbxUtil::IsNumberStart(const char* s){
    char c0=s[0],c1=0; if (c0) c1=s[1];
    return	(c0=='-' && (c0=='.' || within(c1,'0','9')))
        || within(c0,'0','9');
}

bool	FbxUtil::IsAlphaNumeric(char c)
{
    return	within(c,'0','9') || IsSymbolStart(c);
}
bool	FbxUtil::IsWhitespace(char c)
{
    return	(c==' ' || c=='\t' || c=='\n' || c==0xa);
}
bool FbxUtil::IsSeparator(char c)
{
    return	c==',' || c==';';
}

void updateBraceDepth(int& depth,char c){
    if (c=='{')
        depth++;
    if (c=='}')
        depth--;
}

bool	FbxUtil::Stream::fbxSkipComma()
{
    SkipWhitespace();
    char c; (*this)>>c; if (c==',') return true; else unget(); return false;
}
void	FbxUtil::Stream::SkipLine() {ignore(1024,'\n');}
bool	FbxUtil::Stream::IsNumber()
{
    SkipWhitespace();
    char c0=0,c1=0; c0=get(); c1=peek ();  unget();
    return	(c0=='-' && (c1=='.' || within(c1,'0','9')))
        || within(c0,'0','9');
}

void	FbxUtil::Stream::ExitBlock()
{
    int	count=0;
    do {
        updateBraceDepth(count,get());
    } while (count>=0 && !eof());
};

void	FbxUtil::Stream::SkipWhitespace() {
    do {
        char c = peek();
        if (c==' ' || c=='\t' || c=='\n' || c==0xa || c==0xd) {
            get(c);
            continue;
        } else
            break;
    }
    while (!eof());
}

void    FbxUtil::Stream::SkipWhitespaceAndSemicolonComments()
{
    do {
        SkipWhitespace();
        char c = peek();
        if (c==';') {
            SkipLine();
        }
        else break;
    }
    while (!eof());
}

bool	FbxUtil::Stream::EnterBlock()
{
    char c;
    SkipWhitespace();
    do { if (peek()=='}') return false; c=get(); } while (c!='{' && !eof());
    if (c=='{') return true;
    else return false;
}
void	FbxUtil::Stream::SkipBlock()
{
    if (EnterBlock()) { ExitBlock(); };
};

void	FbxUtil::Stream::file_trace_line() {

    auto pos=tellg();
    int	i;
    cout<<"FILE["<<pos<<"]:";
    for (i=0; i<30; i++) { char c[2];c[0]=get();c[1]=0;cout<<c; } cout <<"\n";
    seekg(pos);
}


