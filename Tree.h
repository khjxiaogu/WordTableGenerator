
using namespace std;
template<class T> class SortTree;

template<class T>
class TNode {
public:
    TNode() {
        left = right = parent = 0;
    }
    TNode(const T& el, TNode *l = 0, TNode *r = 0,
                 TNode *p = 0) {
        info = el; left = l; right = r; parent = p;
    }
    T info;
    TNode *left, *right, *parent;
};

template<class T>
class SortTree {
public:
    SortTree() {
        root = 0;
    }
    void inorder() {
        inorder(root);
    }
    T* search(const T&);
    void insert(const T&);
protected:
    TNode<T> *root;
    void rotateR(TNode<T>*);
    void rotateL(TNode<T>*);
    void continueRotation(TNode<T>* gr, TNode<T>* par,
                          TNode<T>* ch, TNode<T>* desc);
    void moveUp(TNode<T>*);
    void inorder(TNode<T>*);
    void virtual visit(TNode<T>*) {
    }
};


template<class T>
void SortTree<T>::continueRotation(TNode<T>* gr, TNode<T>* par,
                                    TNode<T>* ch, TNode<T>* desc) {
    if (gr != 0) {
         if (gr->right == ch->parent)
              gr->right = ch;
         else gr->left  = ch;
    }
    else root = ch;
    if (desc != 0)
        desc->parent = par;
    par->parent = ch;
    ch->parent = gr;
}

template<class T>
void SortTree<T>::rotateR(TNode<T>* p) {
    p->parent->left = p->right;
    p->right = p->parent;
    continueRotation(p->parent->parent,p->right,p,p->right->left);
}

template<class T>
void SortTree<T>::rotateL(TNode<T>* p) {
    p->parent->right = p->left;
    p->left = p->parent;
    continueRotation(p->parent->parent,p->left,p,p->left->right);
}

template<class T>
void SortTree<T>::moveUp(TNode<T>* p) {
    while (p != root) {
        if (p->parent->parent == 0)
             if (p->parent->left == p)
                  rotateR(p);
             else rotateL(p);
        else if (p->parent->left == p)
             if (p->parent->parent->left == p->parent) {
                  rotateR(p->parent);
                  p = p->parent;
             }
             else {
                  rotateR(p);
                  rotateL(p);
             }
        else
             if (p->parent->parent->right == p->parent) {
                  rotateL(p->parent);
                  p = p->parent;
             }
             else {
                  rotateL(p);
                  rotateR(p);
             }
        if (root == 0)
            root = p;
    }
}

template<class T>
T* SortTree<T>::search(const T& el) {
    TNode<T> *p = root;
    while (p != 0)
        if (p->info == el) {
             moveUp(p);
             return &p->info;
        }
        else if (el < p->info)
             p = p->left;
        else p = p->right;
    return 0;
}

template<class T>
void SortTree<T>::insert(const T& el) {
    TNode<T> *p = root, *prev = 0, *newNode;
    while (p != 0) {
        prev = p;
        if (el < p->info)
             p = p->left;
        else p = p->right;
    }
    if ((newNode = new TNode<T>(el,0,0,prev)) == 0) {
         cerr << "OOM!\n";
         exit(1);
    }
    if (root == 0)
         root = newNode;
    else if (el < prev->info)
         prev->left  = newNode;
    else prev->right = newNode;
}

template<class T>
void SortTree<T>::inorder(TNode<T> *p) {
     if (p != 0) {
         inorder(p->left);
         visit(p);
         inorder(p->right);
     }
}

