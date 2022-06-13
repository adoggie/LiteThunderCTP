
#include "app.h"
#include "Controller.h"


int main(int argc , char ** argvs){
    Application::instance()->init(argc, argvs ).run();
}


