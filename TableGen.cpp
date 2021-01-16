#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <random>
using namespace std;
#include "Tree.h"
#include <vector>
#include <map>
//define to improve quality, may cause Overfitting
#define TRIGRAM
//to decent rate of text,and prevent Overfitting
#define RND_CRIT 1
class Word {
public:
    Word() {
        freq = 1;
    }
    char* operator*(){
        return word;
    }
    char* operator=(char* w) {
        return word = w;
    }
    operator int() {
        return freq;
    }
    int operator== (const Word& ir) const {
        return strcmp(word,ir.word) == 0;
    }
    int operator< (const Word& ir) const {
        return strcmp(word,ir.word) < 0;
    }
    int operator++(){
        return freq++;
    }
    int operator++(int) {
        return ++freq;
    }
private:
    char *word;
    int freq;
};

class WordCounter : public SortTree<Word> {

public:
    std::vector<string>words;
    WordCounter() {
        differentWords = wordCnt = 0;
    }
    void run(ifstream&,char*);
private:
    int differentWords,
        wordCnt;
    void visit(TNode<Word>*);
};

void WordCounter::visit(TNode<Word> *p) {
    differentWords++;
    words.push_back(*(p->info));
    wordCnt +=(int)p->info;
}

void WordCounter::run(ifstream& fIn, char *fileName) {
    
    char ch = ' ', i;
    char s[100];
    Word rec;
    while (!fIn.eof()) {
        while (1)
            if (!fIn.eof() && !isalpha(ch))
                 fIn.get(ch);
            else break;
        if (fIn.eof())
             break;
        for (i = 0; !fIn.eof() && isalpha(ch); i++) {
             s[i] = toupper(ch);
             fIn.get(ch);
        }
        s[i] = '\0';
        if (!(rec = new char[strlen(s)+1])) {
             cerr << "OOM!\n";
             exit(1);
        }
        strcpy(*rec,s);
        Word *p = search(rec);
        if (p == 0)
             insert(rec);
        else (*p)++;
    }
    inorder();
}

