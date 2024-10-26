#include "MST_Factory.hpp"
#include "MST_Algorithm.hpp"


MST_Factory *MST_Factory::instance = nullptr;

std::map<std::string, MST_Strategy *> MST_Factory::strategies = {{"prim", nullptr}, {"kruskal", nullptr}};
std::mutex MST_Factory::instance_mutex;

MST_Factory *MST_Factory::getInstance(){
    if (instance == nullptr){
        instance = new MST_Factory();
        strategies["prim"] = new Prim{};
        strategies["kruskal"] = new Kruskal{};
        std::atexit(cleanUp);
    }
    return instance;
}

MST_Strategy* MST_Factory::createMST(std::string type){
    std::lock_guard<std::mutex> lock(instance_mutex);  // Lock the mutex
    if(strategies.find(type) != strategies.end()){
        return strategies[type];
    }
    throw std::invalid_argument("Invalid MST Strategy");
}

void MST_Factory::cleanUp(){
    std::lock_guard<std::mutex> lock(instance_mutex);
    if (instance != nullptr){
        for (auto &strat : strategies){
            delete strat.second;
            strat.second = nullptr;
        }
        delete instance;
        instance = nullptr;
    }
}