#ifndef _LIFE_H_
#define _LIFE_H_
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <array>
#include <unistd.h>
#include <vector>

int saferMod(const int integer, const int modulus) {
  if (modulus < 2) return 0; 
  if (integer < 0) return modulus + integer;
  return integer % modulus;
}

class Cell {
  bool i_alive = false;
  bool i_includedInActiveVector = false; 
  std::vector<Cell*> *i_neighbors = new std::vector<Cell*>; 
  public:
  Cell() {
    //printf("hello cell\n"); 
  }

  void setNeighbor(Cell *neighbor) {
    i_neighbors->push_back(neighbor); 
  }

  void setToAlive() {
    i_alive = true; 
  }

  void setIncludedInActiveVector(const bool setting) {
    i_includedInActiveVector = setting; 
  }

  bool includedInActiveVector() {
    return i_includedInActiveVector; 
  }

  std::vector<Cell*> getNeighborsNotInActiveVector() {
    std::vector<Cell*> ret;
    for (Cell* n : *i_neighbors) {
      if (!n->includedInActiveVector()) {
        ret.push_back(n); 
      } 
    } 
    return ret; 
  }

  static void* testCellOperation(void* ptr) {
    std::cout << "test cell op\n";
    void* trivial;
    return trivial; 
  }

};

class Universe;

struct initLivingPkg {
  int i_rowNum = -1;
  Universe *i_universe = NULL;
  std::vector<Cell*> *i_subActiveVector;
  initLivingPkg(const int rowNum, Universe *universe, std::vector<Cell*> *subActiveVector) {
    i_rowNum = rowNum;
    i_universe = universe; 
    i_subActiveVector = subActiveVector;
  }
};

struct initCellsPkg {
  int i_rowNum = -1;
  std::vector<std::vector<Cell*>*> *i_space = new std::vector<std::vector<Cell*>*>;
  initCellsPkg(const int rowNum, std::vector<std::vector<Cell*>*> *space) {
    i_rowNum = rowNum;
    i_space = space; 
  }
};

struct initCurvePkg {
  int i_diameter = 0;
  std::vector<Cell*> *i_essentialCurve = new std::vector<Cell*>;
  initCurvePkg(const int diam, std::vector<Cell*> *curve) {
    i_diameter = diam;
    i_essentialCurve = curve; 
  }
};

class Universe {
  std::vector<std::vector<Cell*>*> *i_space = new std::vector<std::vector<Cell*>*>; 
  const int i_densityCoefficient = 30; // default
  std::vector<Cell*> *i_activeVector = new std::vector<Cell*>; 
  int testCounter = 0;
  // coordinatized space
  public:
  Universe () {}
  Universe (const int diameter) {
    pthread_t threadId[diameter];
    // initialize space
    printf("initialize space\n"); 
    for (int i = 0; i < diameter; ++i) {
      std::vector<Cell*> *essentialCurve = new std::vector<Cell*>;
      i_space->push_back(essentialCurve);
      int threadRet = pthread_create(&threadId[i], NULL, initializeCurve, (void*)(new initCurvePkg(diameter, essentialCurve)));
    } 
    for (int i = 0; i < diameter; ++i) {
      pthread_join(threadId[i], NULL); 
    } 
    // initialize cells
    printf("initialize cells\n"); 
    for (int i = 0; i < diameter; ++i) {
      int threadRet = pthread_create(&threadId[i], NULL, initializeCells, (void*)(new initCellsPkg(i, i_space)));
    }
    for (int i = 0; i < diameter; ++i) {
      pthread_join(threadId[i], NULL); 
    }
    // initialize living
    printf("initialize living\n"); 
    std::vector<std::vector<Cell*>*> subLivingVectors;
    for (int i = 0; i < diameter; ++i) {
      subLivingVectors.push_back(new std::vector<Cell*>);
      int threadRet = pthread_create(&threadId[i], NULL, initializeLiving, (void*)(new initLivingPkg(i, this, subLivingVectors.at(i))));
    }
    for (int i = 0; i < diameter; ++i) {
      pthread_join(threadId[i], NULL); 
    }
    for (std::vector<Cell*> *s : subLivingVectors) {
      for (Cell* c : *s) {
        i_activeVector->push_back(c); 
      } 
    }
    std::cout << "There are " << i_activeVector->size() << " active cells.\n";
    long double ratio = (long double)i_activeVector->size() / (long double)(diameter * diameter);
    std::cout << "The density is " << ratio << "\n"; 
    
  }