typedef map<string,int*> Gram;
int rate[27]={};
char chs[] = { "ABCDEFGHIJKLMNOPQRSTUVWXYZ" };
int ix;
int mapCHToInt(char& ch) {
    return ch - 'A';
}
char mapIntToCH(int& ch) {
    return ch + 'A';
}
unsigned int trate;
//insert gram
void insert_gram(Gram& gram,string& window, char& predict) {
    auto mgref = gram.find(window);
    if (mgref == gram.end()) {
        int* mpr=new int[27]{};
        memset(mpr, 0, sizeof(int) * 27);
        mpr[mapCHToInt(predict)] = 1;
        string s(window);
        gram.emplace(s, mpr);
    }
    else {
        mgref->second[mapCHToInt(predict)]++;
    }
}
void insert_gram(Gram& gram, char& window, char& predict) {
    char ch[2];
    ch[0] = window;
    ch[1] = 0;
    string wind(ch);
    insert_gram(gram,wind, predict);
}
void insert_gram(Gram& gram, char& window1,char& window2, char& predict) {
    char ch[3];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = 0;
    string wind(ch);
    insert_gram(gram, window2, predict);
    insert_gram(gram, wind, predict);
}
#ifdef TRIGRAM
void insert_gram(Gram& gram, char& window1, char& window2,char & window3,char& predict) {
    char ch[4];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = window3;
    ch[3] = 0;
    string wind(ch);
    insert_gram(gram, window2,window3, predict);
    insert_gram(gram, wind, predict);
}
#else
void insert_gram(Gram& gram, char& window1, char& window2, char& window3, char& predict) {
    insert_gram(gram, window2, window3, predict);
}
#endif
int* search_gram(Gram& gram, string& window) {
    auto mgref = gram.find(window);
    if (mgref == gram.end()) {
        return 0;
    }
    return mgref->second;
}
//random letter
template<class RNG>
char rnd_letter(RNG random) {
    //uniform_int_distribution<unsigned int> distr(0,26);
    int cur = ((unsigned int)rand())%26;
    return mapIntToCH(cur);
    //cout << cur;
    /*int i = 0;
    for (; i < 26; i++) {
        cur -= rate[i];
       if (cur <= 0)return mapIntToCH(i);
    }*/
}
// normal gram query
template<class RNG>
char rnd_gram(Gram& gram,string window, RNG random) {
    int* rts = search_gram(gram, window);
    if (rts == 0)
        return rnd_letter(random);
    int total = 0;
    for (auto r = 0; r < 26; r++) {
        total += rts[r] + RND_CRIT;
    }
    uniform_int_distribution<unsigned int> distr(0, total);
    int cur = distr(random);
    //cout << cur;
    for (auto r = 0; r < 26; r++) {
        cur -= rts[r] + RND_CRIT;
        if (cur <= 0)return mapIntToCH(r);
    }

}
template<class RNG>
char rnd_gram(Gram& gram, char& window,RNG random) {
    char ch[2];
    ch[0] = window;
    ch[1] = 0;
    string wind(ch);
    return rnd_gram(gram, wind, random);
}
template<class RNG>
char rnd_gram(Gram& gram, char& window1,char&window2,RNG random) {
    char ch[3];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = 0;
    string wind(ch);
    return rnd_gram(gram, wind, random);
}
#ifdef TRIGRAN
template<class RNG>
char rnd_gram(Gram& gram, char& window1, char& window2,char& window3, RNG random) {
    char ch[4];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = window3;
    ch[3] = 0;
    string wind(ch);
    return rnd_gram(gram, wind, random);
}
#else
template<class RNG>
char rnd_gram(Gram& gram, char& window1, char& window2, char& window3, RNG random) {
    return rnd_gram(gram, window2,window3, random);
}
#endif
//2d gram query
template<class RNG>
char rnd_gram2d(Gram& gram, string window1, string window2,RNG random) {
    auto rts1 = search_gram(gram, window1);
    auto rts2 = search_gram(gram, window2);
    if (rts1 == 0 && rts2 == 0)
        return rnd_letter(random);
    int total = 0;
    if (rts1 != 0)
        for (auto r = 0; r < 26; r++) {
            total += rts1[r] + RND_CRIT;
        }
    if (rts2 != 0)
        for (auto r = 0; r < 26; r++) {
            total += rts2[r] + RND_CRIT;
        }
    uniform_int_distribution<unsigned int> distr(0, total);
    int cur = distr(random);
    //cout << cur;
    if (rts1 != 0)
        for (auto r = 0; r < 26; r++) {
            cur -= rts1[r] + RND_CRIT;
            if (cur <= 0)return mapIntToCH(r);
        }
    if (rts2 != 0)
        for (auto r = 0; r < 26; r++) {
            cur -= rts2[r] + RND_CRIT;
            if (cur <= 0)return mapIntToCH(r);
        }
}
template<class RNG>
char rnd_gram2d(Gram& gram, char& window1, char& window2, RNG random) {
    char ch[2];
    ch[0] = window1;
    ch[1] = 0;
    string wind(ch);
    ch[0] = window2;
    string wind2(ch);
    return rnd_gram2d(gram, wind, wind2, random);
}
template<class RNG>
char rnd_gram2d(Gram& gram, char& window1, char& window2, char& window3, char& window4, RNG random) {
    char ch[3];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = 0;
    string wind(ch);
    ch[0] = window3;
    ch[1] = window4;
    string wind2(ch);
    return rnd_gram2d(gram, wind, wind2, random);
}
#ifdef TRIGRAN
template<class RNG>
char rnd_gram2d(Gram& gram, char& window1, char& window2, char& window3, char& window4, char& window5, char& window6, RNG random) {
    char ch[4];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = window3;
    ch[3] = 0;
    string wind(ch);
    ch[0] = window4;
    ch[1] = window5;
    ch[2] = window6;
    string wind2(ch);
    return rnd_gram2d(gram, wind, wind2, random);
}
#else
template<class RNG>
char rnd_gram2d(Gram& gram, char& window1, char& window2, char& window3, char& window4, char& window5, char& window6, RNG random) {
    return rnd_gram2d(gram, window2, window3, window5, window6, random);
}
#endif
// unbalanced 2d gram query
template<class RNG>
char rnd_gram2dub(Gram& gram, char& window1, char& window2, char& window3, RNG random) {
    char ch[3];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = 0;
    string wind(ch);
    char chx[2];
    chx[0] = window3;
    chx[1] = 0;
    string wind2(chx);
    return rnd_gram2d(gram, wind, wind2, random);
}
#ifdef TRIGRAN
template<class RNG>
char rnd_gram2dub(Gram& gram, char& window1, char& window2, char& window3, char& window4,char& window5, RNG random) {
    char ch[4];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = window3;
    ch[3] = 0;
    string wind(ch);
    char chx[3];
    chx[0] = window4;
    chx[1] = window5;
    chx[2] = 0;
    string wind2(chx);
    return rnd_gram2d(gram, wind, wind2, random);
}
template<class RNG>
char rnd_gram2dub(Gram& gram, char& window1, char& window2, char& window3,char&window4, RNG random) {
    char ch[4];
    ch[0] = window1;
    ch[1] = window2;
    ch[2] = window3;
    ch[3] = 0;
    string wind(ch);
    char chx[2];
    chx[0] = window4;
    chx[1] = 0;
    string wind2(chx);
    return rnd_gram2d(gram, wind, wind2, random);
}
#else
template<class RNG>
char rnd_gram2dub(Gram& gram, char& window1, char& window2, char& window3, char& window4, char& window5, RNG random) {
    return rnd_gram2d(gram,window2,window3,window4,window5, random);
}
template<class RNG>
char rnd_gram2dub(Gram& gram, char& window1, char& window2, char& window3, char& window4, RNG random) {
    return rnd_gram2dub(gram, window2, window3, window4, random);
}
#endif

