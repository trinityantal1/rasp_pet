#ifndef NN_CONTROLLER
#define NN_CONTROLLER
#include <string>

class NNController {
public:
    NNController ();
    void init();
    void CatMatch();
    void DogMatch();
    void categorise(std::string imgFilename);
    void NMS();

private:
    const int NN_CAT_CODE_CAT = 15; // eliminating hard coded values
    const int NN_CAT_CODE_DOG = 16;

};

#endif // NN_CONTROLLER