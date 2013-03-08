#include "fbxutil.h"

//bool isWithin(char v, char lo, char hi){return v>=lo && v<=hi;}
bool	FbxUtil::IsAlphaNumeric(char c)
{
    return	IsWithin(c,'0','9') || IsSymbolStart(c);
}
bool	FbxUtil::IsWhitespace(char c)
{
    return	(c==' ' || c=='\t' || c=='\n' || c==0xa);
}
bool FbxUtil::IsSeparator(char c)
{
    return	c==',' || c==';';
}
void	FbxUtil::SkipLine(std::ifstream& src) {src.ignore(1024,'\n');}

void updateBraceDepth(int& depth,char c){
    if (c=='{')
        depth++;
    if (c=='}')
        depth--;
}

bool	FbxUtil::fbxSkipComma(std::ifstream& src)
{
    SkipWhitespace(src);
    char c; src>>c; if (c==',') return true; else src.unget(); return false;
}

void	FbxUtil::ExitBlock(std::ifstream& src)
{
    int	count=0;
    do {
        updateBraceDepth(count,src.get());
    } while (count>=0 && !src.eof());
};

void	FbxUtil::SkipWhitespace(std::ifstream& src) {
    do {
        char c = src.peek();
        if (c==' ' || c=='\t' || c=='\n' || c==0xa || c==0xd) {
            src.get(c);
            continue;
        } else
            break;
    }
    while (!src.eof());
}

void    FbxUtil::SkipWhitespaceAndSemicolonComments(std::ifstream& src)
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

bool	FbxUtil::EnterBlock(std::ifstream& src)
{
    char c;
    SkipWhitespace(src);
    do { if (src.peek()=='}') return false; c=src.get(); } while (c!='{' && !src.eof());
    if (c=='{') return true;
    else return false;
}
void	FbxUtil::SkipBlock(std::ifstream& src)
{
    if (EnterBlock(src)) { ExitBlock(src); };
};

void	FbxUtil::file_trace_line(FbxStream& src) {

    auto pos=src.tellg();
    int	i;
    cout<<"FILE["<<pos<<"]:";
    for (i=0; i<30; i++) { char c[2];c[0]=src.get();c[1]=0;cout<<c; } cout <<"\n";
    src.seekg(pos);
}