int main(int argc, char* argv[]) {
    memset(rate, 0, sizeof(int) * 27);
    char fileName[80];
    WordCounter wc;
    if (argc < 2) {
         cout << "input txt file path of the sample text: ";
         cin  >> fileName;
    }
    else strcpy(fileName,argv[1]);
    ifstream fIn(fileName);
    if (fIn.fail()) {
        cerr << "cannot open " << fileName << endl;
        return 1;
    }
    int w=0;
    int h=0;
    if (argc >= 4) {
        w = atoi(argv[2]);
        h = atoi(argv[3]);
    }
    while (w <= 2) {
        cout << endl << "input width(>2):";
        cin >> w;
    }
    while (h <= 2) {
        cout << endl << "input height(>2):";
        cin >> h;
    }
    system("cls");
    cout << "extracting words..."<<endl;
    Gram gram;
    wc.run(fIn,fileName);
    cout << "training AI..." << endl;
    for (auto w = wc.words.begin(); w != wc.words.end(); w++) {
        if (w->size() < 2)continue;
        char last=0;
        char llast=0;
        char lllast = 0;
        for (auto c = w->begin(); c != w->end(); c++) {
            rate[mapCHToInt(*c)]++;
            trate++;
            if (last != 0) {
                if (llast == 0)
                    insert_gram(gram, last, *c);
                else if (lllast == 0)
                    insert_gram(gram, llast, last, *c);
                else
                    insert_gram(gram,lllast, llast, last, *c);
            }
            lllast = llast;
            llast = last;
            last = *c;
        }
    }
    cout << "generating..." << endl;
    char** table=new char*[h];
    for (int i = 0; i < h; i++) {
        table[i] = new char[w];
    }
    //cout << trate;
   /* for (auto it = gram.begin(); it != gram.end(); it++) {
        cout << "window:" << it->first<<",";
        for (int i=0;i<27;i++)
            if(it->second[i]>0)
            cout << mapIntToCH(i) << ":" << it->second[i] << ",";
    }*/
    std::mt19937 gen(rand());
    cout << endl;

    table[1][0] = rnd_gram(gram, table[0][0], gen);
    table[1][1] = rnd_gram2d(gram, table[1][0], table[0][1], gen);
    table[1][2] = rnd_gram2dub(gram, table[1][0], table[1][1], table[0][2], gen);
    table[0][0] = rnd_letter(gen);
    table[0][1] = rnd_gram(gram, table[0][0], gen);
    table[0][2] = rnd_gram(gram, table[0][0], table[0][1], gen);
    table[2][0] = rnd_gram(gram, table[0][0], table[1][0], gen);
    table[2][1] = rnd_gram2dub(gram, table[0][0], table[1][0], table[2][0], gen);
    table[2][2] = rnd_gram2d(gram, table[0][2], table[1][2], table[2][0], table[2][1], gen);
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w; j++) {
            if (i == 0) {
                if (j > 2)
                    table[0][j] = rnd_gram(gram, table[i][j - 3], table[i][j - 2],table[i][j-1], gen);
            }
            else if(i==1){
                if(j>2)
                    table[1][j] = rnd_gram2dub(gram, table[1][j - 3], table[1][j - 2], table[1][j - 1], table[0][j], gen);
            }
            else if (i == 2) {
                if (j > 2)
                    table[2][j] = rnd_gram2d(gram, table[i - 2][j], table[i - 1][j], table[i][j - 2], table[i][j - 1], gen);
            }else {
                if (j == 0)
                    table[i][0] = rnd_gram(gram, table[i - 3][0], table[i - 2][0], table[i - 1][0], gen);
                else if (j == 1)
                    table[i][1] = rnd_gram2dub(gram, table[i - 3][1], table[i - 2][1], table[i - 1][1], table[i][0], gen);
                else if(j==2)
                    table[i][j] = rnd_gram2dub(gram, table[i - 3][2], table[i - 2][2], table[i - 1][2], table[i][0], table[i][1], gen);
                else
                    table[i][j] = rnd_gram2d(gram, table[i - 3][j], table[i - 2][j], table[i - 1][j], table[i][j - 3], table[i][j - 2], table[i][j - 1], gen);
            }
            cout << " " << table[i][j];
        }
        cout << endl;
    }
    fIn.close();
    return 0;
}