  void startTime() {
    for(;;) {
      
      moment();
      std::cout << testCounter << "\n";
      ++testCounter;
      usleep(1000000); 
    
    } 
  }

  void moment() {
    std::cout << "For each active cell, do thing\n";
    std::vector<Cell*> activeVector = *i_activeVector; 
    pthread_t doThreadId[activeVector.size()];
    for (int i = 0; i < activeVector.size(); ++i) {
      // cell do thing 
      int threadRet = pthread_create(&doThreadId[i], NULL, Cell::testCellOperation, NULL);
    } 
  
    for (int i = 0; i < activeVector.size(); ++i) {
      pthread_join(doThreadId[i], NULL); 
    }
    printf("end of moment\n"); 
  }

  static void* initializeCurve(void* essentialCurvePkg) {
    initCurvePkg *curvePkg = (initCurvePkg*)essentialCurvePkg; 
    for (int j = 0; j < curvePkg->i_diameter; ++j) {
      curvePkg->i_essentialCurve->push_back(new Cell); 
    } 
    void *trivial;
    return trivial;
  }

  static void* initializeCells(void* cellsPkg) {
    initCellsPkg *cellPkg = (initCellsPkg*)cellsPkg; 
    int diameter = cellPkg->i_space->at(cellPkg->i_rowNum)->size();
    int rowNum = cellPkg->i_rowNum;
    std::array<int, 3> rays = {-1, 0, 1}; 
    for (int i = 0; i < diameter; ++i) {
      for (int j = 0; j < 3; ++j) {
        for (int k = 0; k < 3; ++k) {
          std::array<int, 2> coord = {saferMod(i + rays.at(j), diameter), saferMod(rowNum + rays.at(k), diameter)};
          Cell *neighbor = cellPkg->i_space->at(coord.at(0))->at(coord.at(1)); 
          cellPkg->i_space->at(rowNum)->at(i)->setNeighbor(neighbor);
       
        } 
      }
    } 
    void *trivial;
    return trivial;
  }

  static void* initializeLiving(void* living) {
    initLivingPkg *livingPkg = (initLivingPkg*)living; 
    int rowNum = livingPkg->i_rowNum; 
    Universe *universe = livingPkg->i_universe;
    std::vector<Cell*> *subActiveVector = livingPkg->i_subActiveVector;
    int diameter = universe->i_space->size(); 
    for (int i = 0; i < diameter; ++i) {
      srand(i + rowNum * diameter);
      if ((rand() % universe->i_densityCoefficient) == 0) { 
        universe->i_space->at(rowNum)->at(i)->setToAlive();
        pthread_mutex_t mutex_cellLock = PTHREAD_MUTEX_INITIALIZER;
        if (!universe->i_space->at(rowNum)->at(i)->includedInActiveVector()) {
          pthread_mutex_lock(&mutex_cellLock);
          subActiveVector->push_back(universe->i_space->at(rowNum)->at(i));   
          universe->i_space->at(rowNum)->at(i)->setIncludedInActiveVector(true); 
          pthread_mutex_unlock(&mutex_cellLock);
        } 

        std::vector<Cell*> nbrsNotIncluded = universe->i_space->at(rowNum)->at(i)->getNeighborsNotInActiveVector();
        for (Cell* n : nbrsNotIncluded) {
          if (!n->includedInActiveVector()) { 
            pthread_mutex_lock(&mutex_cellLock);
            subActiveVector->push_back(n);
            n->setIncludedInActiveVector(true); 
            pthread_mutex_unlock(&mutex_cellLock);
          }
        }            
      
      } 
    }
    void *trivial;
    return trivial; 
  }

};

#endif
